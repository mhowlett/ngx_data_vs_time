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

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <nginx.h>
#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include "ngx_hash_utils.h"
#include "ngx_string_utils.h"

#include "dvt_exposition.h"
#include "dvt_series_spec.h"
#include "dvt_cache.h"
#include "dvt_stringhash.h"
#include "dvt_debug.h"
#include "dvt_simulation.h"
#include "dvt_trie.h"

static char *ngx_http_data_vs_time(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

typedef struct {
    ngx_flag_t enable;
    ngx_uint_t simset;
    dvt_cache_t *cache;
} ngx_http_data_vs_time_loc_conf_t;


static void *
ngx_http_data_vs_time_create_loc_conf(ngx_conf_t *cf)
{
  ngx_http_data_vs_time_loc_conf_t  *conf;

  conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_data_vs_time_loc_conf_t));
  if (conf == NULL) {
    return NGX_CONF_ERROR;
  }

  conf->simset = NGX_CONF_UNSET_UINT;
  conf->enable = NGX_CONF_UNSET;
  conf->cache = NGX_CONF_UNSET_PTR;

  return conf;
}

static char *
ngx_http_data_vs_time_init(ngx_http_data_vs_time_loc_conf_t *conf, ngx_pool_t *pool)
{
  conf->cache = dvt_cache_init(pool);
  return NGX_CONF_OK;
}


static char *
ngx_http_data_vs_time_merge_loc_conf(ngx_conf_t *cf, void *parent, void *child)
{
  ngx_http_data_vs_time_loc_conf_t *prev = parent;
  ngx_http_data_vs_time_loc_conf_t *conf = child;

  ngx_conf_merge_uint_value(conf->simset, prev->simset, 0);
  ngx_conf_merge_value(conf->enable, prev->enable, 0);

  if (conf->enable) {
    ngx_http_data_vs_time_init(conf, cf->pool);
  }

  if (conf->enable && conf->simset != 113) {
     ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
         "simset must be 113");
     return NGX_CONF_ERROR;
  }

  return NGX_CONF_OK;
}


// Where the directives (what can be used in the conf file) are
// defined.
//
// referenced by the main entry point struct defined below.
//
// ngx_http_data_vs_time is defined right at the bottom of this
// file: "a pointer to a function for setting up part of the
// module's configuration; typically this function will translate
// the arguments passed to this directive and save an appropriate
// value in its configuration struct."
//
// there are a number of predefined functions for this purpose,
// like ngx_conf_set_num_slot, but here we have a custom one.

static ngx_command_t  ngx_http_data_vs_time_commands[] = {

  { ngx_string("data_vs_time"),
    NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
    ngx_http_data_vs_time,
    NGX_HTTP_LOC_CONF_OFFSET,
    0,
    NULL },

  { ngx_string("data_vs_time_simset"),
    NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
    ngx_conf_set_num_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_data_vs_time_loc_conf_t, simset),
    NULL },

    ngx_null_command
};

// a bunch of function references for creating the three configurations
// (main, srv, loc) and merging them together
//
// most modules evidently just use the last two.
//
// referenced by ngx_http_data_vs_time_module which is referenced by
// "config"

static ngx_http_module_t  ngx_http_data_vs_time_module_ctx = {
  NULL,                          /* preconfiguration */
  NULL,                          /* postconfiguration */
  NULL,                          /* create main configuration */
  NULL,                          /* init main configuration */
  NULL,                          /* create server configuration */
  NULL,                          /* merge server configuration */
  ngx_http_data_vs_time_create_loc_conf,  /* create location configuration */
  ngx_http_data_vs_time_merge_loc_conf /* merge location configuration */
};

// ENTRY point
//
// Referenced by the "config" module definition file.
//
// This is kind of like the entry point to the module I think.

ngx_module_t ngx_http_data_vs_time_module = {
  NGX_MODULE_V1,
  &ngx_http_data_vs_time_module_ctx, /* module context */
  ngx_http_data_vs_time_commands,   /* module directives */
  NGX_HTTP_MODULE,               /* module type */
  NULL,                          /* init master */
  NULL,                          /* init module */
  NULL,                          /* init process */
  NULL,                          /* init thread */
  NULL,                          /* exit thread */
  NULL,                          /* exit process */
  NULL,                          /* exit master */
  NGX_MODULE_V1_PADDING
};

