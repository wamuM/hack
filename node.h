#pragma once

typedef struct node node;
struct node 
{
    float* lats;
    float* lons;
    char* name;
    int len;
    int incidence_list[];
};

typedef struct graph graph;

struct graph
{
    node* nodes;
    int node_len;
};
