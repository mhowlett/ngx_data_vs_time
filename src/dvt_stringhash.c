
#include "dvt_stringhash.h"

#define DVT_STRINGHASH_ELT_SIZE(name)                                          \
    (sizeof(void *) + ngx_align((name)->key.len + 2, sizeof(void *)))

ngx_int_t
dvt_stringhash_init(dvt_stringhash_init_t *hinit, dvt_stringhash_key_val_t *names, ngx_uint_t nelts)
{
    u_char          *elts;
    size_t           len;
    u_short         *test;
    ngx_uint_t       i, n, key, size, start, bucket_size;
    dvt_stringhash_elt_t  *elt, **buckets;

    for (n = 0; n < nelts; ++n) {
        if (hinit->bucket_size < DVT_STRINGHASH_ELT_SIZE(&names[n]) + sizeof(void *))
        {
            ngx_log_error(NGX_LOG_EMERG, hinit->pool->log, 0,
                          "could not build the %s, you should "
                          "increase %s_bucket_size: %i",
                          hinit->name, hinit->name, hinit->bucket_size);
            return NGX_ERROR;
        }
    }

    test = ngx_alloc(hinit->max_size * sizeof(u_short), hinit->pool->log);
    if (test == NULL) {
        return NGX_ERROR;
    }

    bucket_size = hinit->bucket_size - sizeof(void *);

    start = nelts / (bucket_size / (2 * sizeof(void *)));
    start = start ? start : 1;

    if (hinit->max_size > 10000 && nelts && hinit->max_size / nelts < 100) {
        start = hinit->max_size - 1000;
    }

    for (size = start; size < hinit->max_size; ++size) {

        ngx_memzero(test, size * sizeof(u_short));

        for (n = 0; n < nelts; ++n) {
            if (names[n].key.data == NULL) {
                continue;
            }

            key = names[n].key_hash % size;
            test[key] = (u_short) (test[key] + DVT_STRINGHASH_ELT_SIZE(&names[n]));

#if 0
            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                          "%ui: %ui %ui \"%V\"",
                          size, key, test[key], &names[n].key);
#endif

            if (test[key] > (u_short) bucket_size) {
                goto next;
            }
        }

        goto found;

    next:

        continue;
    }

    ngx_log_error(NGX_LOG_EMERG, hinit->pool->log, 0,
                  "could not build the %s, you should increase "
                  "either %s_max_size: %i or %s_bucket_size: %i",
                  hinit->name, hinit->name, hinit->max_size,
                  hinit->name, hinit->bucket_size);

    ngx_free(test);

    return NGX_ERROR;

found:

    for (i = 0; i < size; ++i) {
        test[i] = sizeof(void *);
    }

    for (n = 0; n < nelts; ++n) {
        if (names[n].key.data == NULL) {
            continue;
        }

        key = names[n].key_hash % size;
        test[key] = (u_short) (test[key] + DVT_STRINGHASH_ELT_SIZE(&names[n]));
    }

    len = 0;

    for (i = 0; i < size; ++i) {
        if (test[i] == sizeof(void *)) {
            continue;
        }

        test[i] = (u_short) (ngx_align(test[i], ngx_cacheline_size));

        len += test[i];
    }


    buckets = ngx_pcalloc(hinit->pool, size * sizeof(dvt_stringhash_elt_t *));
    if (buckets == NULL) {
        ngx_free(test);
        return NGX_ERROR;
    }

    elts = ngx_palloc(hinit->pool, len + ngx_cacheline_size);
    if (elts == NULL) {
        ngx_free(test);
        return NGX_ERROR;
    }

    elts = ngx_align_ptr(elts, ngx_cacheline_size);

    for (i = 0; i < size; ++i) {
        if (test[i] == sizeof(void *)) {
            continue;
        }

        buckets[i] = (dvt_stringhash_elt_t *) elts;
        elts += test[i];
    }

    for (i = 0; i < size; ++i) {
        test[i] = 0;
    }

    for (n = 0; n < nelts; ++n) {
        if (names[n].key.data == NULL) {
            continue;
        }

        key = names[n].key_hash % size;
        elt = (dvt_stringhash_elt_t *) ((u_char *) buckets[key] + test[key]);

        elt->value = names[n].value;
        elt->len = (u_short) names[n].key.len;

        ngx_strlow(elt->name, names[n].key.data, names[n].key.len);

        test[key] = (u_short) (test[key] + DVT_STRINGHASH_ELT_SIZE(&names[n]));
    }

    for (i = 0; i < size; ++i) {
        if (buckets[i] == NULL) {
            continue;
        }

        elt = (dvt_stringhash_elt_t *) ((u_char *) buckets[i] + test[i]);

        elt->value = NULL;
    }

    ngx_free(test);

    hinit->hash->buckets = buckets;
    hinit->hash->size = size;

