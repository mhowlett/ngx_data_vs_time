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
 
#include "dvt_exposition.h"

#define OVERFLOW_CHECK \
  if (--safety_count <= 0 || pos > buffer_end) { goto invalid; }

ngx_array_t*
parse_exposition_text(ngx_str_t *s, ngx_pool_t* pool)
{
  u_char* token_start_pos;
  u_char* pos;
  u_char* tmp_pos;
  u_char* buffer_end;
  int tmp_cnt;
  ngx_str_t tmp_token;
  int safety_count = 100000;
  ngx_str_t errmsg = ngx_null_string;

  ngx_array_t* result;
  ngx_array_t** current_ts_identity;
  ngx_keyval_t* label;

  // TODO: estimate n items via a heuristic from the string length.
  // TODO: and emit WARNING message if the heuristic got it wrong.
  result = ngx_array_create(pool, 32, sizeof(ngx_array_t *));

  buffer_end = s->data + s->len;
  pos = s->data;

new_line:
  OVERFLOW_CHECK
  if (*pos == '\n')
  {
    ++pos;
    goto new_line;
  }
  if (*pos == ' ' || *pos == '\t')
  {
    ++pos;
    goto whitespace_line;
  }
  if (*pos == '#')
  {
    ++pos;
    goto reading_comment;
  }
  if ( (*pos >= 'a' && *pos <= 'z') ||
       (*pos >= 'A' && *pos <= 'Z') ||
       (*pos >= '0' && *pos <= '9') ||
       (*pos == '_') ||
       (*pos == '.') )
  {
    token_start_pos = pos;
    ++pos;
    current_ts_identity = (ngx_array_t **)ngx_array_push(result);
    // look-ahead to determine number of labels.
    // is this better than allocating too much memory?
    // it's quite possibly worse as in vast number of instances
    // there will be less than 4 dimensions, which is not much
    // memory. whatever, it's all quick.
    tmp_pos = pos;
    tmp_cnt = 0;
    while (*tmp_pos != '\n' && tmp_pos < buffer_end)
    {
      if (*tmp_pos == ',')
      {
        ++tmp_cnt;
      }
      ++tmp_pos;
    }
    *current_ts_identity = ngx_array_create(
        pool,
        tmp_cnt+1,
        sizeof(ngx_keyval_t));
    goto reading_metric;
  }
  if (pos >= buffer_end)
  {
    goto completed;
  }
  ngx_str_set(&errmsg, "unexpected character, new_line state.");
  goto error;

whitespace_line:
  OVERFLOW_CHECK
  if (*pos == ' ' || *pos == '\t')
  {
    ++pos;
    goto whitespace_line;
  }
  if (*pos == '\n')
  {
    ++pos;
    goto new_line;
  }
  ngx_str_set(&errmsg, "unexpected character: whitespace_line");
  goto error;

reading_comment:
  OVERFLOW_CHECK
  if (*pos == '\n')
  {
    ++pos;
    goto new_line;
  }
  if (pos >= buffer_end)
  {
    goto completed;
  }
  ++pos; // anything goes!
  goto reading_comment;

reading_metric:
  OVERFLOW_CHECK
  if ( (*pos >= 'a' && *pos <= 'z') ||
       (*pos >= 'A' && *pos <= 'Z') ||
       (*pos >= '0' && *pos <= '9') ||
       (*pos == '_') ||
       (*pos == '.') )
  {
    ++pos;
    goto reading_metric;
  }
  if (*pos == '{' || *pos == ' ' || *pos == '\t')
  {
    label = (ngx_keyval_t *)ngx_array_push(*current_ts_identity);
    ngx_str_set(&label->key, "metric");
    label->value.len = pos - token_start_pos;
    label->value.data = token_start_pos;

    if (*pos == ' ' || *pos == '\t')
    {
      ++pos;
      goto after_metric_whitespace;
    }

    ++pos;
    goto before_label_whitespace;
  }
  ngx_str_set(&errmsg, "unexpected character reading metric name");
  goto error;

after_metric_whitespace:
  OVERFLOW_CHECK
  if (*pos == ' ' || *pos == '\t')
  {
    ++pos;
    goto after_metric_whitespace;
  }
  if (*pos == '{')
  {
    ++pos;
    goto before_label_whitespace;
  }
  ngx_str_set(&errmsg, "unexpected character, after_metric_whitespace");
  goto error;

before_label_whitespace:
  OVERFLOW_CHECK
  if (*pos == ' ' || *pos == '\t')
  {
    ++pos;
    goto before_label_whitespace;
  }
  token_start_pos = pos;
  goto reading_label;

reading_label:
  OVERFLOW_CHECK
  if ( (*pos >= 'a' && *pos <= 'z') ||
       (*pos >= 'A' && *pos <= 'Z') ||
       (*pos >= '0' && *pos <= '9') ||
       (*pos == '_') ||
       (*pos == '.') )
  {
    ++pos;
    goto reading_label;
  }
  if (*pos == '=' || *pos == ' ' || *pos == '\t')
  {
    tmp_token.data = token_start_pos;
    tmp_token.len = pos - token_start_pos;
    if (*pos == '=')
    {
      ++pos;
      goto before_label_value_whitespace;
    }
    ++pos;
    goto after_label_whitespace;
  }
  ngx_str_set(&errmsg, "unexpected char, reading_label");
  goto error;

after_label_whitespace:
  OVERFLOW_CHECK
  if (*pos == ' ' || *pos == '\t')
  {
    ++pos;
    goto after_label_whitespace;
  }
  if (*pos == '=') {
    ++pos;
    goto before_label_value_whitespace;
  }
  ngx_str_set(&errmsg, "unexpected char, after_label_whitespace");
  goto error;

before_label_value_whitespace:
  OVERFLOW_CHECK
  if (*pos == ' ' || *pos == '\t')
  {
    ++pos;
    goto before_label_value_whitespace;
  }
  goto before_label_value_quote;

before_label_value_quote:
  OVERFLOW_CHECK
  if (*pos == '"')
  {
    ++pos;
    token_start_pos = pos;
    goto reading_label_value;
  }
  ngx_str_set(&errmsg, "unexpected char, before_label_value_quote");
  goto error;

reading_label_value:
  OVERFLOW_CHECK
  if ( (*pos >= 'a' && *pos <= 'z') ||
       (*pos >= 'A' && *pos <= 'Z') ||
       (*pos >= '0' && *pos <= '9') ||
       (*pos == '_') ||
       (*pos == '.') )
  {
    ++pos;
    goto reading_label_value;
  }
  if (*pos == '"')
  {
    label = (ngx_keyval_t *)ngx_array_push(*current_ts_identity);
    label->key = tmp_token;
    label->value.len = pos - token_start_pos;
    label->value.data = token_start_pos;

    ++pos;
    goto after_label_value_whitespace;
  }
  ngx_str_set(&errmsg, "unexpected char, reading_label_value");
  goto error;

after_label_value_whitespace:
  OVERFLOW_CHECK
  if (*pos == '}')
  {
    ++pos;
    goto after_identity;
  }
  if (*pos == ',')
  {
    ++pos;
    goto before_label_whitespace;
  }
  if (*pos == ' ' || *pos == '\t')
  {
    ++pos;
    goto after_label_value_whitespace;
  }
  ngx_str_set(&errmsg, "unexpected char, after_label_value_whitespace");
  goto error;

after_identity:
  OVERFLOW_CHECK
  if (*pos == ' ' || *pos == '\t')
  {
    ++pos;
    goto after_identity;
  }
  if (*pos == '\n')
  {
    ++pos;
    goto new_line;
  }
  if (pos >= buffer_end)
  {
    goto completed;
  }
  ngx_str_set(&errmsg, "unexpected char, after_identity");
  goto error;

invalid:
  ngx_str_set(&errmsg, "invalid format");

error:
  // TODO: log error.
  return NULL;

completed:
  return result;
}

