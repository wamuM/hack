#include "desviation.h"

int region_in_path(int *path, int path_size, graph *all_nodes, node *region) {
	int is_in_path = 0;
	for (int i = 0; i < path_size && !is_in_path; ++i) {
		if (all_nodes[path[i]] == *region) {
			is_in_path = 1;
		}
	}
	return is_in_path;
}

int region_in_graph(graph *all_nodes, node *region) {
	int is_in_graph = 0;
	for (int i = 0; i < all_nodes->node_len && !is_in_graph; ++i) {
		if (all_nodes->nodes[i] == *region) {
			is_in_graph = 1;
		}
	}
}

int desviation_case(int *path, int path_size, graph *all_nodes, node *region) {
	/* FIRST CASE: The node that the user introduced is in the path*/
	if (region_in_path(path, path_size, all_nodes, region)) {
		return 1;
	}
	/* SECOND CASE: The node is not in the path but it takes part in the connected component*/
	else if (!region_in_path(path, path_size, all_nodes, region) && region_in_graph(all_nodes, region)) {
		return 2;
	}
	/* GENERAL CASE: The node is not in the path as well as in the connected component */ 	 else {
		return 0;
	}
}
