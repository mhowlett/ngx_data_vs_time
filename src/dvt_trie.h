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
