/*
 * This file is part of ngx_data_vs_time.
 * (c) Matt Howlett 2015 https://www.matthowlett.com
 *
 * ngx_data_vs_time is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ngx_data_vs_time is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ngx_data_vs_time.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * ngx_data_vs_time is derived, in part, from nginx source which is 
 * distributed with the following license:
 *
 * Copyright (C) 2002-2015 Igor Sysoev
 * Copyright (C) 2011-2015 Nginx, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "dvt_simulation.h"
#include "dvt_cache.h"
#include "dvt_series_spec.h"
#include <math.h>

#define MAX_N_VALUES 10000

typedef struct functionObject_s {
  void *data;
  double (*fn)(struct functionObject_s* d, int64_t t, int64_t step);
} functionObject_t;

// START MURMUR3

uint32_t fmix ( uint32_t h )
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

uint32_t getblock ( const uint32_t * p, int i )
{
  return p[i];
}

inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
  return (x << r) | (x >> (32 - r));
}
#define ROTL32(x,y)     rotl32(x,y)

uint32_t MurmurHash3_x86_32 ( const void * key, int len, uint32_t seed )
{
  const uint8_t * data = (const uint8_t*)key;
  const int nblocks = len / 4;

  uint32_t h1 = seed;

  uint32_t c1 = 0xcc9e2d51;
  uint32_t c2 = 0x1b873593;

  const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

  int i;
  for(i = -nblocks; i; i++)
  {
    uint32_t k1 = getblock(blocks,i);

    k1 *= c1;
    k1 = ROTL32(k1,15);
    k1 *= c2;

    h1 ^= k1;
    h1 = ROTL32(h1,13);
    h1 = h1*5+0xe6546b64;
  }

  const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

  uint32_t k1 = 0;

  switch(len & 3)
  {
  case 3: k1 ^= tail[2] << 16;
  case 2: k1 ^= tail[1] << 8;
  case 1: k1 ^= tail[0];
          k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
  };

  h1 ^= len;

  h1 = fmix(h1);

  return h1;
}

double uniform_rand_01(int64_t t)
{
  return ((double)MurmurHash3_x86_32(&t,sizeof(int64_t),0) / (double)UINT32_MAX);
}

// END MURMUR3


static double dvt_simulate_ping_time_ms(functionObject_t* fo, int64_t t, int64_t step)
{
  double lvl = *((double *)(fo->data));
  double rnd = uniform_rand_01(t/15000 + (int)lvl);
  // double rnd = 0.1;

  if (rnd < 0.95) {
    return rnd * (1 + lvl / 10) + lvl;
  }
  if (rnd < 0.98) {
    return (rnd - 0.95) * 20.0 * 200 + 800;
  }
  return 5000;
}

static double dvt_simulate_sin(functionObject_t* fo, int64_t t, int64_t step)
{
  double period_seconds = *((double *)(fo->data));
  return sin((double)t/1000.0*M_2_PI/period_seconds);
}

static double dvt_simulate_random_wave(functionObject_t* fo, int64_t t, int64_t step)
{
  double period_seconds = *((double *)(fo->data));
  double rnd = uniform_rand_01(t + (int)period_seconds);
  return sin((double)t/1000.0*M_2_PI/period_seconds) + rnd * 0.1 - 0.05;
}

static double dvt_simulate_mixed_wave(functionObject_t* fo, int64_t t, int64_t step)
{
  return
    10.0 +
    sin((double)t/1000.0*M_2_PI/100.0) +
    sin((double)t/1000.0*M_2_PI/802.0)*2.0 +
    sin((double)t/1000.0*M_2_PI/7)*0.02 +
    sin((double)t/1000.0*M_2_PI/9)*0.05;
}

static double dvt_simulate_wt_tonnes(functionObject_t* fo, int64_t t, int64_t step)
{
  uint32_t level = ((uint32_t *)(fo->data))[0];
  uint32_t period_10m = ((uint32_t *)(fo->data))[1] * 60 * 10 * 1000;

  int64_t p1 = (int64_t) ((uniform_rand_01(t/(int64_t)period_10m) / 2.0) * period_10m + (t/(int64_t)period_10m)*period_10m);
  int64_t p1_a = p1 - 1000*20;

  int64_t p2 = (int64_t) ((uniform_rand_01(t/(int64_t)period_10m+1) / 2.0 + 0.5) * period_10m + (t/(int64_t)period_10m)*period_10m);
  int64_t p2_b = p2 + 1000*20;

  if (t >= p1 && t <= p2) {
    // implement a bit of (resolution dependent!) smoothing to make it look better.
    double v1 = (1 + uniform_rand_01(t) * 0.04);
    double v2 = (1 + uniform_rand_01(t-step) * 0.04);
    double v3 = (1 + uniform_rand_01(t-2*step) * 0.04);
    double v4 = (1 + uniform_rand_01(t-3*step) * 0.04);
    return (v1+v2+v3+v4) * level / 4;
  }

  if (t > p1_a && t < p1) {
    return (double)(t - p1_a) / (double)(p1 - p1_a) * level;
  }

  if (t > p2 && t < p2_b) {
    return (double)(p2_b - t) / (double)(p2_b - p2) * level;
  }

  return 0.0;
}

ngx_str_t dvt_simulate_data(
  dvt_cache_t *cache,
  uint32_t *series_spec,
  uint64_t start,
  uint64_t stop,
  uint64_t step,
  ngx_pool_t *pool)
{
  ngx_str_t result = ngx_null_string;
  char tmp_str[32];
  double d;
  int i;

  ngx_str_t *metric_name;
  ngx_str_t label_value;

  int n = (int)((stop - start) / step);
  if (n < 1 || n > MAX_N_VALUES)
  {
    return result;
  }

  uint32_t mid = dvt_series_spec_metric_value_id(series_spec);
  metric_name = dvt_cache_label_value_name_from_id(cache, metric_label_id, mid);
  //uint32_t mid = 0;
  //ngx_str_t mn = ngx_string("ping_time_ms");
  //metric_name = &mn;

  functionObject_t fo;
  if (metric_name->len == sizeof("sin")-1 &&
      strncmp(metric_name->data, "sin", 3) == 0)
  {
    ngx_str_t period_str = ngx_string("period");
    label_value = dvt_series_spec_label_value(cache, series_spec, &period_str);
    i = ngx_atoi(label_value.data, label_value.len);
    d = (double)i;

    fo.data = &d;
    fo.fn = &dvt_simulate_sin;
  }
  else if (metric_name->len == sizeof("bytes_sent")-1 &&
      strncmp(metric_name->data, "bytes_sent", 3) == 0)
  {
    d = (double)24;

    fo.data = &d;
    fo.fn = &dvt_simulate_sin;
  }
  else if (metric_name->len == sizeof("ping_time_ms")-1 &&
      strncmp(metric_name->data, "ping_time_ms", 3) == 0)
  {
    ngx_str_t level_str = ngx_string("level");
    label_value = dvt_series_spec_label_value(cache, series_spec, &level_str);
    i = ngx_atoi(label_value.data, label_value.len);
    d = (double)i;
    //d = 120.0;

    fo.data = &d;
    fo.fn = &dvt_simulate_ping_time_ms;
  }
  else if (metric_name->len == sizeof("random_wave")-1 &&
      strncmp(metric_name->data, "random_wave", 3) == 0)
  {
    ngx_str_t period_str = ngx_string("period");
    label_value = dvt_series_spec_label_value(cache, series_spec, &period_str);
    i = ngx_atoi(label_value.data, label_value.len);
    d = (double)i;

    fo.data = &d;
    fo.fn = &dvt_simulate_random_wave;
  }
  else if (metric_name->len == sizeof("mixed_wave")-1 &&
      strncmp(metric_name->data, "mixed_wave", 3) == 0)
  {
    d = 0.0;
    fo.data = &d;
    fo.fn = &dvt_simulate_mixed_wave;
  }
  else if (metric_name->len == sizeof("wt_tonnes")-1 &&
      strncmp(metric_name->data, "wt_tonnes", 3) == 0)
  {
    uint32_t* ars = (uint32_t *)ngx_palloc(pool, sizeof(uint32_t) * 2);

    ngx_str_t level_str = ngx_string("level");
    label_value = dvt_series_spec_label_value(cache, series_spec, &level_str);
    ars[0] = (uint32_t)ngx_atoi(label_value.data, label_value.len);

    ngx_str_t width_str = ngx_string("width");
    label_value = dvt_series_spec_label_value(cache, series_spec, &width_str);
    ars[1] = (uint32_t)ngx_atoi(label_value.data, label_value.len);

    fo.data = ars;
    fo.fn = &dvt_simulate_wt_tonnes;
  }
  else
  {
    return result;
  }

  result.data = ngx_palloc(pool, 16 * n); // oodles of room. todo: better.
  if (result.data == NULL)
  {
    ngx_str_t res = ngx_null_string;
    return res;
  }
  result.len = 0;

  u_char *p = result.data;
  p = ngx_cpymem(p, "[", sizeof("[") - 1);

  int64_t current;
  for (current = start; current<stop; current += step)
  {
    if (current != start)
    {
      p = ngx_cpymem(p, ",", sizeof(",") - 1);
    }

    double d = fo.fn(&fo, current, step);
    snprintf(tmp_str, sizeof(tmp_str), "\"%.4f\"", d);
    p = ngx_cpymem(p, tmp_str, strlen(tmp_str));
  }

  p = ngx_cpymem(p, "]", sizeof("]\0") - 1);

  result.len = strlen((char *)result.data);

  return result;
}
