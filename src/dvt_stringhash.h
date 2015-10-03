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

#ifndef __DVT_STRINGHASH_H
#define __DVT_STRINGHASH_H


#include <ngx_config.h>
#include <ngx_core.h>

// - the layout of data stored in the buckets. 
// - this type is never instantiated and the length 1 of the 
//   name array is irrelevant.
// - more than one element can be in a bucket (if they are 
//   small enough. if so they will be memory aligned.
typedef struct {
    void             *value;
    u_short           len;
    u_char            name[1];
} dvt_stringhash_elt_t;

// size is the number of buckets, not the number of elements.
typedef struct {
    dvt_stringhash_elt_t  **buckets;
    ngx_uint_t        size;
} dvt_stringhash_t;

// data format given on hash initialization.
// all fields must be specified.
typedef struct {
    ngx_str_t         key;
    ngx_uint_t        key_hash;
    void             *value;
} dvt_stringhash_key_val_t;

// function pointer to the hashing function.
typedef ngx_uint_t (*dvt_stringhash_key_gen_pt) (u_char *data, size_t len);

typedef struct {
    dvt_stringhash_t *hash;
    dvt_stringhash_key_gen_pt   key_gen;

    ngx_uint_t        max_size;
    ngx_uint_t        bucket_size;

    char             *name;
    ngx_pool_t       *pool;
} dvt_stringhash_init_t;

ngx_int_t dvt_stringhash_init(dvt_stringhash_init_t *hinit, dvt_stringhash_key_val_t *names, ngx_uint_t nelts);
void *dvt_stringhash_find(dvt_stringhash_t *hash, ngx_uint_t key, u_char *name, size_t len);
ngx_array_t *dvt_stringhash_to_array(dvt_stringhash_t *hash, ngx_pool_t* pool);

#endif
