#include "dvt_debug.h"

void dbg_cstr(char* msg)
{
  int safety = 1000;
  while (*msg != '\0' && --safety > 0)
  {
    fprintf(stderr, "%c", *msg);
    msg += 1;
  }
  fflush(stderr);
}

void dbg_cstr_nl(char* msg)
{
  dbg_cstr(msg);
  fprintf(stderr, "\n");
  fflush(stderr);
}

void dbg_char(char c)
{
  char s[5];
  sprintf(s, ">%c<", c);
  dbg_cstr_nl(s);
}

void dbg_int(int i)
{
  char s[32];
  sprintf(s, ">%d<", i);
  dbg_cstr_nl(s);
}

u_char *zero_terminate(ngx_str_t s, ngx_pool_t *pool)
{
  u_char* result;
  result = ngx_palloc(pool, sizeof(u_char) *(s.len + 1));
  ngx_cpymem(result, s.data, s.len);
  result[s.len] = '\0';
  return result;
}

void dbg_ngx_ptr(ngx_str_t* msg, ngx_pool_t* pool)
{
  if (msg == NULL)
  {
    dbg_cstr_nl("dbg_ngx_ptr ERROR: msg is null");
    return;
  }
  if (msg->data == NULL)
  {
    dbg_cstr_nl("dbg_ngx_ptr ERROR: data (null)");
    return;
  }
  if(msg->len == 0)
  {
    dbg_cstr_nl("dbg_ngx_ptr ERROR: 0 length string");
    return;
  }
  dbg_cstr_nl(zero_terminate(*msg, pool));
}

void dbg_ngx(ngx_str_t s, ngx_pool_t *pool)
{
  dbg_ngx_ptr(&s, pool);
}

void dbg_cstr_ascii(const char* msg, int len)
{
  int i;
  char s[32];
  char c[2];
  for (i=0; i<len; ++i)
  {
    sprintf(s, "%d", msg[i]);
    dbg_cstr(s);
    dbg_cstr(" ");
    c[0] = msg[i];
    c[1] = '\0';
    sprintf(s, ">%s<", c);
    dbg_cstr(s);
    dbg_cstr("\n");
  }
}
