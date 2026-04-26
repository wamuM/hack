/*
 * path.c
 *
 * Despite the filename, this implements a BFS (Breadth-First Search) to find
 * the shortest path (by hop count) between two nodes in an unweighted graph.
 *
 * The graph is unweighted, so BFS is both correct and optimal here.
 * Dijkstra's algorithm would give identical results but with unnecessary
 * overhead — BFS is the right tool.
 *
 * Path.nodes  — array of node *pointers* (into g->nodes), in order from
 *               start to goal, inclusive.
 * Path.len    — number of nodes in that array (0 means no path found).
 *
 * The caller is responsible for freeing the path with bfs_free_path().
 */

#include "path.h"
#include "node.h"

#include <stdlib.h>
#include <string.h>

/* -------------------------------------------------------------------------
 * Internal queue used by BFS
 * ------------------------------------------------------------------------- */

typedef struct {
    int  *data;
    int   head;
    int   tail;
    int   capacity;
} Queue;

static int queue_init(Queue *q, int capacity)
{
    q->data = malloc(capacity * sizeof(int));
    if (q->data == NULL) return -1;
    q->head     = 0;
    q->tail     = 0;
    q->capacity = capacity;
    return 0;
}

static void queue_free(Queue *q)
{
    free(q->data);
    q->data = NULL;
}

static int queue_empty(const Queue *q)
{
    return q->head == q->tail;
}

static int queue_push(Queue *q, int value)
{
    if (q->tail >= q->capacity) return -1;   /* should never happen */
    q->data[q->tail++] = value;
    return 0;
}

static int queue_pop(Queue *q)
{
    return q->data[q->head++];
}

/* -------------------------------------------------------------------------
 * get_neighbors
 *
 * Returns a freshly-allocated array of the indices of all neighbors of
 * node `index` in graph `g`.  *out_count receives the array length.
 * The caller must free() the returned pointer.
 * Returns NULL (and *out_count = 0) on error or if the node is isolated.
 * ------------------------------------------------------------------------- */
int *get_neighbors(const graph *g, int index, int *out_count)
{
    *out_count = 0;

    if (g == NULL || index < 0 || index >= g->node_len) {
        return NULL;
    }

    const node *n = &g->nodes[index];

    if (n->incidence_cnt == 0) {
        return NULL;
    }

    int *neighbors = malloc(n->incidence_cnt * sizeof(int));
    if (neighbors == NULL) {
        return NULL;
    }

    memcpy(neighbors, n->incidence_list, n->incidence_cnt * sizeof(int));
    *out_count = n->incidence_cnt;

    return neighbors;
}

/* -------------------------------------------------------------------------
 * BFS
 * ------------------------------------------------------------------------- */
void bfs_core(Path* result, graph *g, int start, int goal, int useMask, int* mask)
{
    result->nodes = NULL;
    result->len   = 0;

    if (g == NULL || start < 0 || goal < 0 ||
        start >= g->node_len || goal >= g->node_len) {
        return;
    }

    /* Trivial case: start == goal */
    if (start == goal) {
        result->nodes = malloc(sizeof(node));
        if (result->nodes == NULL) return;
        result->nodes[0] = start;
        result->len = 1;
        return;
    }

    const int N = g->node_len;

    /*
     * prev[i] = index of the node from which i was reached.
     * -1 means "not visited".
     */
    int *prev = malloc(N * sizeof(int));
    if (prev == NULL) return;
    for (int i = 0; i < N; i++) prev[i] = -1;

    Queue q;
    if (queue_init(&q, N) != 0) {
        free(prev);
        return;
    }

    /* Mark start as visited (using itself as a sentinel) */
    prev[start] = start;
    queue_push(&q, start);

    int found = 0;

    while (!queue_empty(&q)) {
        int cur = queue_pop(&q);

        if (cur == goal) {
            found = 1;
            break;
        }

        for (int i = 0; i < g->nodes[cur].incidence_cnt; i++) {
            int nb = g->nodes[cur].incidence_list[i];
            if(useMask && mask[nb] == 0)continue;
            if (prev[nb] == -1) {
                prev[nb] = cur;
                queue_push(&q, nb);
            }
        }
    }

    queue_free(&q);

    if (!found) {
        free(prev);
        return;   /* no path — len stays 0 */
    }

    /* -----------------------------------------------------------------------
     * Reconstruct the path by walking prev[] backwards from goal to start.
     * ----------------------------------------------------------------------- */

    /* First pass: count the path length */
    int path_len = 0;
    for (int cur = goal; ; cur = prev[cur]) {
        path_len++;
        if (cur == start) break;
    }

    int *path_nodes = malloc(path_len * sizeof(int));
    if (path_nodes == NULL) {
        free(prev);
        return;
    }

    /* Second pass: fill in reverse order, then reverse in place */
    int idx = path_len - 1;
    for (int cur = goal; ; cur = prev[cur]) {
        path_nodes[idx--] = cur;
        if (cur == start) break;
    }

    free(prev);

    result->nodes = path_nodes;
    result->len   = path_len;
}
void bfs(Path* result, graph *g, int start, int goal){
    bfs_core(result, g, start, goal, 0, NULL);
}
void masked_bfs(Path* result, graph *g, int start, int goal, int* mask){
    bfs_core(result, g, start, goal, 1, mask);
}
/* -------------------------------------------------------------------------
 * bfs_free_path
 * ------------------------------------------------------------------------- */
void bfs_free_path(Path *path)
{
    if (path == NULL) return;
    free(path->nodes);
    path->nodes = NULL;
    path->len   = 0;
}