static ngx_str_t label_and_value_summary_handler(ngx_http_request_t *r)
{
  ngx_http_data_vs_time_loc_conf_t *cglcf;
  cglcf = ngx_http_get_module_loc_conf(r, ngx_http_data_vs_time_module);
  return *cglcf->cache->label_and_value_summary_json;
}

static ngx_str_t query_handler(ngx_http_request_t *r, uint32_t args_offset)
{
  uint32_t i, len;
  ngx_str_t result_body;

  ngx_str_t error_str = ngx_string("\"error\"");
  ngx_str_t series_spec_text = ngx_null_string;
  int64_t start = -1;
  int64_t stop = -1;
  int64_t step = -1;
  int64_t epoch = -1;
  int32_t include_series = 1;
  int32_t include_timestamps = 0;
  int32_t pretty = 0;

  ngx_http_data_vs_time_loc_conf_t *cglcf;
  cglcf = ngx_http_get_module_loc_conf(r, ngx_http_data_vs_time_module);

  result_body.data = NULL;
  result_body.len = 0;

  u_char *p_args = (u_char *)r->unparsed_uri.data + args_offset;

  char* args = ngx_palloc(r->pool, r->unparsed_uri.len - args_offset + 1);
  if (!args) {
    return result_body;
  }
  strncpy(args, (u_char *)p_args, r->unparsed_uri.len - args_offset);
  args[r->unparsed_uri.len - args_offset] = '\0';


  while (args != NULL)
  {
     char* arg = strsep(&args, "&");

     char* vname = strsep(&arg, "=");
     char* vvalue = arg;

     if (strncmp(vname, "start", sizeof("start") - 1) == 0)
     {
       start = strtoull(vvalue, (char **)NULL, 10);
     }
     else if (strncmp(vname, "stop", sizeof("stop") - 1) == 0)
     {
       stop = strtoull(vvalue, (char **)NULL, 10);
     }
     else if (strncmp(vname, "step", sizeof("step") - 1) == 0)
     {
       step = strtoull(vvalue, (char **)NULL, 10);
     }
     else if (strncmp(vname, "series", sizeof("series") - 1) == 0)
     {
       series_spec_text.data = vvalue;
       series_spec_text.len = strlen(vvalue);
     }
     else if (strncmp(vname, "epoch", sizeof("epoch") - 1) == 0)
     {
       epoch = strtoull(vvalue, (char **)NULL, 10);
     }
     else if (strncmp(vname, "info", sizeof("info") - 1) == 0)
     {
       len = strlen(vvalue);
       for (i=0; i<len; ++i)
       {
         switch (vvalue[i])
         {
           case 'S':
            include_series = 1;
            break;
           case 's':
            include_series = 0;
            break;
           case 'T':
            include_timestamps = 1;
            break;
           case 't':
            include_timestamps = 0;
            break;
           case 'P':
            pretty = 1;
            break;
         }
       }
     }
     else if (strncmp(vname, "aggregate", sizeof("aggregate") - 1) == 0)
     {
       // TODO: aggregate fnuctionality.
     }
     else
     {
       return result_body;
     }
  }

  ngx_array_t ra2;
  ngx_array_init(&ra2, r->pool, 32000, sizeof(u_char));


  if (series_spec_text.data == NULL)
  {
    return error_str;
  }

  uint32_t *ss = dvt_parse_series_spec_text(cglcf->cache, &series_spec_text, r->pool);
  if (ss == NULL)
  {
    dbg_cstr_nl("invalid series spec");
    return error_str;
  }
  ngx_array_t *sss = dvt_cache_get_series_set(cglcf->cache, ss, r->pool);

  ngx_array_t ra;
  ngx_array_init(&ra, r->pool, 32000, sizeof(u_char));

  ngx_push_to_string_array_cstr(&ra, "[");
  if (pretty)
  {
    ngx_push_to_string_array_cstr(&ra, "\n");
  }
  for (i=0; i<sss->nelts; ++i)
  {
    uint32_t *cur = ((uint32_t **)sss->elts)[i];

    if (pretty)
    {
      ngx_push_to_string_array_cstr(&ra, "  {\n");
    }
    else
    {
      ngx_push_to_string_array_cstr(&ra, "{");
    }
    int require_comma = 0;
    if (include_series)
    {
      if (pretty)
      {
        ngx_push_to_string_array_cstr(&ra, "    ");
      }
      ngx_push_to_string_array_cstr(&ra, "\"series\": ");
      ngx_push_to_string_array_ngx(&ra,
        dvt_series_spec_to_json(cglcf->cache, cur, r->pool) );
      require_comma = 1;
    }
    if (start >= 0 && stop > 0 && step >= 0)
    {
      if (require_comma == 1)
      {
        ngx_push_to_string_array_cstr(&ra, ",");
        if (pretty)
        {
          ngx_push_to_string_array_cstr(&ra, "\n");
        }
      }
      if (pretty)
      {
        ngx_push_to_string_array_cstr(&ra, "    ");
      }
      ngx_push_to_string_array_cstr(&ra, "\"values\": ");
      ngx_str_t vs = dvt_simulate_data(cglcf->cache, cur, start, stop, step, r->pool);
      ngx_push_to_string_array_ngx(&ra, vs);
      if (pretty)
      {
        ngx_push_to_string_array_cstr(&ra, "\n");
      }
    }
    else
    {
      if (pretty)
      {
        ngx_push_to_string_array_cstr(&ra, "\n");
      }
    }
    if (pretty)
    {
      ngx_push_to_string_array_cstr(&ra, "  ");
    }
    ngx_push_to_string_array_cstr(&ra, "}");
    if (i != sss->nelts - 1)
    {
      ngx_push_to_string_array_cstr(&ra, ",");
    }
    if (pretty)
    {
      ngx_push_to_string_array_cstr(&ra, "\n");
    }
  }
  ngx_push_to_string_array_cstr(&ra, "]");

  result_body = ngx_string_array_to_string(&ra);

  return result_body;

}

