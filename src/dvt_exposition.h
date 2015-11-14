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

#ifndef __EXPOSITION_TEXT_UTILS
#define __EXPOSITION_TEXT_UTILS

#include <ngx_core.h>

ngx_array_t* load_test_exposition(ngx_pool_t *pool);

// returns:
//   array [of series specs]
//     which are array [of label:value pairs as ngx_keyval_t]
ngx_array_t* parse_exposition_text(ngx_str_t *s, ngx_pool_t* pool);

#endif
