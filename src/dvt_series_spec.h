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