static ngx_int_t ngx_http_data_vs_time_handler(ngx_http_request_t *r)
{
  ngx_buf_t    *b;
  ngx_chain_t   out;
  ngx_str_t    result_body;

  if (strncmp((char *)r->uri.data,
            "/api/v1/query?",
     sizeof("/api/v1/query?")-1) == 0)
  {
    result_body = query_handler(r, sizeof("/api/v1/query?") -1 );
  }

  else if (strncmp((char *)r->uri.data,
             "/api/v1/label-and-value-summary",
      sizeof("/api/v1/label-and-value-summary")-1) == 0)
  {
    result_body = label_and_value_summary_handler(r);
  }

  else
  {
    return NGX_DECLINED;
  }

  if (result_body.len == 0)
  {
    return NGX_HTTP_INTERNAL_SERVER_ERROR;
  }

  r->headers_out.content_type.len = sizeof("application/json; charset=utf-8") - 1;
  r->headers_out.content_type.data = (u_char *) "application/json; charset=utf-8";

  b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));

  out.buf = b;
  out.next = NULL;

  b->pos = result_body.data;
  b->last = result_body.data + result_body.len;
  b->memory = 1;
  b->last_buf = 1;

  r->headers_out.status = NGX_HTTP_OK;
  r->headers_out.content_length_n = result_body.len;
  ngx_http_send_header(r);

  return ngx_http_output_filter(r, &out);
}

// Called when the data_vs_time command directive is encountered,
// as specified in the command array.
//
// a pointer to an ngx_conf_t struct, which contains the arguments passed to the directive
// a pointer to the current ngx_command_t struct
// a pointer to the module's custom configuration struct

static char *ngx_http_data_vs_time(
  ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_http_core_loc_conf_t  *clcf;
  ngx_http_data_vs_time_loc_conf_t *cglcf = conf;

  clcf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
  clcf->handler = ngx_http_data_vs_time_handler;

  cglcf->enable = 1;

  return NGX_CONF_OK;
}
