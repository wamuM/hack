#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cJSON.h"
#include "fetcher.h"
#include "graph.h"
#include "path.h"
#include "challenge_generator.h"

static int failures = 0;

static void check(int condition, const char* name)
{
    if (condition) {
        printf("[OK] %s\n", name);
        return;
    }

    failures++;
    printf("[FAIL] %s\n", name);
}

static int find_node_by_name(const graph* g, const char* name)
{
    for (int i = 0; i < g->node_len; i++) {
        if (strcmp(g->nodes[i].name, name) == 0) {
            return i;
        }
    }

    return -1;
}

static int are_adjacent(const graph* g, const char* a_name, const char* b_name)
{
    int a = find_node_by_name(g, a_name);
    int b = find_node_by_name(g, b_name);

    if (a < 0 || b < 0) {
        return 0;
    }

    for (int i = 0; i < g->nodes[a].incidence_cnt; i++) {
        int nb = g->nodes[a].incidence_list[i];

        if (strcmp(g->nodes[nb].name, b_name) == 0) {
            return 1;
        }
    }

    return 0;
}

static void test_solution_path_is_valid(const graph* g, const Path* solution)
{
    check(solution->len > 0, "solution path is not empty");
    for (int i = 0; i < solution->len - 1; i++) {
        char msg[256];
        const char *name_a = g->nodes[solution->nodes[i]].name;
        const char *name_b = g->nodes[solution->nodes[i + 1]].name;
        snprintf(msg, sizeof(msg), "path step is valid: %s -> %s", name_a, name_b);
        check(are_adjacent(g, name_a, name_b), msg);
    }
}

static void test_catalog_has_known_comarques(const graph* g)
{
    check(find_node_by_name(g, "Alt Camp") >= 0,
          "graph contains Alt Camp");
    check(find_node_by_name(g, "Baix Camp") >= 0,
        "graph contains Baix Camp");
    check(find_node_by_name(g, "Priorat") >= 0,
        "graph contains Priorat");
}

static void test_negative_inputs(graph* g)
{
    int start = -1;
    int goal = -1;
    Path solution = { .nodes = NULL, .len = 0 };

    check(generate_random_start_goal(NULL, 2, &start, &goal, &solution) == -1,
          "rejects NULL graph");
    check(generate_random_start_goal(g, 1, &start, &goal, &solution) == -1,
          "rejects min_path_node_count < 2");
    check(generate_random_start_goal(g, 2, NULL, &goal, &solution) == -1,
          "rejects NULL start pointer");
    check(generate_random_start_goal(g, 2, &start, NULL, &solution) == -1,
          "rejects NULL goal pointer");
    check(generate_random_start_goal(g, 2, &start, &goal, NULL) == -1,
          "rejects NULL solution pointer");
    check(generate_random_start_goal(g, g->node_len + 1, &start, &goal, &solution) == -1,
          "rejects impossible minimum length by node count");
}

static void test_positive_case(graph* g)
{
    int start = -1;
    int goal = -1;
    Path solution = { .nodes = NULL, .len = 0 };

    srand(7);
    check(generate_random_start_goal(g, 3, &start, &goal, &solution) == 0,
          "finds start and goal in Catalunya graph");

    check(start >= 0 && start < g->node_len, "start index is valid");
    check(goal >= 0 && goal < g->node_len, "goal index is valid");
    check(start != goal, "start and goal are different");
    check(solution.len >= 3, "returned path length satisfies minimum");

    if (solution.len > 0) {
		check(strcmp(g->nodes[solution.nodes[0]].name, g->nodes[start].name) == 0,
      "path starts at selected start comarca");
		check(strcmp(g->nodes[solution.nodes[solution.len - 1]].name, 
		g->nodes[goal].name) == 0, "path ends at selected goal comarca");
        
		test_solution_path_is_valid(g, &solution);
    }

    bfs_free_path(&solution);
}

int main(void)
{
    cJSON* root = get_json("'name'='Catalunya'", 4, 7);
    if (root == NULL) {
        printf("[FAIL] could not fetch Catalunya comarques json\n");
        return 1;
    }

	graph *g = malloc(sizeof(graph));
	if (g == NULL) {
    	printf("Error: could not allocate graph\n");
    	cJSON_Delete(root);
    	return 1;
	}
	if (graph_create_from_cjson(root, g) != 0) {
    	printf("Error: could not create graph\n");
    	free(g);
    	cJSON_Delete(root);
    	return 1;
	}

    test_catalog_has_known_comarques(g);
    test_negative_inputs(g);
    test_positive_case(g);

	graph *g2 = malloc(sizeof(graph));
	if (graph_create_from_cjson(root, g2) != 0) {
    	printf("Error: could not create graph\n");
    	free(g2);
    	cJSON_Delete(root);
    	return 1;
	}
    graph_destroy(g2);

// Al final: graph_destroy(g); — esto sí funciona
    if (failures != 0) {
        printf("\n%d check(s) failed.\n", failures);
        return 1;
    }

    printf("\nAll challenge_generator checks passed (Catalunya comarques graph).\n");
    return 0;
}
