#pragma once

#include "node.h"
#include "cJSON.h"

// Builds a graph from an already parsed Overpass API JSON.
graph* graph_create_from_cjson(cJSON* root);

// Frees all memory used by the graph.
void graph_destroy(graph* g);

// Adds an undirected edge between two nodes.
int graph_add_edge(graph* g, int from, int to);