/*
 * test_deviation.c
 *
 * Unit tests for deviation.c.
 * Lives in tests/ — source files are one level up in ../.
 *
 * Solution path used in most tests:  A->B->C->D  (indices 0,1,2,3)
 *
 *   0:A --- 1:B --- 2:C --- 3:D
 *                   |
 *                  4:E --- 5:F
 *
 *   6:G  (isolated)
 *
 * Expected classifications:
 *   A(0), B(1), C(2), D(3) -> PATH_NODE
 *   E(4)                   -> ALMOST_PATH_NODE  (neighbour of C)
 *   F(5)                   -> INCORRECT_NODE    (neighbour of E only)
 *   G(6)                   -> INCORRECT_NODE    (isolated)
 */

#define _POSIX_C_SOURCE 200809L   /* unlocks strdup under -std=c11 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../node.h"
#include "../graph.h"
#include "../deviation.h"

/* -------------------------------------------------------------------------
 * Minimal test framework
 * ------------------------------------------------------------------------- */

static int tests_run    = 0;
static int tests_passed = 0;

#define CHECK(description, condition)                                        \
    do {                                                                     \
        tests_run++;                                                         \
        if (condition) {                                                     \
            printf("  [PASS] %s\n", description);                           \
            tests_passed++;                                                  \
        } else {                                                             \
            printf("  [FAIL] %s  (line %d)\n", description, __LINE__);      \
        }                                                                    \
    } while (0)

/* -------------------------------------------------------------------------
 * Graph builder
 * ------------------------------------------------------------------------- */
static graph *make_graph(const char **names, int n)
{
    graph *g = malloc(sizeof(graph));
    if (!g) return NULL;

    g->nodes    = calloc(n, sizeof(node));
    g->node_len = n;

    if (!g->nodes) { free(g); return NULL; }

    for (int i = 0; i < n; i++) {
        g->nodes[i].name           = strdup(names[i]);
        g->nodes[i].incidence_list = NULL;
        g->nodes[i].incidence_cnt  = 0;
    }

    return g;
}

/* -------------------------------------------------------------------------
 * Test groups
 * ------------------------------------------------------------------------- */

static const int PATH[]    = {0, 1, 2, 3};
static const int PATH_SIZE = 4;

static void test_region_in_path(void)
{
    printf("\n-- region_in_path --\n");

    CHECK("A (0) is in path",     region_in_path(0, PATH, PATH_SIZE) == 1);
    CHECK("C (2) is in path",     region_in_path(2, PATH, PATH_SIZE) == 1);
    CHECK("D (3) is in path",     region_in_path(3, PATH, PATH_SIZE) == 1);
    CHECK("E (4) is NOT in path", region_in_path(4, PATH, PATH_SIZE) == 0);
    CHECK("G (6) is NOT in path", region_in_path(6, PATH, PATH_SIZE) == 0);
}

static void test_is_almost_path_node(graph *g)
{
    printf("\n-- is_almost_path_node --\n");

    CHECK("E is almost-path (neighbour of C)",
          is_almost_path_node(g, 4, PATH, PATH_SIZE) == 1);
    CHECK("F is NOT almost-path (neighbour of E only)",
          is_almost_path_node(g, 5, PATH, PATH_SIZE) == 0);
    CHECK("G (isolated) is NOT almost-path",
          is_almost_path_node(g, 6, PATH, PATH_SIZE) == 0);
    /* A neighbours B which is on the path — safe to call directly */
    CHECK("A (on path) almost-path check does not crash",
          is_almost_path_node(g, 0, PATH, PATH_SIZE) == 1);
}

static void test_classify_node(graph *g)
{
    printf("\n-- classify_node --\n");

    CHECK("A (0) -> PATH_NODE",        classify_node(g, 0, PATH, PATH_SIZE) == PATH_NODE);
    CHECK("B (1) -> PATH_NODE",        classify_node(g, 1, PATH, PATH_SIZE) == PATH_NODE);
    CHECK("C (2) -> PATH_NODE",        classify_node(g, 2, PATH, PATH_SIZE) == PATH_NODE);
    CHECK("D (3) -> PATH_NODE",        classify_node(g, 3, PATH, PATH_SIZE) == PATH_NODE);
    CHECK("E (4) -> ALMOST_PATH_NODE", classify_node(g, 4, PATH, PATH_SIZE) == ALMOST_PATH_NODE);
    CHECK("F (5) -> INCORRECT_NODE",   classify_node(g, 5, PATH, PATH_SIZE) == INCORRECT_NODE);
    CHECK("G (6) -> INCORRECT_NODE",   classify_node(g, 6, PATH, PATH_SIZE) == INCORRECT_NODE);
}

