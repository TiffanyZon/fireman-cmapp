#ifndef ASTAR_H
#define ASTAR_H

#include <stdint.h>

#define GRID_SIZE 10
#define MAX_PATH_SIZE 100

typedef struct {
    int x, y;
} Point;

typedef struct {
    int f, g, h;  // f = g + h
    Point position;
    Point parent;
} Node;

typedef struct {
    Point start;
    Point goal;
    int grid[GRID_SIZE][GRID_SIZE];
    Point path[MAX_PATH_SIZE];
    int pathLength;
} AStar;

void astar_init(AStar *astar, int grid[GRID_SIZE][GRID_SIZE], Point start, Point goal);
int astar_run(AStar *astar);
Point* astar_get_path(AStar *astar, int *pathLength);
void create_grid(int grid[GRID_SIZE][GRID_SIZE]);
int run_astar_algorithm(int grid[GRID_SIZE][GRID_SIZE], Point start, Point goal);
void initialize_grid(int grid[GRID_SIZE][GRID_SIZE]);
void set_obstacle(int grid[GRID_SIZE][GRID_SIZE], int x, int y);


#endif // ASTAR_H
