#include "dvt_cache.h"
#include "dvt_exposition.h"
#include "dvt_series_spec.h"
#include "dvt_trie.h"

typedef struct {
    ngx_str_t  label_value_name;
    uint16_t   series_count;
} dvt_label_value_summary_info_t;


typedef struct {
    ngx_str_t     label_name;
    ngx_array_t*  label_values;
} dvt_label_summary_info_t;


ngx_array_t* lavs_from_series_set(ngx_array_t* series_set, ngx_pool_t* pool)
{
  int i, j, k, found_loc;
  ngx_array_t* current_series;
  ngx_keyval_t* kv;
  ngx_str_t* label_name;
  ngx_str_t* label_value;
  ngx_str_t* tmp_str;

  dvt_label_summary_info_t* tmp_label_info;
  dvt_label_value_summary_info_t* tmp_label_value_info;

  ngx_array_t* label_names;
  ngx_array_t* labels;

  labels = ngx_array_create(pool, 32, sizeof(dvt_label_summary_info_t));

  for (i=0; i<series_set->nelts; ++i)
  {
    current_series = ((ngx_array_t**)series_set->elts)[i];
    for (j=0; j<current_series->nelts; ++j)
    {
      kv = &((ngx_keyval_t*)current_series->elts)[j];
      label_name = &kv->key;
      label_value = &kv->value;
      // key is label name, value is label value.

      // 1. find or create label info.
      found_loc = -1;
      for (k=0; k<labels->nelts; ++k)
      {
        tmp_str = &((dvt_label_summary_info_t *)labels->elts)[k].label_name;
        if (tmp_str->len != label_name->len)
	      {
	        continue;
	      }
	      if (ngx_strncmp(tmp_str->data, label_name->data, label_name->len) != 0)
	      {
	        continue;
	      }
	      found_loc = k;
	      break;
      }

      if (found_loc == -1)
      {
	      found_loc = labels->nelts;
	      tmp_label_info = (dvt_label_summary_info_t *)ngx_array_push(labels);
	      tmp_label_info->label_name.len = label_name->len;
	      tmp_label_info->label_name.data = ngx_palloc(pool, label_name->len);
	      ngx_cpymem(tmp_label_info->label_name.data, label_name->data, label_name->len);
	      tmp_label_info->label_values = ngx_array_create(
	         pool, 32, sizeof(dvt_label_value_summary_info_t));
      }

      tmp_label_info = &((dvt_label_summary_info_t *)labels->elts)[found_loc];

      // 2. find or create label value info.

      label_names = tmp_label_info->label_values;
      found_loc = -1;
      for (k = label_names->nelts-1; k >= 0; --k)
      {
	      tmp_str = &((dvt_label_value_summary_info_t *)label_names->elts)[k].label_value_name;
	      if (tmp_str->len != label_value->len)
	      {
	        continue;
	      }
	      if (ngx_strncmp(tmp_str->data, label_value->data, label_value->len) != 0)
	      {
	        continue;
	      }
	      found_loc = k;
	      break;
      }

      if (found_loc == -1)
      {
	      found_loc = label_names->nelts;
	      tmp_label_value_info = (dvt_label_value_summary_info_t *)ngx_array_push(label_names);
	      tmp_label_value_info->label_value_name.len = label_value->len;
	      tmp_label_value_info->label_value_name.data = ngx_palloc(pool, label_value->len);
	      ngx_cpymem(tmp_label_value_info->label_value_name.data, label_value->data, label_value->len);
	      tmp_label_value_info->series_count = 0;
      }

      tmp_label_value_info = &((dvt_label_value_summary_info_t *)label_names->elts)[found_loc];

      tmp_label_value_info->series_count += 1;
    }
  }

  return labels;
}


