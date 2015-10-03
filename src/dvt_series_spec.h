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

#ifndef __DVT_SERIES_SPEC_H
#define __DVT_SERIES_SPEC_H

#include <ngx_core.h>
#include "dvt_cache.h"

// series spec is just a c-array of uint32_t's.
// it would have been really good if this could
// have been uint16_t's as this would suffice
// in many (most?) scenarios and the memory
// saving would be great in e.g. hash lookups,
// but unfortunately 32 bits are required in
// many others.
//
// 1. metric_value_id  (UINT32_MAX => metric not specified)
// 2. additional_label_count
// 3.   label_id
// 4.   value_id
// 5.   ...
//
// label/values are always stored sorted by
// label_id.
//
// metric can in fact be not specifid, this is
// signaled by matric_value_id = UINT32_MAX.
// This arrangement is memory efficient c.f.
// including the metric value in the general
// label list and notably results in a data
// structure that is a multiple of 8 bytes
// long.

#define dvt_series_spec_metric_value_id(spec) *(spec)
#define dvt_series_spec_label_count(spec) *((spec) + 1)
#define dvt_series_spec_label_id(spec, i) *((spec) + 2 + (i)*2)
#define dvt_series_spec_value_id(spec, i) *((spec) + 2 + (i)*2 + 1)
#define dvt_series_spec_kvs(spec) ((spec)+2)

uint32_t dvt_series_in_set(uint32_t *set, uint32_t *spec);

//ngx_str_t *dvt_series_metric_name(dvt_cache_t *cache, uint32_t *spec);
//ngx_str_t *dvt_series_label_value(dvt_cache_t *cache, uint32_t *spec, ngx_str_t *label_name);
ngx_str_t dvt_series_spec_label_value(dvt_cache_t *cache, uint32_t *spec, ngx_str_t *name);
uint32_t *dvt_series_spec_from_kv_array(dvt_cache_t *cache, ngx_array_t *kvps, ngx_pool_t* pool);
uint32_t *dvt_parse_series_spec_text(dvt_cache_t *cache, ngx_str_t* s, ngx_pool_t* pool);
ngx_str_t dvt_series_spec_to_text (dvt_cache_t *cache, uint32_t *spec, ngx_pool_t *pool);
ngx_str_t dvt_series_spec_to_json (dvt_cache_t *cache, uint32_t *spec, ngx_pool_t *pool);

// todo.
ngx_str_t dvt_series_spec_to_text_raw(uint32_t *spec, ngx_pool_t *pool);

#endif
