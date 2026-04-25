#include "deviation.h"

/* ------------------------------------------------------------------
 * find_node_index_by_name
 * ------------------------------------------------------------------ */
int find_node_index_by_name(const graph *g, const char *name)
{
    if (g == NULL || name == NULL) return -1;

    for (int i = 0; i < g->node_len; i++) {
        if (g->nodes[i].name != NULL &&
            strcmp(g->nodes[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

/* ------------------------------------------------------------------
 * region_in_path
 * ------------------------------------------------------------------ */
int region_in_path(int index, const int *path_indices, int path_size)
{
    for (int i = 0; i < path_size; i++) {
        if (path_indices[i] == index) return 1;
    }
    return 0;
}

/* ------------------------------------------------------------------
 * is_almost_path_node
 *
 * A node qualifies as ALMOST_PATH_NODE when it is NOT itself on the
 * path but at least one of its direct neighbours IS on the path —
 * i.e. BFS distance to the path is exactly 1.
 * ------------------------------------------------------------------ */
int is_almost_path_node(const graph *g, int index,
                        const int *path_indices, int path_size)
{
    if (g == NULL || path_indices == NULL) return 0;

    const node *n = &g->nodes[index];

    for (int i = 0; i < n->incidence_cnt; i++) {
        int nb = n->incidence_list[i];
        if (region_in_path(nb, path_indices, path_size)) {
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------
 * classify_node
 * ------------------------------------------------------------------ */
Desviation_case classify_node(const graph *g, int index,
                               const int *path_indices, int path_size)
{
    /* Case 1 — node is part of the correct solution path */
    if (region_in_path(index, path_indices, path_size)) {
        return PATH_NODE;
    }

    /* Case 2 — node is not on the path but is adjacent to a path node
     *          (BFS distance == 1)                                     */
    if (is_almost_path_node(g, index, path_indices, path_size)) {
        return ALMOST_PATH_NODE;
    }

    /* General case — not on the path and no direct connection to it */
    return INCORRECT_NODE;
}

/* ------------------------------------------------------------------
 * evaluate_user_regions
 *
 * Iterates over an array of region name strings, resolves each one to
 * a graph node index, then classifies it.  Unknown names (not found in
 * the graph) are silently treated as INCORRECT_NODE.
 * ------------------------------------------------------------------ */
int evaluate_user_regions(const graph *g,
                          const char **user_regions, int region_count,
                          const int  *path_indices,  int path_size,
                          Desviation_case *out_results)
{
    if (g == NULL || user_regions == NULL || region_count <= 0 ||
        path_indices == NULL || path_size <= 0 || out_results == NULL) {
        return -1;
    }

    for (int i = 0; i < region_count; i++) {
        int idx = find_node_index_by_name(g, user_regions[i]);

        if (idx == -1) {
            /* Name not found in the graph — treat as wrong */
            out_results[i] = INCORRECT_NODE;
            continue;
        }

        out_results[i] = classify_node(g, idx, path_indices, path_size);
    }

    return 0;
}
