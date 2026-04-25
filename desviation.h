#ifndef DESVIATION_H
#define DESVIATION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "struct_node.h"

typedef enum {
    INCORRECT_NODE, // It is the value 0
    NEIGHBOR_NODE, // It is the value 1
    PATH_NODE     // It is the value 2
} Desviation_case; 

// Pre: Index is the ID of the user's introduced node and it has to exist, 
// *path is the array with all the middle nodes until the final node and path_size
// is the number of nodes in the path
// Post: Returns 1 if region is in the path list, 0 if it is not
int region_in_path(int index, int *path, int path_size);

// Pre: Index is the ID of the user's introduced node and it has to exist,
// *neighbors is the array with all the neighbors of the target node and neighbors_size
// is the amount of neighbors that the actual node has
// Post: It returns 1 if region is in the graph, 0 if it is not
int region_in_graph(int index, int *neighbors, int neighbors_size);

// Pre: index is the ID of the user's introduced node and it has to exist, *path
// is the array with all the middle nodes until the final node, path_size is 
// the amount of nodes in the path array, *neighbors is the array of the
// neighbors of the actual node and neighbors_size is the amount of neighbors the
// actual node has
// Post: Returns INCORRECT_NODE if the index is neither in the neighbors and the path
// array, PATH_NODE if index is in the path array and NEIGHBOR_NODE if it is not 
// in the path array but it is in the neighbors array
Desviation_case correctness_of_the_node(int index, int *path, int path_size, int *neighbors, int neighbors_size);

#endif
