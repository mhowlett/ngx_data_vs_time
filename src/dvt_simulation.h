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
