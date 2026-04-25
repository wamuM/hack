#include "desviation.h"

int region_in_path(int index, int *path, int path_size) {
	int is_in_path = 0;
	for (int i = 0; i < path_size && !is_in_path; ++i) {
		if (path[i] == index) {
			is_in_path = 1;
		}
	}
	return is_in_path;
}

int region_in_graph(int index, int *neighbors, int neighbors_size) {
	int is_in_graph = 0;
	for (int i = 0; i < neighbors_size && !is_in_graph; ++i) {
		if (neighbors[i] == index) {
			is_in_graph = 1;
		}
	}
  return is_in_graph;
}

Desviation_case correctness_of_the_node (int index, int *path, int path_size, int *neighbors, int neighbors_size) {
  /* FIRST CASE: The node that the user introduced is in the path*/
	if (region_in_path(index, path, path_size)) {
		return PATH_NODE; 
	}
	/* SECOND CASE: The node is not in the path but it takes part in the connected component*/
	else if (!region_in_path(index, path, path_size) && region_in_graph(index, neighbors, neighbors_size)) {
		return NEIGHBOR_NODE;
	}
	/* GENERAL CASE: The node is not in the path as well as in the connected component */ 	 else {
	return INCORRECT_NODE;
	}
}
