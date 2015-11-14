#ifndef __DVT_SIMULATION_H
#define __DVT_SIMULATION_H

#include <stdint.h>
#include <ngx_core.h>
#include "dvt_cache.h"
#include "dvt_series_spec.h"

ngx_str_t dvt_simulate_data(
  dvt_cache_t *cache,
  uint32_t *series_spec,
  uint64_t start,
  uint64_t stop,
  uint64_t step,
  ngx_pool_t *pool);

#endif
