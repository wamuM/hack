#include "graph.h"
#include "cJSON.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    int* refs;
    int ref_count;
} BorderRefs;

static int is_relation(cJSON* element)
{
    cJSON* type = cJSON_GetObjectItem(element, "type");

    return cJSON_IsString(type) &&
           strcmp(cJSON_GetStringValue(type), "relation") == 0;
}

static char* get_relation_name(cJSON* element)
{
    cJSON* tags = cJSON_GetObjectItem(element, "tags");
    cJSON* name = NULL;

    if (cJSON_IsObject(tags)) {
        name = cJSON_GetObjectItem(tags, "name:ca");

        if (!cJSON_IsString(name)) {
            name = cJSON_GetObjectItem(tags, "name");
        }
    }

    if (cJSON_IsString(name)) {
        return strdup(cJSON_GetStringValue(name));
    }

    return strdup("unknown");
}

static int add_ref(BorderRefs* border_refs, int ref)
{
    int* new_refs = realloc(
        border_refs->refs,
        (border_refs->ref_count + 1) * sizeof(int)
    );

    if (new_refs == NULL) {
        return -1;
    }

    border_refs->refs = new_refs;
    border_refs->refs[border_refs->ref_count] = ref;
    border_refs->ref_count++;

    return 0;
}

static int collect_outer_refs(cJSON* element, BorderRefs* border_refs)
{
    cJSON* members = cJSON_GetObjectItem(element, "members");

    if (!cJSON_IsArray(members)) {
        return 0;
    }

    int member_count = cJSON_GetArraySize(members);

    for (int i = 0; i < member_count; i++) {
        cJSON* member = cJSON_GetArrayItem(members, i);

        cJSON* type = cJSON_GetObjectItem(member, "type");
        cJSON* role = cJSON_GetObjectItem(member, "role");
        cJSON* ref = cJSON_GetObjectItem(member, "ref");

        if (cJSON_IsString(type) &&
            cJSON_IsString(role) &&
            cJSON_IsNumber(ref) &&
            strcmp(cJSON_GetStringValue(type), "way") == 0 &&
            strcmp(cJSON_GetStringValue(role), "outer") == 0) {

            if (add_ref(border_refs, (int)cJSON_GetNumberValue(ref)) != 0) {
                return -1;
            }
        }
    }

    return 0;
}

static int share_ref(const BorderRefs* a, const BorderRefs* b)
{
    for (int i = 0; i < a->ref_count; i++) {
        for (int j = 0; j < b->ref_count; j++) {
            if (a->refs[i] == b->refs[j]) {
                return 1;
            }
        }
    }

    return 0;
}

static void free_border_refs(BorderRefs* borders, int count)
{
    if (borders == NULL) {
        return;
    }

    for (int i = 0; i < count; i++) {
        free(borders[i].refs);
    }

    free(borders);
}

int graph_add_edge(graph* g, int from, int to)
{
    if (g == NULL || from < 0 || to < 0 ||
        from >= g->node_len || to >= g->node_len || from == to) {
        return -1;
    }

    node* a = &g->nodes[from];
    node* b = &g->nodes[to];

    int* new_a = realloc(
        a->incidence_list,
        (a->incidence_cnt + 1) * sizeof(int)
    );

    if (new_a == NULL) {
        return -1;
    }

    a->incidence_list = new_a;
    a->incidence_list[a->incidence_cnt] = to;
    a->incidence_cnt++;

    int* new_b = realloc(
        b->incidence_list,
        (b->incidence_cnt + 1) * sizeof(int)
    );

    if (new_b == NULL) {
        return -1;
    }

    b->incidence_list = new_b;
    b->incidence_list[b->incidence_cnt] = from;
    b->incidence_cnt++;

    return 0;
}

static int build_edges_from_refs(graph* g, BorderRefs* borders)
{
    for (int i = 0; i < g->node_len; i++) {
        for (int j = i + 1; j < g->node_len; j++) {
            if (share_ref(&borders[i], &borders[j])) {
                if (graph_add_edge(g, i, j) != 0) {
                    return -1;
                }
            }
        }
    }

    return 0;
}

void graph_destroy(graph* g)
{
    if (g == NULL) {
        return;
    }

    for (int i = 0; i < g->node_len; i++) {
        free(g->nodes[i].incidence_list);
        free(g->nodes[i].name);
    }

    free(g->nodes);
    free(g);
}

graph* graph_create_from_cjson(cJSON* root)
{
    if (root == NULL) {
        return NULL;
    }

    cJSON* elements = cJSON_GetObjectItem(root, "elements");

    if (!cJSON_IsArray(elements)) {
        return NULL;
    }

    int total = cJSON_GetArraySize(elements);

    // Count relations
    int relation_count = 0;
    for (int i = 0; i < total; i++) {
        cJSON* element = cJSON_GetArrayItem(elements, i);
        if (is_relation(element)) {
            relation_count++;
        }
    }

    if (relation_count == 0) {
        return NULL;
    }

    graph* g = malloc(sizeof(graph));
    if (g == NULL) {
        return NULL;
    }

    g->node_len = relation_count;
    g->nodes = calloc(relation_count, sizeof(node));

    if (g->nodes == NULL) {
        free(g);
        return NULL;
    }

    BorderRefs* borders = calloc(relation_count, sizeof(BorderRefs));
    if (borders == NULL) {
        graph_destroy(g);
        return NULL;
    }

    int node_index = 0;

    // Fill nodes
    for (int i = 0; i < total; i++) {
        cJSON* element = cJSON_GetArrayItem(elements, i);

        if (!is_relation(element)) {
            continue;
        }

        node* n = &g->nodes[node_index];

        n->incidence_list = NULL;
        n->incidence_cnt = 0;
        n->name = get_relation_name(element);

        if (n->name == NULL) {
            free_border_refs(borders, relation_count);
            graph_destroy(g);
            return NULL;
        }

        if (collect_outer_refs(element, &borders[node_index]) != 0) {
            free_border_refs(borders, relation_count);
            graph_destroy(g);
            return NULL;
        }

        node_index++;
    }

    // Build edges
    if (build_edges_from_refs(g, borders) != 0) {
        free_border_refs(borders, relation_count);
        graph_destroy(g);
        return NULL;
    }

    free_border_refs(borders, relation_count);

    return g;
}