ngx_str_t *
lavs_to_json(ngx_array_t *lavs, ngx_pool_t *tmp_pool, ngx_pool_t* pool)
{
  int i, j, k;
  ngx_str_t *s_result;
  ngx_array_t *result;
  char* current;
  dvt_label_summary_info_t* tmp_label_info;
  dvt_label_value_summary_info_t* tmp_label_value_info;
  char tmp[32];

  s_result = ngx_palloc(pool, sizeof(ngx_str_t));
  result = ngx_array_create(tmp_pool, 1024, sizeof(char));

  current = (char *)ngx_array_push(result);
  *current = '{';

  for (i=0; i<lavs->nelts; ++i)
  {
    tmp_label_info = &((dvt_label_summary_info_t *)lavs->elts)[i];
    current = (char *)ngx_array_push(result);
    *current = '"';
    for (j=0; j<tmp_label_info->label_name.len; ++j)
    {
      current = (char *)ngx_array_push(result);
      *current = tmp_label_info->label_name.data[j];
    }
    current = (char *)ngx_array_push(result);
    *current = '"';
    current = (char *)ngx_array_push(result);
    *current = ':';
    current = (char *)ngx_array_push(result);
    *current = '{';

    for (j=0; j<tmp_label_info->label_values->nelts; ++j)
    {
      tmp_label_value_info
        = &((dvt_label_value_summary_info_t *)tmp_label_info->label_values->elts)[j];
      current = (char *)ngx_array_push(result);
      *current = '"';

      for (k=0; k<tmp_label_value_info->label_value_name.len; ++k)
      {
        current = (char *)ngx_array_push(result);
        *current = tmp_label_value_info->label_value_name.data[k];
      }
      current = (char *)ngx_array_push(result);
      *current = '"';
      current = (char *)ngx_array_push(result);
      *current = ':';
      current = (char *)ngx_array_push(result);
      *current = '"';
      sprintf(tmp, "%d", tmp_label_value_info->series_count);
      k = 0;
      while(tmp[k] != '\0')
      {
	      current = (char *)ngx_array_push(result);
	      *current = tmp[k++];
      }
      current = (char *)ngx_array_push(result);
      *current = '"';
      if (j != tmp_label_info->label_values->nelts-1) {
	      current = (char *)ngx_array_push(result);
	      *current = ',';
      }
    }

    current = (char *)ngx_array_push(result);
    *current = '}';
    if (i != lavs->nelts-1)
    {
      current = (char *)ngx_array_push(result);
      *current = ',';
    }
  }

  current = (char *)ngx_array_push(result);
  *current = '}';

  s_result->len = result->nelts;
  s_result->data = result->elts;

  current = ngx_palloc(pool, s_result->len);
  ngx_cpymem(current, s_result->data, s_result->len);
  s_result->data = current;

  return s_result;
}

static int dvt_cache_label_test(dvt_cache_t *cache, ngx_pool_t *pool);

#define ARBITRARY_OFFSET_TO_AVOID_PTR_EQ_NULL 100

dvt_cache_t *dvt_cache_init(ngx_pool_t *pool)
{
  dvt_cache_t *cache = ngx_palloc(pool, sizeof(dvt_cache_t));

  cache->label_and_value_summary_json = NGX_CONF_UNSET_PTR;
  cache->label_names = NGX_CONF_UNSET_PTR;
  cache->label_ids = NGX_CONF_UNSET_PTR;
  cache->label_value_names = NGX_CONF_UNSET_PTR;
  cache->label_value_ids = NGX_CONF_UNSET_PTR;

  uint64_t i, j;
  ngx_str_t *jsn = NGX_CONF_UNSET_PTR;
  ngx_array_t* series_collection;
  ngx_array_t* lavs; // labels and values
  ngx_str_t *current_str;
  ngx_array_t *hash_elts;
  ngx_hash_key_t *el;
  ngx_array_t **ael;
  ngx_hash_init_t hash_spec;

  // ### json summary.
  series_collection = load_test_exposition(pool);
  if (series_collection)
  {
    lavs = lavs_from_series_set(series_collection, pool);
    jsn = lavs_to_json(lavs, pool, pool);
  }
  cache->label_and_value_summary_json = jsn;

  // ### label_id -> label_names array
  cache->label_names = ngx_array_create(pool, 32, sizeof(ngx_str_t));
  for (i=0; i<lavs->nelts; ++i)
  {
    current_str = ngx_array_push(cache->label_names);
    *current_str = ((dvt_label_summary_info_t *)lavs->elts)[i].label_name;
  }

  // ### label_name -> label_id hash
  hash_elts = ngx_array_create(pool, 32, sizeof(ngx_hash_key_t));
  for (i=0; i<cache->label_names->nelts; ++i)
  {
    el = ngx_array_push(hash_elts);
    el->key = ((ngx_str_t *)cache->label_names->elts)[i];
    el->key_hash = ngx_hash_key(el->key.data, el->key.len);
    el->value = (void *)(i + ARBITRARY_OFFSET_TO_AVOID_PTR_EQ_NULL);
  }
  cache->label_ids = ngx_palloc(pool, sizeof(ngx_hash_t));
  hash_spec.hash = cache->label_ids;
  hash_spec.key = ngx_hash_key;
  hash_spec.max_size = 1024;
  hash_spec.bucket_size = ngx_align(64, ngx_cacheline_size);
  hash_spec.name = "label_ids";
  hash_spec.pool = pool;
  if (ngx_hash_init(&hash_spec, hash_elts->elts, hash_elts->nelts) != NGX_OK) {
    return NGX_CONF_ERROR;
  }

  // ### label_value_names
  cache->label_value_names = ngx_array_create(
    pool, cache->label_names->nelts, sizeof(ngx_array_t *));
  for (i=0; i<cache->label_names->nelts; ++i)
  {
    dvt_label_summary_info_t *labsum =
        &(((dvt_label_summary_info_t *)lavs->elts)[i]);
    ael = ngx_array_push(cache->label_value_names);
    *ael = ngx_array_create(
      pool, labsum->label_values->nelts, sizeof(ngx_str_t *));
    for (j=0; j<labsum->label_values->nelts; ++j)
    {
      dvt_label_value_summary_info_t* labval_info
        = &((dvt_label_value_summary_info_t *)labsum->label_values->elts)[j];
      ngx_str_t *lval
        = &labval_info->label_value_name;
      ngx_str_t **p = ngx_array_push(*ael);
      *p = lval;
    }
  }

  // ### label_value_ids
  cache->label_value_ids = ngx_array_create(
    pool, cache->label_names->nelts, sizeof(ngx_hash_t));
  for (i=0; i<cache->label_names->nelts; ++i)
  {
    hash_elts = ngx_array_create(pool, 32, sizeof(ngx_hash_key_t));
    ngx_array_t *value_names
      = ((ngx_array_t **)cache->label_value_names->elts)[i];
    for (j=0; j<value_names->nelts; ++j)
    {
      el = ngx_array_push(hash_elts);
      el->key = *((ngx_str_t **)value_names->elts)[j];
      el->key_hash = ngx_hash_key(el->key.data, el->key.len);
      el->value = (void *)(j + ARBITRARY_OFFSET_TO_AVOID_PTR_EQ_NULL);
    }

    ngx_hash_t *hel = ngx_array_push(cache->label_value_ids);
    hash_spec.hash = hel;
    hash_spec.key = ngx_hash_key;
    hash_spec.max_size = 1024;
    hash_spec.bucket_size = ngx_align(64, ngx_cacheline_size);
    hash_spec.name = "label_value_ids";
    hash_spec.pool = pool;
    if (ngx_hash_init(&hash_spec, hash_elts->elts, hash_elts->nelts) != NGX_OK) {
      return NGX_CONF_ERROR;
    }
  }

  // TODO: some sort of partitioning.
  // Probably binary.
  // Goal is to keep as balanced as possible.
  // ### all series
  ngx_array_init(&cache->all_series, pool, 128, sizeof(uint32_t *));
  for (i=0; i<series_collection->nelts; ++i)
  {
    ngx_array_t *series_kvps = ((ngx_array_t **)series_collection->elts)[i];
    uint32_t **loc = ngx_array_push(&cache->all_series);
    *loc = dvt_series_spec_from_kv_array(cache, series_kvps, pool);
    dbg_ngx(dvt_series_spec_to_text(cache, *loc, pool), pool);
  }

  return cache;
}

