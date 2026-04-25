/*
 * test_path.c
 *
 * Unit tests for path.c (BFS shortest-path, get_neighbors, bfs_free_path).
 * Lives in tests/ — source files are one level up in ../.
 *
 * Graph used in the tests (indices / names):
 *
 *   0:A --- 1:B --- 2:C --- 3:D
 *                   |
 *                  4:E --- 5:F
 *
 *   6:G  (isolated)
 */

#define _POSIX_C_SOURCE 200809L   /* unlocks strdup under -std=c11 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../node.h"
#include "../graph.h"
#include "../path.h"

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
 * Helpers
 * ------------------------------------------------------------------------- */
static const char *path_node_name(const Path *p, int i)
{
    if (p == NULL || i < 0 || i >= p->len || p->nodes == NULL) return "(null)";
    return p->nodes[i].name ? p->nodes[i].name : "(unnamed)";
}

static int path_matches(const Path *p, const char **expected, int expected_len)
{
    if (p->len != expected_len) return 0;
    for (int i = 0; i < expected_len; i++) {
        if (strcmp(path_node_name(p, i), expected[i]) != 0) return 0;
    }
    return 1;
}

/* -------------------------------------------------------------------------
 * Test groups
 * ------------------------------------------------------------------------- */
static void test_basic_paths(graph *g)
{
    printf("\n-- Basic path finding --\n");

    {
        Path p = bfs(g, 0, 3);
        const char *exp[] = {"A", "B", "C", "D"};
        CHECK("A->D length is 4",        p.len == 4);
        CHECK("A->D route is A-B-C-D",   path_matches(&p, exp, 4));
        bfs_free_path(&p);
    }
    {
        Path p = bfs(g, 0, 5);
        const char *exp[] = {"A", "B", "C", "E", "F"};
        CHECK("A->F length is 5",        p.len == 5);
        CHECK("A->F route is A-B-C-E-F", path_matches(&p, exp, 5));
        bfs_free_path(&p);
    }
    {
        Path p = bfs(g, 3, 5);
        const char *exp[] = {"D", "C", "E", "F"};
        CHECK("D->F length is 4",        p.len == 4);
        CHECK("D->F route is D-C-E-F",   path_matches(&p, exp, 4));
        bfs_free_path(&p);
    }
    {
        Path p = bfs(g, 1, 1);
        CHECK("B->B trivial length is 1", p.len == 1);
        CHECK("B->B trivial node is B",
              p.len == 1 && strcmp(path_node_name(&p, 0), "B") == 0);
        bfs_free_path(&p);
    }
}

static void test_no_path(graph *g)
{
    printf("\n-- No path (isolated node) --\n");

    Path p = bfs(g, 0, 6);
    CHECK("A->G (isolated) returns len 0", p.len == 0);
    CHECK("A->G nodes pointer is NULL",    p.nodes == NULL);
    bfs_free_path(&p);
}

static void test_invalid_inputs(graph *g)
{
    printf("\n-- Invalid inputs --\n");

    Path p;

    p = bfs(NULL, 0, 1);
    CHECK("NULL graph returns len 0",        p.len == 0);

    p = bfs(g, -1, 1);
    CHECK("Negative start returns len 0",    p.len == 0);

    p = bfs(g, 0, 999);
    CHECK("Out-of-range goal returns len 0", p.len == 0);

    bfs_free_path(NULL);
    CHECK("bfs_free_path(NULL) does not crash", 1);
}

static void test_get_neighbors(graph *g)
{
    printf("\n-- get_neighbors --\n");

    {
        int cnt = 0;
        int *nb = get_neighbors(g, 2, &cnt);
        CHECK("C has 3 neighbours", cnt == 3);
        free(nb);
    }
    {
        int cnt = 0;
        int *nb = get_neighbors(g, 0, &cnt);
        CHECK("A has 1 neighbour",  cnt == 1 && nb != NULL && nb[0] == 1);
        free(nb);
    }
    {
        int cnt = 0;
        int *nb = get_neighbors(g, 6, &cnt);
        CHECK("G (isolated) returns NULL", nb == NULL);
        CHECK("G (isolated) count is 0",   cnt == 0);
    }
    {
        int cnt = 0;
        int *nb = get_neighbors(NULL, 0, &cnt);
        CHECK("NULL graph returns NULL", nb == NULL);
        CHECK("NULL graph count is 0",   cnt == 0);
    }
}

static void test_free_reuse(graph *g)
{
    printf("\n-- Free and reuse --\n");

    Path p = bfs(g, 0, 3);
    CHECK("First A->D succeeds",    p.len == 4);
    bfs_free_path(&p);
    CHECK("After free, len is 0",   p.len == 0);
    CHECK("After free, nodes NULL", p.nodes == NULL);

    p = bfs(g, 0, 3);
    CHECK("Second A->D still works", p.len == 4);
    bfs_free_path(&p);
}

/* -------------------------------------------------------------------------
 * main
 * ------------------------------------------------------------------------- */
int main(void)
{
    printf("========================================\n");
    printf("  test_path — BFS unit tests\n");
    printf("========================================\n");

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
    /* G (index 6) intentionally left isolated */

    test_basic_paths(g);
    test_no_path(g);
    test_invalid_inputs(g);
    test_get_neighbors(g);
    test_free_reuse(g);

    graph_destroy(g);

    printf("\n========================================\n");
    printf("  Results: %d / %d passed\n", tests_passed, tests_run);
    printf("========================================\n");

    return (tests_passed == tests_run) ? 0 : 1;
}
