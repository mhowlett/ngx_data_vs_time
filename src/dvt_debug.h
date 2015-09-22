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
