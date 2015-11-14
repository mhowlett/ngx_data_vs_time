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
