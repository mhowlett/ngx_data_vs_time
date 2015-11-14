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

#ifndef __DVT_DEBUG_H
#define __DVT_DEBUG_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <ngx_core.h>

// I really have no idea what i'm doing, as indicated by
// by the nature of this collection of debug functions
// which I've been using to debug this module.

void dbg_ngx_ptr(ngx_str_t* msg, ngx_pool_t *pool);
void dbg_ngx(ngx_str_t msg, ngx_pool_t *pool);
void dbg_cstr(char* msg);
void dbg_cstr_nl(char* msg);
void dbg_char(char c);
void dbg_int(int i);
u_char *zero_terminate(ngx_str_t s, ngx_pool_t *pool);
void dbg_cstr_ascii(const char* msg, int len);

#endif
