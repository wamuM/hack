#pragma once
#include "node.h"

#define MAX_NUM_SUGGEST 3

// Returns the number of suggestions that have been placed in output
int generate_suggestions(char* input, graph* g, char** output, int* state);
