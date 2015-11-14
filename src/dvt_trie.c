#include "dvt_trie.h"
#include "ngx_string_utils.h"

// this is all rubbish.

/*
// TODO:
//   series lists probably don't need to be stored in memory for all
//   nodes. They should be kept on disk and loaded into memory when
//   needed in LRU fashion.

void dvt_trie_init(dvt_trie_node_t *node, ngx_pool_t *pool)
{
  node->inbound_label_id = UINT32_MAX;
  node->inbound_value_id = UINT32_MAX;
  ngx_array_init(&node->series, pool, 1, sizeof(uint32_t *));
  ngx_array_init(&node->children, pool, 1, sizeof(dvt_trie_node_t *));
}

static void dvt_trie_insert_at(
  dvt_trie_node_t *node,
  uint32_t *series,
  uint32_t current_index,
  ngx_pool_t *pool)
{
  int i, j;

  // 1. place series set in our series list (unless root)
  if (current_index != 0)
  {
    uint32_t **new_series_element = ngx_array_push(&node->series);
    *new_series_element = series;
  }

  // 2. if current is the last element of the series, we're done.
  // last label is current_index - 1 but +1 for mertic.
  if (current_index == dvt_series_spec_label_count(series))
  {
    return;
  }

  for (j=current_index; j<=dvt_series_spec_label_count(series); ++j)
  {
    uint32_t label_id;
    uint32_t value_id;
    if (j == 0)
    {
      label_id = metric_label_id;
      value_id = dvt_series_spec_metric_value_id(series);
    }
    else
    {
      label_id = dvt_series_spec_label_id(series, j-1);
      value_id = dvt_series_spec_value_id(series, j-1);
    }

    // 3. try to find next label of series in branch list and recurse.

    // TODO:
    //   1. keep children sorted, and
    //   2. do a binary search to find and
    //   3. probably two list levels - 1=label_id, 2=value_id
    int done = 0;
    for (i=0; i<node->children.nelts; ++i)
    {
      dvt_trie_node_t *c = ((dvt_trie_node_t **)node->children.elts)[i];
      if (c->inbound_label_id == label_id && c->inbound_value_id == value_id)
      {
        dvt_trie_insert_at(c, series, j + 1, pool);
        done = 1;
        break;
      }
    }
    if (done)
    {
      continue;
    }

    // 4. else not found, add appropriate child and recurse.
    dvt_trie_node_t **new_node = ngx_array_push(&node->children);
    *new_node = ngx_palloc(pool, sizeof(dvt_trie_node_t));
    dvt_trie_init(*new_node, pool);
    (*new_node)->inbound_label_id = label_id;
    (*new_node)->inbound_value_id = value_id;

    dvt_trie_insert_at(*new_node, series, j + 1, pool);
  }
}

void dvt_trie_insert(dvt_trie_node_t *root, uint32_t *series, ngx_pool_t *pool)
{
  dvt_trie_insert_at(root, series, 0, pool);
}

ngx_str_t dvt_trie_to_string(dvt_trie_node_t *node, ngx_pool_t *pool)
{
  ngx_str_t r = ngx_string("todo");
  return r;
}

ngx_array_t *
dvt_trie_get_series_specs(dvt_trie_node_t *node, uint32_t* spec)
{
  int i, j;

  dvt_trie_node_t *c_o = node;
  for (i=0; i<=dvt_series_spec_label_count(spec); ++i)
  {
    dbg_int(i);
    uint32_t label_id;
    uint32_t value_id;
    if (i == 0)
    {
      label_id = metric_label_id;
      value_id = dvt_series_spec_metric_value_id(spec);
      if (value_id == UINT32_MAX)
      {
        continue;
      }
    }
    else
    {
      label_id = dvt_series_spec_label_id(spec, i-1);
      value_id = dvt_series_spec_value_id(spec, i-1);
    }

    int found = 0;
    for (j=0; j<c_o->children.nelts; ++j)
    {
      dvt_trie_node_t *c = ((dvt_trie_node_t **)node->children.elts)[j];
      if (c->inbound_label_id == label_id && c->inbound_value_id == value_id)
      {
        found = 1;
        c_o = c;
        break;
      }
    }
    if (!found)
    {
      dbg_int(42);
      return NULL;
    }
  }

  return &c_o->series;
}

*/
