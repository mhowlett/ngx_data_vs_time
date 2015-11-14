#include <string.h>

#include "ngx_string_utils.h"

#include "dvt_series_spec.h"
#include "dvt_cache.h"
#include "dvt_debug.h"


#define OVERFLOW_CHECK \
  if (--safety_count <= 0 || pos > buffer_end) { goto invalid; }

// http://www.quora.com/Is-it-safe-to-use-a-colon-in-the-path-of-a-URL
// -> : and ' and () are all valid in a url.

static ngx_array_t *
dvt_parse_series_spec_text_str(ngx_str_t* s, ngx_pool_t *pool)
{
  ngx_str_t errmsg = ngx_null_string;
  ngx_array_t* result;
  u_char* buffer_end;
  u_char* token_start_pos;
  u_char* pos;
  ngx_keyval_t* current;
  int safety_count = 10000;
  result = ngx_array_create(pool, 4, sizeof(ngx_keyval_t));

  buffer_end = s->data + s->len;
  pos = s->data;

start:
  OVERFLOW_CHECK
  if (*pos == '{')
  {
    ++pos;
    token_start_pos = pos;
    current = (ngx_keyval_t *)ngx_array_push(result);
    goto reading_label;
  }
  ngx_str_set(&errmsg, "unexpected character, start");
  goto error;

reading_label:
  OVERFLOW_CHECK
  if ( (*pos >= 'a' && *pos <= 'z') ||
       (*pos >= 'A' && *pos <= 'Z') ||
       (*pos == '_') ||
       (*pos == '.') ||
       (*pos >= '0' && *pos <= '9'))
  {
    ++pos;
    goto reading_label;
  }
  if (*pos == ':')
  {
    current->key.len = pos - token_start_pos;
    current->key.data = token_start_pos;
    ++pos;
    goto reading_open_quote;
  }
  ngx_str_set(&errmsg, "unexpected character, reading_label");
  goto error;

reading_open_quote:
  OVERFLOW_CHECK
  if (*pos == '\'')
  {
    ++pos;
    token_start_pos = pos;
    goto reading_value;
  }
  if (*pos == '%')
  {
    ++pos;
    if (*pos == '2')
    {
      ++pos;
      if (*pos == '7')
      {
        ++pos;
        token_start_pos = pos;
        goto reading_value;
      }
      ngx_str_set(&errmsg, "unexpected character, reading_open_quote (1)");
      goto error;
    }
    ngx_str_set(&errmsg, "unexpected character, reading_open_quote (2)");
    goto error;
  }
  ngx_str_set(&errmsg, "unexpected character, reading_open_quote (3)");
  goto error;

reading_value:
  OVERFLOW_CHECK
  if ( (*pos >= 'a' && *pos <= 'z') ||
       (*pos >= 'A' && *pos <= 'Z') ||
       (*pos == '_') ||
       (*pos == '.') ||
       (*pos >= '0' && *pos <= '9'))
  {
    ++pos;
    goto reading_value;
  }
  if (*pos == '\'')
  {
    current->value.len = pos - token_start_pos;
    current->value.data = token_start_pos;
    ++pos;
    goto reading_comma_or_end;
  }
  if (*pos == '%')
  {
    ++pos;
    if (*pos == '2')
    {
      ++pos;
      if (*pos == '7')
      {
        current->value.len = pos - 2 - token_start_pos;
        current->value.data = token_start_pos;
        ++pos;
        goto reading_comma_or_end;
      }
      ngx_str_set(&errmsg, "unexpected character, reading_value (1)");
      goto error;
    }
    ngx_str_set(&errmsg, "unexpected character, reading_value (2)");
    goto error;
  }
  ngx_str_set(&errmsg, "unexpected character, reading_value (3)");
  goto error;

reading_comma_or_end:
  OVERFLOW_CHECK
  if (*pos == ',')
  {
    ++pos;
    token_start_pos = pos;
    current = (ngx_keyval_t *)ngx_array_push(result);
    goto reading_label;
  }
  if (*pos == '}')
  {
    ++pos;
    goto completed;
  }
  ngx_str_set(&errmsg, "unexpected character, reading_comma_or_end");
  goto error;

invalid:
  ngx_str_set(&errmsg, "invalid format");

error:
  dbg_cstr_nl("Error parsing series spec:");
  dbg_ngx(errmsg, pool);
  return NULL;

completed:
  return result;
}

