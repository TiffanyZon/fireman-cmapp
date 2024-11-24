#include "A_STAR.H"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#define INF 0x3f3f3f3f

//Direction: UP, Down, Left, Right
const Point directios[]= {{-1,0},{1,0},{0,-1},{0,1}};
const int NUM_DIRECTIONS=4;


//Utility function to calculate Manhattan distance
static int calculate_heuristic(Point a, Point b){
    return abs(a.x-b.x)+abs(a.y-b.y);
}

//check if a point is within the gird and walkable
static int is_same_point(Point a, Point b){
    return a.x==b.x && a.y==b.y;
}

//Finds a node in the open list
static Node* find_in_open_list(Node *openList, int openSize, Point Position){
    for (int i=0; i<openSize; i++ ){
        if(is_same_point(openList[i].position, position)){
            return &openList[i];
        }
    }
    return NULL;
}

//Initialize the A* instance
void astar_init(AStar *astar, int grid[GRID_SIZE][GRID_SIZE], Point start, Point goal){
    memcpy(astar->grid, grid, sizeof(astar->grid));
    astar->start=start;
    astar->goal=goal;
    astar->pathLength=0;
}

//Run the A* algorithm
int astar_run(AStar *astar){
    Node openList[GRID_SIZE *GRID_SIZE];
    int openSize=0;
    int closedList[GRID_SIZE][GRID_SIZE];
    memset(closedList,0,sizeof(closedList));

    Node startNode={0,0,calculate_heuristic(astar->start, astar->goal), astar->atart, {-1,-1}};
    openList[openSize++]= startNode;

    while (openSize>0){
        // Find the node with the smallest f in the open list
        int smallestIdx=0;
        for(int i=1; i<openSize; i++){
            if(openList[i].f < openList[smallestIdx].f){
                smallestIdx=i;
            }
        }
        
        //Process the current node
        Node current = openList[smallestIdx];
        openList[smallestIdx]= openList[--openSize];
        closedList[current.position.x][currentposition.y]=1;

        //check if we've reached the goal
        if(is_same_point(current.position, astar->goal)){
            //reconstruct the path
            Point path[MAX_PATH_SIZE];
            int pathIdx=0;

            while(current.parent.x != -1 && current.parent.y !=-1){
                path[pathIdx++]= current.position;
                current= find_in_open_list(openList, openSize, current.parent)? *find_in_open_list(openList, openSize, current, current.parent): current;
            }
            path[pathIdx]=astar->start;


            //reverse the path
            for(int i=0; i<pathIdx; i++){
                astar->path[i]= path[pathIdx-i-1];
            }
            astar->pathLength=pathIdx;
            return 1; //success
    
        }


        //Explore neighbors
        for(int i=0; i<NUM_DIRECTIONS; i++){
            Point neighbor = {current.position.x+ directions[i].x,current.position.y+directions[i].y };

            if(!is_valid(neighbor, astar->grid)|| closedList[neighbor.x][neighbor.y]){
                continue;
            }

            int gCost=current.g+1;
            int hCost= calculate_heuristic(neighbor, astar->goal);
            int fCost= gCost+hCost;

            Node *neighborNode= find_in_open_list(openList, openSize, neighbor);

            if(!neighborNode){
                //add to open list
                Node newNode={fCost, gCost, hCost, neighbor, current.position};
                openList[openSize++]= newNode;
            }else if (gCost< neighborNode->g)
            {
                neighborNode-> g= gCost;
                neighborNode-> f=fCost;
                neighborNode->parent = current.position;
            }
            
        }
    }
    return 0; //no path found
}

Point* astar_get_path(AStar *astar, int *pathLength){
    *pathLength= astar->pathLength;
    return aster->path;
}

int main(){
    
     int grid[GRID_SIZE][GRID_SIZE] = {
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 1, 1, 1, 1, 1, 1, 1, 0},
        {0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
        {0, 1, 1, 1, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 1, 0, 1, 0, 1, 0},
        {0, 1, 1, 0, 1, 0, 0, 0, 1, 0},
        {0, 1, 1, 0, 0, 0, 1, 1, 1, 0},
        {0, 0, 0, 0, 1, 1, 1, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };

    Point start = {0, 0};
    Point goal = {9, 9};

    AStar astar;
    astar_init(&astar, grid, start, goal);

    if (astar_run(&astar)) {
        int pathLength;
        Point *path = astar_get_path(&astar, &pathLength);

        printf("Path found (%d steps):\n", pathLength);
        for (int i = 0; i < pathLength; i++) {
            printf("(%d, %d)\n", path[i].x, path[i].y);
        }
    } else {
        printf("No path found.\n");
    }

    return 0;

}
