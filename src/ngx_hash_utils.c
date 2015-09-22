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
