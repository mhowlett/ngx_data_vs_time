/*
 * ngx_data_vs_time
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
