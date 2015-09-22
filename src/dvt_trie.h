#ifndef __DVT_TRIE_H
#define __DVT_TRIE_H

// this is all rubbish.

/*
#include <ngx_core.h>
#include "dvt_series_spec.h"

typedef struct dvt_trie_node_s
{
  uint32_t inbound_label_id;
  uint32_t inbound_value_id;
  ngx_array_t series;   // of uint32_t* : series in the set defined by the node.
  ngx_array_t children; // of dvt_trie_node_t*
} dvt_trie_node_t;

void dvt_trie_init(dvt_trie_node_t *root, ngx_pool_t *pool);
void dvt_trie_insert(dvt_trie_node_t *root, uint32_t *series, ngx_pool_t *pool);
ngx_str_t dvt_trie_to_string(dvt_trie_node_t *node, ngx_pool_t *pool);

// return array type uint32_t*
ngx_array_t *dvt_trie_get_series_specs(dvt_trie_node_t *root, uint32_t *spec);
*/

#endif
