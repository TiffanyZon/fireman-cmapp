#include "a_star.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INF 0x3f3f3f3f

// Directions: Up, Down, Left, Right
const Point directions[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
const int NUM_DIRECTIONS = 4;

// Utility function to calculate Manhattan distance
static int calculate_heuristic(Point a, Point b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

// Checks if a point is within the grid and walkable
static int is_valid(Point p, int grid[GRID_SIZE][GRID_SIZE]) {
    return p.x >= 0 && p.x < GRID_SIZE && p.y >= 0 && p.y < GRID_SIZE && grid[p.x][p.y] == 0;
}

// Checks if two points are the same
static int is_same_point(Point a, Point b) {
    return a.x == b.x && a.y == b.y;
}

// Finds a node in the open list
static Node* find_in_open_list(Node *openList, int openSize, Point position) {
    for (int i = 0; i < openSize; i++) {
        if (is_same_point(openList[i].position, position)) {
            return &openList[i];
        }
    }
    return NULL;
}

// دالة للبحث عن العقدة في القائمة المغلقة
static Node* find_in_closed_list(int closedList[GRID_SIZE][GRID_SIZE], Point position) {
    // قم بالبحث عن العقدة في القائمة المغلقة
    if (closedList[position.x][position.y] == 1) {
        Node* foundNode = malloc(sizeof(Node));
        // تم العثور على العقدة، ولكن يجب ضبط خصائصها هنا بناءً على كيفية تخزين الوالد في القائمة المغلقة.
        foundNode->position = position;
        return foundNode;
    }
    return NULL;
}

// Initializes the A* instance
void astar_init(AStar *astar, int grid[GRID_SIZE][GRID_SIZE], Point start, Point goal) {
    memcpy(astar->grid, grid, sizeof(astar->grid));
    astar->start = start;
    astar->goal = goal;
    astar->pathLength = 0;
}

// Runs the A* algorithm
int astar_run(AStar *astar) {
    Node openList[GRID_SIZE * GRID_SIZE];
    int openSize = 0;
    int closedList[GRID_SIZE][GRID_SIZE];
    memset(closedList, 0, sizeof(closedList));

    Node startNode = {0, 0, calculate_heuristic(astar->start, astar->goal), astar->start, {-1, -1}};
    openList[openSize++] = startNode;

    while (openSize > 0) {
        // Find the node with the smallest f in the open list
        int smallestIdx = 0;
        for (int i = 1; i < openSize; i++) {
            if (openList[i].f < openList[smallestIdx].f) {
                smallestIdx = i;
            }
        }

        // Process the current node
        Node current = openList[smallestIdx];
        openList[smallestIdx] = openList[--openSize];
        closedList[current.position.x][current.position.y] = 1;

        // Check if we've reached the goal
        if (is_same_point(current.position, astar->goal)) {
            // Reconstruct the path
            Point path[MAX_PATH_SIZE];
            int pathIdx = 0;

           while (!is_same_point(current.position, astar->start)) {
             path[pathIdx++] = current.position;

            // search about parent node in open list 
             Node *parentNode = find_in_open_list(openList, openSize, current.parent);
             if (parentNode == NULL) {
                parentNode=find_in_closed_list(closedList, current.parent);
              }
              if(parentNode==NULL){
                fprintf(stderr,"Eror: parent node not found during path.\n ");
                return 0;
              }
    current = *parentNode;
           }
           

    path[pathIdx++] = astar->start;


            // Reverse the path
            for (int i = 0; i < pathIdx; i++) {
                astar->path[i] = path[pathIdx - i - 1];
            }
            astar->pathLength = pathIdx;
            return 1; // Success
        }

        // Explore neighbors
        for (int i = 0; i < NUM_DIRECTIONS; i++) {
            Point neighbor = {current.position.x + directions[i].x, current.position.y + directions[i].y};

            if (!is_valid(neighbor, astar->grid) || closedList[neighbor.x][neighbor.y]) {
                continue;
            }

            int gCost = current.g + 1;
            int hCost = calculate_heuristic(neighbor, astar->goal);
            int fCost = gCost + hCost;

            Node *neighborNode = find_in_open_list(openList, openSize, neighbor);

            if (!neighborNode) {
                // Add neighbor to open list
                Node newNode = {fCost, gCost, hCost, neighbor, current.position};
                openList[openSize++] = newNode;
            } else if (gCost < neighborNode->g) {
                // Update costs if the new path is better
                neighborNode->g = gCost;
                neighborNode->f = fCost;
                neighborNode->parent = current.position;
            }
        }
    }
    return 0; // No path found
}

// Retrieves the calculated path
Point* astar_get_path(AStar *astar, int *pathLength) {
    *pathLength = astar->pathLength;
    return astar->path;
}

void create_grid(int grid[GRID_SIZE][GRID_SIZE]) {
 
    int predefined_grid[GRID_SIZE][GRID_SIZE] = {
        {0, 0, 0, 0, 0},
        {0, 1, 1, 1, 0},
        {0, 1, 0, 1, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0}
    };

    // نسخ الخريطة المحددة إلى الخريطة المرسلة
    for (int i = 0; i < GRID_SIZE; i++) {
        for (int j = 0; j < GRID_SIZE; j++) {
            grid[i][j] = predefined_grid[i][j];
        }
    }
}

int run_astar_algorithm(int grid[GRID_SIZE][GRID_SIZE], Point start, Point goal) {
    AStar astar;
    astar_init(&astar, grid, start, goal);

    if (astar_run(&astar)) {
        printf("Path found (%d steps):\n", astar.pathLength);
        for (int i = 0; i < astar.pathLength; i++) {
            printf("(%d, %d)\n", astar.path[i].x, astar.path[i].y);
        }
        return 1; // Succsses
    } else {
        printf("No path found.\n");
        return 0; // Fail
    }
}

