#include "challenge_generator.h"
#include "dijkstra.h"

#include <stdlib.h>

/*
 * Assumptions:
 * - dijkstra(g, start, goal) returns a Path struct.
 * - Path.nodes contains the ordered list of node indices in the shortest path.
 * - Path.len is the number of nodes in that path.
 * - dijkstra_free_path(&path) releases any memory allocated inside Path.
 * - If no path exists, dijkstra returns Path with len <= 0.
 */

int generate_random_start_goal(graph* g, int min_path_node_count, int* start, int* goal, Path* solution)
{
    if (g == NULL || start == NULL || goal == NULL || solution == NULL ||
        min_path_node_count < 2 || g->node_len < min_path_node_count) {
        return -1;
    }

    const int max_attempts = 10000;

    for (int i = 0; i < max_attempts; i++) {
        int a = rand() % g->node_len;
        int b = rand() % g->node_len;

        if (a == b) {
            continue;
        }

        Path path = dijkstra(g, a, b);

        if (path.len >= min_path_node_count) {
            *start = a;
            *goal = b;
            *solution = path;  // caller must free it
            return 0;
        }

        dijkstra_free_path(&path);
    }

    return -1;
}