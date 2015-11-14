#ifndef __EXPOSITION_TEXT_UTILS
#define __EXPOSITION_TEXT_UTILS

#include <ngx_core.h>

ngx_array_t* load_test_exposition(ngx_pool_t *pool);

// returns:
//   array [of series specs]
//     which are array [of label:value pairs as ngx_keyval_t]
ngx_array_t* parse_exposition_text(ngx_str_t *s, ngx_pool_t* pool);

#endif
