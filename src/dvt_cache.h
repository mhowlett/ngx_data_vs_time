#ifndef __DVT_SERIES_SPEC_STORE
#define __DVT_SERIES_SPEC_STORE

#include <ngx_core.h>

#define metric_label_id 0

typedef struct dvt_cache_s
{
  // TODO: none of these need to be pointers. simplify.
  ngx_str_t *label_and_value_summary_json;
  ngx_array_t *label_names;
  ngx_hash_t *label_ids;
  ngx_array_t *label_value_names; // array [label_id] of arrays [label_value_id -> name]
  ngx_array_t *label_value_ids; // array [label_id] of hashes [label_value_name -> id]
  ngx_array_t all_series;
} dvt_cache_t;


dvt_cache_t *dvt_cache_init(ngx_pool_t *pool);

uint32_t dvt_number_of_labels(dvt_cache_t *cache);

ngx_array_t *dvt_cache_get_series_set(dvt_cache_t *cache, uint32_t* set_spec, ngx_pool_t *pool);

uint32_t dvt_cache_label_id_from_name(dvt_cache_t *cache, ngx_str_t *name);
ngx_str_t *dvt_cache_label_name_from_id(dvt_cache_t *cache, uint32_t id);
uint32_t dvt_cache_label_value_id_from_name(dvt_cache_t *cache, uint32_t label_id, ngx_str_t *name);
ngx_str_t *dvt_cache_label_value_name_from_id(dvt_cache_t *cache, uint32_t label_id, uint32_t id);


#endif
