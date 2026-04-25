#include "challenge_generator.h"
#include "path.h"

#include <stdlib.h>

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

        Path path = path(g, a, b);

        if (path.len >= min_path_node_count) {
            *start = a;
            *goal = b;
            *solution = path;  // caller must free it
            return 0;
        }

        path_free_path(&path);
    }

    return -1;
}