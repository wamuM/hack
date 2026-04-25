#include <stdio.h>
#include <string.h>

#include "cJSON.h"
#include "fetcher.h"
#include "graph.h"

static int find_node_by_name(graph* g, const char* name)
{
    for (int i = 0; i < g->node_len; i++) {
        if (strcmp(g->nodes[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int are_adjacent(graph* g, int a, int b)
{
    for (int i = 0; i < g->nodes[a].incidence_cnt; i++) {
        if (g->nodes[a].incidence_list[i] == b) {
            return 1;
        }
    }
    return 0;
}

static void test_neighbors(graph* g, const char* a_name, const char* b_name, int expected)
{
    int a = find_node_by_name(g, a_name);
    int b = find_node_by_name(g, b_name);

    if (a == -1 || b == -1) {
        printf("[ERROR] Node not found: %s or %s\n", a_name, b_name);
        return;
    }

    int result = are_adjacent(g, a, b);

    printf("[%s] %s <-> %s | expected=%d got=%d\n",
           result == expected ? "OK" : "FAIL",
           a_name, b_name,
           expected,
           result);
}

static void validate_graph(graph* g)
{
    for (int i = 0; i < g->node_len; i++) {
        for (int j = 0; j < g->nodes[i].incidence_cnt; j++) {
            int nb = g->nodes[i].incidence_list[j];

            if (!are_adjacent(g, nb, i)) {
                printf("[FAIL] Asymmetry: %s -> %s but not reverse\n",
                       g->nodes[i].name, g->nodes[nb].name);
            }

            for (int k = j + 1; k < g->nodes[i].incidence_cnt; k++) {
                if (g->nodes[i].incidence_list[k] == nb) {
                    printf("[FAIL] Duplicate neighbor in %s: %s\n",
                           g->nodes[i].name, g->nodes[nb].name);
                }
            }
        }

        if (g->nodes[i].incidence_cnt == 0) {
            printf("[WARN] Isolated node: %s\n", g->nodes[i].name);
        }
    }

    printf("Graph validation finished.\n");
}

int main(void)
{
    cJSON* obj = get_json("Catalunya", 7);

    if (obj == NULL) {
        printf("Error: could not fetch JSON\n");
        return 1;
    }

    graph* g = graph_create_from_cjson(obj);

    if (g == NULL) {
        printf("Error: could not create graph\n");
        cJSON_Delete(obj);
        return 1;
    }

    printf("Graph created successfully!\n");
    printf("Number of nodes: %d\n", g->node_len);

    validate_graph(g);
    
    test_neighbors(g, "Tarragonès", "Alt Camp", 1);
    test_neighbors(g, "Tarragonès", "Baix Penedès", 1);
    test_neighbors(g, "Tarragonès", "Vallès Occidental", 0);
    test_neighbors(g, "Barcelonès", "Baix Llobregat", 1);
    test_neighbors(g, "Barcelonès", "Segrià", 0);

    graph_destroy(g);
    cJSON_Delete(obj);

    return 0;
}