// ## NOTE: All strings are converted to Lower Case in ngx hash.

const char* exposition2 =
"# HELP bytes_sent Number of bytes sent from LAN to remote\n"
"# TYPE bytes_sent gauge\n"
"bytes_sent{host_remote=\"www.google.com\",ip_remote=\"200.32.1.65\",host_lan=\"oberon\",ip_lan=\"192.168.0.4\",l4_type=\"tcp\",l7_type=\"https\"}\n"
"bytes_sent{host_remote=\"www.google.com\",ip_remote=\"200.32.1.65\",host_lan=\"franklin\",ip_lan=\"192.168.0.5\",l4_type=\"tcp\",l7_type=\"https\"}\n"
"bytes_sent{host_remote=\"www.google.com\",ip_remote=\"200.32.1.65\",host_lan=\"federation\",ip_lan=\"192.168.0.16\",l4_type=\"tcp\",l7_type=\"https\"}\n"
"bytes_sent{host_remote=\"www.google.com\",ip_remote=\"200.32.1.65\",host_lan=\"judd\",ip_lan=\"192.168.0.22\",l4_type=\"tcp\",l7_type=\"https\"}\n"
"bytes_received{host_remote=\"www.google.com\",ip_remote=\"200.32.1.65\",host_lan=\"oberon\",ip_lan=\"192.168.0.4\",l4_type=\"tcp\",l7_type=\"https\"}\n"
"bytes_received{host_remote=\"www.google.com\",ip_remote=\"200.32.1.65\",host_lan=\"franklin\",ip_lan=\"192.168.0.5\",l4_type=\"tcp\",l7_type=\"https\"}\n"
"bytes_received{host_remote=\"www.google.com\",ip_remote=\"200.32.1.65\",host_lan=\"federation\",ip_lan=\"192.168.0.16\",l4_type=\"tcp\",l7_type=\"https\"}\n"
"bytes_received{host_remote=\"www.google.com\",ip_remote=\"200.32.1.65\",host_lan=\"judd\",ip_lan=\"192.168.0.22\",l4_type=\"tcp\",l7_type=\"https\"}\n"
;

