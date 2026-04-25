#pragma once

typedef struct node node;
struct node 
{
    int* incidence_list;
    int incidence_cnt;
    char* name;
};

typedef struct graph graph;

struct graph
{
    node* nodes;
    int node_len;
};
