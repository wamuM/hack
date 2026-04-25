#pragma once

typedef struct node node;
struct node 
{
    int* incidence_list;
    float** lats;
    float** lons;
    int num_cc;
    int* len_cc;
};

typedef struct graph graph;

struct graph
{
    node* nodes;
    int node_len;
};
