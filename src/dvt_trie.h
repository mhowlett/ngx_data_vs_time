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
 * This file may be derived, in part from nginx source, distributed with
 * the following license:
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
 
#ifndef __DVT_TRIE_H
#define __DVT_TRIE_H

// this is all rubbish.

/*
#include <ngx_core.h>
#include "dvt_series_spec.h"

typedef struct dvt_trie_node_s
{
  uint32_t inbound_label_id;
  uint32_t inbound_value_id;
  ngx_array_t series;   // of uint32_t* : series in the set defined by the node.
  ngx_array_t children; // of dvt_trie_node_t*
} dvt_trie_node_t;

void dvt_trie_init(dvt_trie_node_t *root, ngx_pool_t *pool);
void dvt_trie_insert(dvt_trie_node_t *root, uint32_t *series, ngx_pool_t *pool);
ngx_str_t dvt_trie_to_string(dvt_trie_node_t *node, ngx_pool_t *pool);

// return array type uint32_t*
ngx_array_t *dvt_trie_get_series_specs(dvt_trie_node_t *root, uint32_t *spec);
*/

#endif