const char* exposition =
"# HELP ping_time_ms Ping time to a hypothetical host\n"
"# TYPE ping_time_ms gauge\n"
"ping_time_ms{level=\"4\",group=\"all\"}\n"
"ping_time_ms{level=\"27\",group=\"all\"}\n"
"ping_time_ms{level=\"120\",group=\"all\"}\n"
"ping_time_ms{level=\"130\",group=\"all\"}\n"
"ping_time_ms{level=\"180\",group=\"all\"}\n"
"ping_time_ms{level=\"220\",group=\"all\"}\n"
"ping_time_ms{level=\"320\",group=\"all\"}\n"
"ping_time_ms{level=\"500\",group=\"all\"}\n"
"# HELP sin Sinusoidal curve\n"
"# TYPE sin gauge\n"
"sin{period=\"4\",scale=\"small\",group=\"all\"}\n"
"sin{period=\"5\",scale=\"small\",group=\"all\"}\n"
"sin{period=\"7\",scale=\"small\",group=\"all\"}\n"
"sin{period=\"9\",scale=\"small\",group=\"all\"}\n"
"sin{period=\"17\",scale=\"medium\",group=\"all\"}\n"
"sin{period=\"23\",scale=\"medium\",group=\"all\"}\n"
"sin{period=\"29\",scale=\"medium\",group=\"all\"}\n"
"sin{period=\"36\",scale=\"medium\",group=\"all\"}\n"
"sin{period=\"95\",scale=\"medium\",group=\"all\"}\n"
"sin{period=\"113\",scale=\"large\",group=\"all\"}\n"
"sin{period=\"198\",scale=\"large\",group=\"all\"}\n"
"sin{period=\"207\",scale=\"large\",group=\"all\"}\n"
"# HELP random_wave Sinusoidal curve with noise\n"
"# TYPE random_wave gauge\n"
"random_wave{period=\"4\",scale=\"small\",group=\"all\"}\n"
"random_wave{period=\"5\",scale=\"small\",group=\"all\"}\n"
"random_wave{period=\"7\",scale=\"small\",group=\"all\"}\n"
"random_wave{period=\"10\",scale=\"medium\",group=\"all\"}\n"
"random_wave{period=\"25\",scale=\"medium\",group=\"all\"}\n"
"random_wave{period=\"42\",scale=\"medium\",group=\"all\"}\n"
"random_wave{period=\"67\",scale=\"medium\",group=\"all\"}\n"
"random_wave{period=\"133\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"145\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"168\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"220\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"265\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"310\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"340\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"387\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"412\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"444\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"502\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"550\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"599\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"680\",scale=\"large\",group=\"all\"}\n"
"random_wave{period=\"850\",scale=\"x_large\",group=\"all\"}\n"
"# HELP wt_tonnes Simulation of bulk solid tonnes over weightometer\n"
"# TYPE wt_tonnes gauge\n"
"wt_tonnes{level=\"300\",width=\"4\",group=\"all\"}\n"
"wt_tonnes{level=\"100\",width=\"10\",group=\"all\"}\n"
"wt_tonnes{level=\"237\",width=\"7\",group=\"all\"}\n"
"wt_tonnes{level=\"310\",width=\"5\",group=\"all\"}\n"
"wt_tonnes{level=\"290\",width=\"11\",group=\"all\"}\n"
"wt_tonnes{level=\"276\",width=\"6\",group=\"all\"}\n"
"wt_tonnes{level=\"302\",width=\"3\",group=\"all\"}\n"
"wt_tonnes{level=\"288\",width=\"8\",group=\"all\"}\n"
"# HELP mixed_wave Superposition of sinusoidal curves.\n"
"# TYPE mixed_wave gauge\n"
"mixed_wave{scale=\"large\",group=\"all\"}\n"
;


ngx_array_t *
load_test_exposition(ngx_pool_t *pool)
{
  ngx_str_t expo;
  expo.data = (char *)exposition;
  expo.len = strlen(exposition);

  return parse_exposition_text(&expo, pool);
}