// http://www.quora.com/Is-it-safe-to-use-a-colon-in-the-path-of-a-URL
// -> : and ' and () are all valid in a url.

ngx_str_t dvt_series_spec_to_text(dvt_cache_t *cache, uint32_t *spec, ngx_pool_t *pool)
{
  // TODO: something more efficient.
  // If this is happening in a request, there may be very little cost in
  // allocating more memory than we are likely to need.

  int i;
  ngx_array_t *result;
  result = ngx_array_create(pool, 128, sizeof(u_char));
  u_char *current;

  if (dvt_series_spec_metric_value_id(spec) != UINT32_MAX)
  {
    ngx_push_to_string_array_cstr(result, "{metric:'");
    ngx_push_to_string_array_ngx_ptr(result, dvt_cache_label_value_name_from_id(cache, 0, dvt_series_spec_metric_value_id(spec)));
    ngx_push_to_string_array_cstr(result, "',");
  }
  else
  {
    ngx_push_to_string_array_cstr(result, "{");
  }

  for (i=0; i<dvt_series_spec_label_count(spec); ++i)
  {
    ngx_push_to_string_array_ngx_ptr(result, dvt_cache_label_name_from_id(cache, dvt_series_spec_label_id(spec, i)));
    ngx_push_to_string_array_cstr(result, ":'");
    ngx_push_to_string_array_ngx_ptr(result, dvt_cache_label_value_name_from_id(cache, dvt_series_spec_label_id(spec, i), dvt_series_spec_value_id(spec, i)));
    ngx_push_to_string_array_cstr(result, "'");
    ngx_push_to_string_array_cstr(result, ",");
  }

  ((u_char *)result->elts)[result->nelts-1] = '}';

  return ngx_string_array_to_string(result);
}

ngx_str_t dvt_series_spec_to_text_raw(uint32_t *spec, ngx_pool_t *pool)
{
  char tmp_str[32];
  ngx_array_t result;

  //snprintf(tmp_str, sizeof(tmp_str), "\"%d\"", d);
  //p = ngx_cpymem(p, tmp_str, strlen(tmp_str));

  // TODO: not finished!

  return ngx_string_array_to_string(&result);
}

ngx_str_t dvt_series_spec_to_json(dvt_cache_t *cache, uint32_t *spec, ngx_pool_t *pool)
{
  // TODO: something more efficient.
  // If this is happening in a request, there may be very little cost in
  // allocating more memory than we are likely to need.

  int i;
  ngx_array_t *result;
  result = ngx_array_create(pool, 128, sizeof(u_char));
  u_char *current;

  if (dvt_series_spec_metric_value_id(spec) != UINT32_MAX)
  {
    ngx_push_to_string_array_cstr(result, "{\"metric\":\"");
    ngx_push_to_string_array_ngx_ptr(result, dvt_cache_label_value_name_from_id(cache, 0, dvt_series_spec_metric_value_id(spec)));
    ngx_push_to_string_array_cstr(result, "\",");
  }
  else
  {
    ngx_push_to_string_array_cstr(result, "{");
  }

  for (i=0; i<dvt_series_spec_label_count(spec); ++i)
  {
    ngx_push_to_string_array_cstr(result, "\"");
    ngx_push_to_string_array_ngx_ptr(result, dvt_cache_label_name_from_id(cache, dvt_series_spec_label_id(spec, i)));
    ngx_push_to_string_array_cstr(result, "\":\"");
    ngx_push_to_string_array_ngx_ptr(result, dvt_cache_label_value_name_from_id(cache, dvt_series_spec_label_id(spec, i), dvt_series_spec_value_id(spec, i)));
    ngx_push_to_string_array_cstr(result, "\"");
    ngx_push_to_string_array_cstr(result, ",");
  }

  ((u_char *)result->elts)[result->nelts-1] = '}';

  return ngx_string_array_to_string(result);
}

