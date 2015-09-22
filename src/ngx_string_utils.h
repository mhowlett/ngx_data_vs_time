#ifndef __NGX_STRING_UTILS_H
#define __NGX_STRING_UTILS_H

#include <ngx_core.h>

void ngx_push_to_string_array_cstr(ngx_array_t *a, const char *s);
void ngx_push_to_string_array_ngx_ptr(ngx_array_t *a, ngx_str_t *s);
ngx_str_t ngx_string_array_to_string(ngx_array_t *a);

#endif
