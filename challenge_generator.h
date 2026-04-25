#pragma once

#include "node.h"
#include "path.h"

int generate_random_start_goal(
    graph* g,
    int min_path_node_count,
    int* start,
    int* goal,
    Path* solution
);
