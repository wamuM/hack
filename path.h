#ifndef PATH_H
#define PATH_H

#include "node.h"   /* defines node and graph — was missing in original */

/*
 * An ordered array of node values (copied from the graph) representing the
 * shortest path from start to goal, inclusive.
 * len == 0 means no path was found.
 */
typedef struct Path Path;
struct Path {
    int *nodes;   /* heap-allocated; free with dijkstra_free_path() */
    int   len;
};

/*
 * BFS shortest path (by hop count) from `start` to `goal` in graph `g`.
 * Returns a Path the caller must free with bfs_free_path().
 */
void bfs(Path* result, graph *g, int start, int goal);
// this version only traverses nodes whose index in max is not 0
void masked_bfs(Path* result, graph *g, int start, int goal, int* mask);
/*
 * Frees the internal node array and resets the struct to the empty state.
 * Takes a pointer so the caller's struct is zeroed as well.
 *
 * FIX: was `Path &path` (C++ reference syntax — invalid in C).
 */
void bfs_free_path(Path *path);

/*
 * Returns a freshly heap-allocated copy of the neighbour index list of node
 * `index` in graph `g`.  *out_count receives the array length.
 * Caller must free() the returned pointer.
 * Returns NULL (and *out_count = 0) on error or if the node is isolated.
 */
int *get_neighbors(const graph *g, int index, int *out_count);

#endif /* PATH_H */
