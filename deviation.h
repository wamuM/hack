#ifndef DEVIATION_H
#define DEVIATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "node.h"      /* was "struct_node.h" — that file does not exist */
#include "graph.h"
#include "path.h"

/*
 * Result of evaluating one user-supplied region name against the solution path.
 *
 * INCORRECT_NODE   — the node is not on the path and its BFS distance to every
 *                    path node is > 1.
 * ALMOST_PATH_NODE — the node is not on the path but there exists at least one
 *                    path node reachable in exactly 1 hop (i.e. it is a direct
 *                    neighbour of some path node).
 * PATH_NODE        — the node is on the correct solution path.
 *
 * Numeric values are preserved so that higher == better, which is useful for
 * any scoring logic in the UI layer.
 */
typedef enum {
    INCORRECT_NODE   = 0,
    ALMOST_PATH_NODE = 1,
    PATH_NODE        = 2
} Deviation_case;

/*
 * Resolves a region name to its index in the graph.
 * Returns -1 if the name is not found.
 */
int find_node_index_by_name(const graph *g, const char *name);

/*
 * Returns 1 if `index` appears in path_indices[0..path_size-1], 0 otherwise.
 */
int region_in_path(int index, const int *path_indices, int path_size);

/*
 * Checks whether any neighbour of `index` in `g` is a path node.
 * Returns 1 if such a neighbour exists, 0 otherwise.
 */
int is_almost_path_node(const graph *g, int index,
                        const int *path_indices, int path_size);

/*
 * Classifies a single node index against the solution path.
 *
 * Pre:
 *   g            — valid graph pointer
 *   index        — valid node index in g  (0 <= index < g->node_len)
 *   path_indices — array of node indices forming the solution path
 *   path_size    — length of path_indices
 *
 * Post:
 *   Returns PATH_NODE        if index is in path_indices.
 *   Returns ALMOST_PATH_NODE if index is not in the path but shares an edge
 *                             with at least one node that is.
 *   Returns INCORRECT_NODE   otherwise.
 */
Desviation_case classify_node(const graph *g, int index,
                               const int *path_indices, int path_size);

/*
 * Evaluates an entire array of user-supplied region names.
 *
 * Pre:
 *   g            — valid graph pointer
 *   user_regions — array of region name strings supplied by the user
 *   region_count — number of strings in user_regions
 *   path_indices — array of node indices forming the solution path
 *   path_size    — length of path_indices
 *   out_results  — caller-allocated array of length region_count; each entry
 *                  receives the Desviation_case for the corresponding name.
 *                  Unknown names receive INCORRECT_NODE.
 *
 * Post:
 *   Fills out_results[0..region_count-1].
 *   Returns 0 on success, -1 if any argument is NULL or region_count <= 0.
 */
int evaluate_user_regions(const graph *g,
                          const char **user_regions, int region_count,
                          const int  *path_indices,  int path_size,
                          Desviation_case *out_results);

#endif /* DEVIATION_H */
