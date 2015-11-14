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

#ifndef __NGX_HASH_UTILS_H
#define __NGX_HASH_UTILS_H

#include <ngx_config.h>
#include <ngx_core.h>

ngx_array_t *ngx_hash_to_array(ngx_hash_t* hash, ngx_pool_t* pool);
ngx_int_t compare_hash_items(const void *element_a, const void *element_b);

#endif
