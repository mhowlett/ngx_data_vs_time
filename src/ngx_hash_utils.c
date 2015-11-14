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

#include "ngx_hash_utils.h"

// a = ngx_hash_to_array(&hash, pool);
// els = a->elts;
// for (i=0; i<a->nelts; ++i) {
//   dbgint(els[i].key.len);
// }
ngx_array_t *ngx_hash_to_array(ngx_hash_t* hash, ngx_pool_t* pool)
{
  ngx_array_t* result;
  ngx_hash_elt_t  *elt;
  int i;
  ngx_hash_key_t* v;

  result = ngx_array_create(pool, 32, sizeof(ngx_hash_key_t));
  
  for (i = 0; i<hash->size; ++i) {
    elt = hash->buckets[i];
    
    while(elt->value) {
      v = (ngx_hash_key_t *)ngx_array_push(result);
      v->value = elt->value;
      v->key.data = elt->name;
      v->key.len = elt->len;
      v->key_hash = ngx_hash_key(elt->name, elt->len);
      
      elt = (ngx_hash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len,
                                               sizeof(void *));
    }
  }
 
  return result;
}


ngx_int_t compare_hash_items(const void *element_a, const void *element_b)
{
  ngx_hash_key_t* a = (ngx_hash_key_t *)element_a;
  ngx_hash_key_t* b = (ngx_hash_key_t *)element_b;

  // compare keys.
  return 0;
}