ngx_array_t *dvt_cache_get_series_set(dvt_cache_t *cache, uint32_t* set_spec, ngx_pool_t *pool)
{
  ngx_array_t *r;
  uint32_t **nw;
  uint32_t *c;
  uint32_t i;

  r = ngx_array_create(pool, 32, sizeof(uint32_t *));

  for (i=0; i<cache->all_series.nelts; ++i)
  {
    c = ((uint32_t **)cache->all_series.elts)[i];
    if (dvt_series_in_set(set_spec, c))
    {
      nw = ngx_array_push(r);
      *nw = c;
    }
  }

  return r;
}

uint32_t dvt_cache_label_id_from_name(dvt_cache_t *cache, ngx_str_t *name)
{
  ngx_uint_t key_hash = ngx_hash_key(name->data, name->len);
  int64_t r = (uint64_t)ngx_hash_find(cache->label_ids, key_hash, name->data, name->len);
  return (uint32_t) r - ARBITRARY_OFFSET_TO_AVOID_PTR_EQ_NULL;
}

ngx_str_t *dvt_cache_label_name_from_id(dvt_cache_t *cache, uint32_t id)
{
  return &((ngx_str_t *)cache->label_names->elts)[id];
}

uint32_t dvt_cache_label_value_id_from_name(dvt_cache_t *cache, uint32_t label_id, ngx_str_t *name)
{
  ngx_uint_t key_hash = ngx_hash_key(name->data, name->len);
  ngx_hash_t *sh = &(((ngx_hash_t *)cache->label_value_ids->elts)[label_id]);
  int64_t r = (uint64_t)ngx_hash_find(sh, key_hash, name->data, name->len) - ARBITRARY_OFFSET_TO_AVOID_PTR_EQ_NULL;
  return (uint32_t)r;
}

ngx_str_t *dvt_cache_label_value_name_from_id(dvt_cache_t *cache, uint32_t label_id, uint32_t id)
{
  //dbg_int(label_id);
  //dbg_int(id);
  ngx_array_t *value_names
    = ((ngx_array_t **)cache->label_value_names->elts)[label_id];
  return ((ngx_str_t **)value_names->elts)[id];
}


uint32_t dvt_number_of_labels(dvt_cache_t *cache)
{
  return cache->label_names->nelts;
}

static int dvt_cache_label_test(dvt_cache_t *cache, ngx_pool_t *pool)
{
  uint32_t i, j;
  for (i=0; i<dvt_number_of_labels(cache); ++i)
  {
    ngx_str_t *s = dvt_cache_label_name_from_id(cache, i);
    j = dvt_cache_label_id_from_name(cache, s);
    //if (i != j)
    //{
    //  return 1;
    //}
    dbg_int(j);
  }

  // TODO.
}
