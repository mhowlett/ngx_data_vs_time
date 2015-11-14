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

#include "ngx_string_utils.h"

void
ngx_push_to_string_array_cstr(ngx_array_t *a, const char *s)
{
  int i;
  int len = strlen(s);
  u_char *c;

  for (i=0; i<len; ++i)
  {
    c = (u_char *)ngx_array_push(a);
    *c = s[i];
  }
}

void
ngx_push_to_string_array_ngx_ptr(ngx_array_t *a, ngx_str_t *s)
{
  int i;
  u_char *c;

  for (i=0; i<s->len; ++i)
  {
    c = (u_char *)ngx_array_push(a);
    *c = s->data[i];
  }
}

void
ngx_push_to_string_array_ngx(ngx_array_t *a, ngx_str_t s)
{
  int i;
  u_char *c;

  for (i=0; i<s.len; ++i)
  {
    c = (u_char *)ngx_array_push(a);
    *c = s.data[i];
  }
}

ngx_str_t
ngx_string_array_to_string(ngx_array_t *a)
{
  ngx_str_t r;
  r.len = a->nelts;
  r.data = a->elts;
  return r;
}