#if 0

    for (i = 0; i < size; i++) {
        ngx_str_t   val;
        ngx_uint_t  key;

        elt = buckets[i];

        if (elt == NULL) {
            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                          "%ui: NULL", i);
            continue;
        }

        while (elt->value) {
            val.len = elt->len;
            val.data = &elt->name[0];

            key = hinit->key(val.data, val.len);

            ngx_log_error(NGX_LOG_ALERT, hinit->pool->log, 0,
                          "%ui: %p \"%V\" %ui", i, elt, &val, key);

            elt = (ngx_hash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len,
                                                   sizeof(void *));
        }
    }

#endif

    return NGX_OK;
}


void *dvt_stringhash_find(dvt_stringhash_t *hash, ngx_uint_t key, u_char *name, size_t len)
{
    ngx_uint_t       i;
    dvt_stringhash_elt_t  *elt;

#if 0
    ngx_log_error(NGX_LOG_ALERT, ngx_cycle->log, 0, "hf:\"%*s\"", len, name);
#endif

    elt = hash->buckets[key % hash->size];

    if (elt == NULL) {
        return NULL;
    }

    while (elt->value) {
        if (len != (size_t) elt->len) {
            goto next;
        }

        for (i = 0; i < len; i++) {
            if (name[i] != elt->name[i]) {
                goto next;
            }
        }

        return elt->value;

    next:

        elt = (dvt_stringhash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len,
                                               sizeof(void *));
        continue;
    }

    return NULL;
}


ngx_array_t *dvt_stringhash_to_array(dvt_stringhash_t* hash, ngx_pool_t* pool)
{
  ngx_array_t* result;
  dvt_stringhash_elt_t  *elt;
  int i;
  int* v;

  result = ngx_array_create(pool, 32, sizeof(int));
  
  for (i = 0; i<hash->size; ++i) {

    if (hash->buckets[i] == NULL)
    {
      continue;
    }

    while(elt->value) {
     v = ngx_array_push(result);
     *v = *((int *)elt->value);

     elt = (dvt_stringhash_elt_t *) ngx_align_ptr(&elt->name[0] + elt->len,
                                               sizeof(void *));
    }
  }
 
  return result;
}


/* /\* */
/* static void make_hash(ngx_array_t* series_set, ngx_pool_t* pool) */
/* { */
/*   dvt_stringhash_t hash; */
/*   dvt_stringhash_init_t hash_spec; */
/*   ngx_array_t* elts; */
/*   ngx_array_t* elts2; */
/*   dvt_stringhash_key_val_t* el; */
/*   ngx_str_t tmp; */
/*   int* r; */

/*   elts = ngx_array_create(pool, 32, sizeof(dvt_stringhash_key_val_t)); */

/*   //  el = ngx_array_push(elts); */
/*   // ngx_str_set(&el->key, "hello key"); */
/*   // el->key_hash = ngx_hash_key(el->key.data, el->key.len); */
/*  //el->value = ngx_palloc(pool, sizeof(int)); */
/*  // *((int *)el->value) = 13; */

/*   //  el = ngx_array_push(elts); */
/*   // ngx_str_set(&el->key, "the key number 2"); */
/*   //  el->key_hash = ngx_hash_key(el->key.data, el->key.len); */
/*   // el->value = ngx_palloc(pool, sizeof(int)); */
/*   //\*((int *)el->value) = 42; */

/*   // el = ngx_array_push(elts); */
/*   //ngx_str_set(&el->key, "the key number 3"); */
/*   //el->key_hash = ngx_hash_key(el->key.data, el->key.len); */
/*   //el->value = ngx_palloc(pool, sizeof(int)); */
/*   //\*((int *)el->value) = 41; */

/*   hash_spec.hash = &hash; */
/*   hash_spec.key_gen = ngx_hash_key; */
/*   hash_spec.max_size = 512; */
/*   hash_spec.bucket_size = ngx_align(64, ngx_cacheline_size); */
/*   hash_spec.name = "ubute_hash"; */
/*   hash_spec.pool = pool; */

/*   if (dvt_stringhash_init(&hash_spec, elts->elts, elts->nelts) != NGX_OK) { */
/*     return; // NGX_ERROR; */
/*   } */

/*   dbgint(hash.size); */

/*   ngx_str_set(&tmp, "hello key"); */
/*   r = (int *)dvt_stringhash_find(&hash,  */
/*     ngx_hash_key(tmp.data, tmp.len), */
/*     tmp.data, tmp.len); */

/*   dbgint(*r); */

/*   elts2 = dvt_stringhash_to_array(&hash, pool); */
/*   dbgint(elts2->nelts); */
/* } */
/* *\/ */