ngx_int_t dvt_kv_cmp(const void *first, const void* second)
{
  // just compare keys, value is ignored.
  if (*((uint32_t *)first) < *((uint32_t *)second)) { return -1; }
  if (*((uint32_t *)first) == *((uint32_t *)second)) { return 0; }
  return 1;
}

uint32_t *
dvt_parse_series_spec_text(dvt_cache_t *cache, ngx_str_t* s, ngx_pool_t* pool)
{
  ngx_array_t *str_spec = dvt_parse_series_spec_text_str(s, pool);
  if (str_spec == NULL)
  {
    return NULL;
  }
  return dvt_series_spec_from_kv_array(cache, str_spec, pool);
}

uint32_t *
dvt_series_spec_from_kv_array(dvt_cache_t *cache, ngx_array_t *kvps, ngx_pool_t* pool)
{
  uint32_t *result;
  uint32_t i;
  uint32_t additional_label_cnt = 0;

  // +1 si required in the case there is not metric value in the kv array.
  // TODO: really don't want to allocate this.
  result = ngx_palloc(pool, sizeof(uint32_t) * (kvps->nelts+1) * 2);
  dvt_series_spec_metric_value_id(result) = UINT32_MAX;

  for (i=0; i<kvps->nelts; ++i)
  {
    ngx_keyval_t *kvp = &((ngx_keyval_t *)kvps->elts)[i];
    uint32_t label_id = dvt_cache_label_id_from_name(cache, &kvp->key);
    uint32_t value_id = dvt_cache_label_value_id_from_name(cache, label_id, &kvp->value);

    if (label_id == metric_label_id)
    {
      dvt_series_spec_metric_value_id(result) = value_id;
      continue;
    }
    dvt_series_spec_label_id(result, additional_label_cnt) = label_id;
    dvt_series_spec_value_id(result, additional_label_cnt) = value_id;
    additional_label_cnt += 1;
  }
  dvt_series_spec_label_count(result) = additional_label_cnt;

  // ngx_sort is inserion sort, which is perfect because there will be a
  // relatively low number of items here.
  ngx_sort(dvt_series_spec_kvs(result), dvt_series_spec_label_count(result), 2*sizeof(uint32_t), dvt_kv_cmp);

  return result;
}

ngx_str_t dvt_series_spec_label_value(dvt_cache_t *cache, uint32_t *spec, ngx_str_t *name)
{
  uint32_t i;
  uint32_t lid = dvt_cache_label_id_from_name(cache, name);
  uint32_t mid = UINT32_MAX;
  if (lid == 0)
  {
    mid = dvt_series_spec_metric_value_id(spec);
  }
  else
  {
    for (i=0; i<dvt_series_spec_label_count(spec); ++i)
    {
      if (dvt_series_spec_label_id(spec, i) == lid)
      {
        mid = dvt_series_spec_value_id(spec, i);
        break;
      }
    }
  }

  if (mid == UINT32_MAX)
  {
    ngx_str_t nl;
    nl.len = 0;
  }

  return *dvt_cache_label_value_name_from_id(cache, lid, mid);
}


uint32_t dvt_series_in_set(uint32_t *set, uint32_t *spec)
{
  uint32_t i, j, k;

  if (dvt_series_spec_metric_value_id(set) != UINT32_MAX)
  {
    if (dvt_series_spec_metric_value_id(spec) != dvt_series_spec_metric_value_id(set))
    {
      return 0;
    }
  }

  j = 0;
  for (i=0; i<dvt_series_spec_label_count(set); ++i)
  {
    k = dvt_series_spec_label_id(set, i);
    while (dvt_series_spec_label_id(spec, j) < k && j<dvt_series_spec_label_count(spec))
    {
      j += 1;
    }
    if (j == dvt_series_spec_label_count(spec))
    {
      return 0;
    }
    if (dvt_series_spec_value_id(set, i) != dvt_series_spec_value_id(spec, j))
    {
      return 0;
    }
  }

  return 1;
}
