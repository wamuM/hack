#pragma once
#include "node.h"

// Returns the number of suggestions that have been placed in output
int generate_suggestions(char *input, graph *g, char **output_str, int* output_index, int* state, int max);
