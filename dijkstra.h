#ifndef DIJKSTRA_H
#define DIJKSTRA_H

typedef struct Path Path;
struct Path {
	node *nodes;
	int len;
};

Path dijkstra(graph *g, int start, int goal);

void dijkstra_free_path(Path &path);

#endif
