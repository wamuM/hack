#pragma once

#include "node.h"
#include "cJSON.h"

// Builds a graph from an already parsed Overpass API JSON.
int graph_create_from_cjson(cJSON* root, graph* g, char* lang);

// Frees all memory used by the graph.
void graph_destroy(graph* g);

// Adds an undirected edge between two nodes.
int graph_add_edge(graph* g, int from, int to);