static void test_find_node_index_by_name(graph *g)
{
    printf("\n-- find_node_index_by_name --\n");

    CHECK("'A' resolves to 0",    find_node_index_by_name(g, "A") == 0);
    CHECK("'C' resolves to 2",    find_node_index_by_name(g, "C") == 2);
    CHECK("'G' resolves to 6",    find_node_index_by_name(g, "G") == 6);
    CHECK("Unknown returns -1",   find_node_index_by_name(g, "Z") == -1);
    CHECK("NULL name returns -1", find_node_index_by_name(g, NULL) == -1);
    CHECK("NULL graph returns -1",find_node_index_by_name(NULL, "A") == -1);
}

static void test_evaluate_user_regions(graph *g)
{
    printf("\n-- evaluate_user_regions: mixed input --\n");

    const char *regions[] = {"A", "E", "F", "Unknown"};
    Deviation_case results[4];

    int rc = evaluate_user_regions(g, regions, 4, PATH, PATH_SIZE, results);

    CHECK("returns 0 on success",          rc == 0);
    CHECK("'A'       -> PATH_NODE",        results[0] == PATH_NODE);
    CHECK("'E'       -> ALMOST_PATH_NODE", results[1] == ALMOST_PATH_NODE);
    CHECK("'F'       -> INCORRECT_NODE",   results[2] == INCORRECT_NODE);
    CHECK("'Unknown' -> INCORRECT_NODE",   results[3] == INCORRECT_NODE);
}

static void test_evaluate_all_correct(graph *g)
{
    printf("\n-- evaluate_user_regions: full correct path --\n");

    const char *regions[] = {"A", "B", "C", "D"};
    Deviation_case results[4];

    evaluate_user_regions(g, regions, 4, PATH, PATH_SIZE, results);

    CHECK("'A' -> PATH_NODE", results[0] == PATH_NODE);
    CHECK("'B' -> PATH_NODE", results[1] == PATH_NODE);
    CHECK("'C' -> PATH_NODE", results[2] == PATH_NODE);
    CHECK("'D' -> PATH_NODE", results[3] == PATH_NODE);
}

static void test_invalid_inputs(graph *g)
{
    printf("\n-- Invalid inputs --\n");

    Deviation_case results[2];
    const char *regions[] = {"A", "B"};

    CHECK("NULL graph -> -1",
          evaluate_user_regions(NULL, regions, 2, PATH, PATH_SIZE, results) == -1);
    CHECK("NULL regions -> -1",
          evaluate_user_regions(g, NULL, 2, PATH, PATH_SIZE, results) == -1);
    CHECK("count 0 -> -1",
          evaluate_user_regions(g, regions, 0, PATH, PATH_SIZE, results) == -1);
    CHECK("NULL path -> -1",
          evaluate_user_regions(g, regions, 2, NULL, PATH_SIZE, results) == -1);
    CHECK("NULL out_results -> -1",
          evaluate_user_regions(g, regions, 2, PATH, PATH_SIZE, NULL) == -1);
}

/* -------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------- */
int main(void)
{
    printf("==========================================\n");
    printf("  test_deviation — deviation unit tests\n");
    printf("==========================================\n");

    const char *names[] = {"A", "B", "C", "D", "E", "F", "G"};
    graph *g = make_graph(names, 7);
    if (g == NULL) {
        fprintf(stderr, "FATAL: could not allocate test graph\n");
        return 1;
    }

    graph_add_edge(g, 0, 1);   /* A-B */
    graph_add_edge(g, 1, 2);   /* B-C */
    graph_add_edge(g, 2, 3);   /* C-D */
    graph_add_edge(g, 2, 4);   /* C-E */
    graph_add_edge(g, 4, 5);   /* E-F */
    /* G intentionally isolated */

    test_region_in_path();
    test_is_almost_path_node(g);
    test_classify_node(g);
    test_find_node_index_by_name(g);
    test_evaluate_user_regions(g);
    test_evaluate_all_correct(g);
    test_invalid_inputs(g);

    graph_destroy(g);

    printf("\n==========================================\n");
    printf("  Results: %d / %d passed\n", tests_passed, tests_run);
    printf("==========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
