#ifdef A_STAR_H
#define A_STAR_H

#include <stdint.h>

#define GRID_SIZE 10
#define MAX_PATH_SIZE 100 

typedef struct 
{
   int x, y;
}point;

typedef struct
{
  int f,g,h;
  Point position;
  Point parent;
}Node;


typedef struct
{
    Point start;
    Point goal;
    int grid[GRID_SIZE][GRID_SIZE];
    Point path[MAX_PATH_SIZE];
    int pathLength
}AStar;

void astar_init(AStar *astar, int grid[GRID_SIZE][GRID_SIZE], Point start, Point goal);

void astar_run(AStar *astar, int *pathLength);

Point* astar_get_path(AStar *astar, int *pathLength);

#endif 

