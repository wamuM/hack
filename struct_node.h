typedef struct node node;
struct node 
{
    int len;
    int* incidence_list;
    float* lats;
    float* lons;
};

typedef struct graph graph;

struct graph
{
    node *nodes;
    int node_len;
};
