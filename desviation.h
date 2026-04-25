#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "struct_node.h"

// Pre: path is an array of the subindex of a connected component of the graph all_nodes, path_size is how many nodes are stored in the path and region is the node that the user typed previously
// Post: Returns 1 if region is in the path list, 0 if it is not
int region_in_path(int *path, int path_size, graph *all_nodes, node *region);

// Pre: all_nodes is the graph of all available nodes and region is the user's typed region
// Post: It returns 1 if region is in the graph, 0 if it is not
int region_in_graph(graph *all_nodes, node *region);

// Pre: path is an array of the subindex of a connected component of the graph all_nodes, path_size is how many nodes are stored in the path and region is the node that the user typed previously
// Post: Returns 1 if region is in the path, 2 if it is not in the path but it is in the all_nodes graph and 0 if it is not in neither
int desviation_case(int *path, int path_size, graph *all_nodes, node *region);
