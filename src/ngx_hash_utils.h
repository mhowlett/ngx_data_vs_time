#ifndef __NGX_HASH_UTILS_H
#define __NGX_HASH_UTILS_H

#include <ngx_config.h>
#include <ngx_core.h>

ngx_array_t *ngx_hash_to_array(ngx_hash_t* hash, ngx_pool_t* pool);
ngx_int_t compare_hash_items(const void *element_a, const void *element_b);

#endif
