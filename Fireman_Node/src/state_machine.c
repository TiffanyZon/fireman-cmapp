#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
// #include <esp_now.h>
// #include <esp_wifi.h>
#include "state_machine.h"
#include "communication.h"
#include "test.h"
#include "a_star.h"
#define STATE_SEARCH 0
#define STATE_REQUEST 1
#define STATE_FIRE 2
#define STATE_PERSON 3
#define STATE_COLLEAGUE 4
#define STATE_WAIT 5
#define STATE_HELP_PERSON_OUT 6
#define STATE_FIND 7

#define FIRE "FIRE"
#define PERSON "PERSON"
#define COLLEAGUE "COLLEAGUE"
#define ACCEPTPERSON "ACCEPTPERSON"
#define ACCEPTFIRE "ACCEPTFIRE"
#define ACCEPTCOLL "ACCEPTCOLL"
#define ARRIVED "ARRIVED"

#define QUEUE_SIZE 10
#define MESSAGE_SIZE 32


static int currentState = STATE_SEARCH;
static int lastState;
static int waitingCounter = 0;           // Väntar 5 ticks innan svarsmeddelanden på uppdrag hanteras
char checkedMessage[15];
bool ifSelected = false;

static int destX; // Uppdragets x-koordinat
static int destY;
int X;
int Y;

int amountReplied = 0;
int amountHelping;
int amountArrived = 0;

double step_x = 1.0; // Hur mycket noden rör sig i x-led, test
double step_y = 0.5; // Hur mycket noden rör sig i y-led, test 

bool arrived = false;

char sentMissionAccept[20] = "x";




void search()
{ 
    myCoordX += step_x; 
    myCoordY += step_y;
}

/**
 * När brandmän hittar något random. Stannar kvar i statet ett tag
 */
void find() // OM KNAPP HELP
{
/*
    if (cell == FIRE) // OM KNAPP 2
    {
        // BROADCASTA FIRE MESSAGE TILL ALLA
    }
    if (cell == PERSON) // OM KNAPP 3
    {
        //  BROADCASTA PERSONMESSAGE TILL ALLA
    } */
}


/**
 * 
 */
void announce_arrival(int amountHelping)
{
    char sendMessage[25];

    for (int i = 0; i < amountHelping; ++i){ // Meddelar alla i helpers-listan att noden är framme vid brand / skadad person
        snprintf(sendMessage, sizeof(sendMessage), "ARRIVED (%d,%d)", 0, 0); // behöver inte ha med int här?
        send_message(helpers[i].senderMac, sendMessage);
    }
}


void walk_person_out()
{
    // Gå till utgång, här behövs samspel mellan noderna som går ut tillsammans
    // när framme PRESS BUTTON 2
}


void find_path(){
  int grid[GRID_SIZE][GRID_SIZE];
    initialize_grid(grid); 
    // Point start = {espX.coordinates[0], espX.coordinates[1]};
    Point start = {myCoordX, myCoordY};
    Point goal = {destX, destY};


   /* if (cell == PERSON) {                         
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (grid[i][j] == PERSON) {
                    goal.x = i;
                    goal.y = j;
                    break;
                }
            }
        }
    } else if (cell == FIRE) {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                if (grid[i][j] == FIRE) {
                    goal.x = i;
                    goal.y = j;
                    break;
                }
            }
        } 
    } else {
        goal.x = 0; 
        goal.y = 0;
    } */

    if (grid[goal.x][goal.y] == 0) {
        printf("Running A* from (%d, %d) to (%d, %d)\n", start.x, start.y, goal.x, goal.y);
        run_astar_algorithm(grid, start, goal); 
    } else {
        printf("Invalid goal point (%d, %d)\n", goal.x, goal.y);
    } 

}


void state_machine()
{
    // while (1)
    // {
    sleep(1);

    //printf("MY COORDINATES: %d %d\n", myCoordX, myCoordY);

    //  vTaskDelay(pdMS_TO_TICKS(1000));
    Message newMessage = check_messages();

    if (newMessage.message[0] == '\0') { // Kontrollera om strängen är tom
        printf("No message!\n");
        strcpy(checkedMessage, "empty");
    }    

    else{
    sscanf(newMessage.message, "%s (%d,%d)", checkedMessage, &X, &Y);
    printf("Message says: %s \n",checkedMessage);    
    }

    switch (currentState){

    case STATE_SEARCH:
    printf("I STATE_SEARCH\n");

        lastState = currentState;

        if (strcmp(checkedMessage, FIRE) == 0){ // LÄGGA IN KNAPPTRYCK ATT MEDDELA "JAG ÄR TILLGÄNGLIG", TYP?? 
            strcpy(sentMissionAccept, ACCEPTFIRE);
            destX = X;
            destY = Y;
            mission_reply(newMessage, sentMissionAccept, 0);
            currentState = STATE_REQUEST;
        }

        else if (strcmp(checkedMessage, PERSON) == 0){ // LÄGGA IN KNAPPTRYCK ATT MEDDELA "JAG ÄR TILLGÄNGLIG", TYP?? 
            strcpy(sentMissionAccept, ACCEPTPERSON);
            mission_reply(newMessage, sentMissionAccept, 0);
            currentState = STATE_REQUEST;
        }

        else if (strcmp(checkedMessage, COLLEAGUE) == 0){ // LÄGGA IN KNAPPTRYCK ATT MEDDELA "JAG ÄR TILLGÄNGLIG", TYP?? 
            strcpy(sentMissionAccept, ACCEPTCOLL);
            mission_reply(newMessage, sentMissionAccept, 0);
            currentState = STATE_REQUEST;
        }

        else{
            search(); 
        }

        break;

    case STATE_FIRE:
    printf("IN STATE FIRE");

        lastState = currentState;

        if (strcmp(checkedMessage, PERSON) == 0){   // LÄGGA IN KNAPPTRYCK ATT MEDDELA "JAG ÄR TILLGÄNGLIG", TYP?? 
            strcpy(sentMissionAccept, ACCEPTPERSON);
            mission_reply(newMessage, sentMissionAccept, 1);
            currentState = STATE_REQUEST;
        }

        else if (strcmp(checkedMessage, COLLEAGUE) == 0){   // LÄGGA IN KNAPPTRYCK ATT MEDDELA "JAG ÄR TILLGÄNGLIG", TYP?? 
            strcpy(sentMissionAccept, ACCEPTCOLL);
            mission_reply(newMessage, sentMissionAccept, 1);
            currentState = STATE_REQUEST;
        }

        else{
           // arrived = walk_to_destination(); // Var går vi? 
            find_path();
            if (arrived == true){           // KNAPPTRYCK FÖR ATT MEDDELA ATT BRANDMANNEN ÄR FRAMME VID BRANDEN?
                announce_arrival(4);
                currentState = STATE_WAIT;
            }

            else if (arrived == false){
                printf("Walking to destination\n");
            }
        }
        
        break;   

    case STATE_PERSON:

        lastState = currentState;

        if (strcmp(checkedMessage, COLLEAGUE) == 0){
            strcpy(sentMissionAccept, ACCEPTCOLL);
            mission_reply(newMessage, sentMissionAccept, 2);
            currentState = STATE_REQUEST;
        }

        else{
          //  arrived = walk_to_destination();
            if (arrived == true){
                announce_arrival(2);
                currentState = STATE_WAIT;
            }

            else if (arrived == false){
                // inget
            }
        }

        break;

    case STATE_COLLEAGUE:

        lastState = currentState;

        // arrived = walk_to_destination();
        if (arrived == true){
            currentState = STATE_WAIT;
        }

        else if (arrived == false){
            // inget
        }

        break;     

    case STATE_REQUEST:
    printf("I STATE_REQUEST\n");

        ++waitingCounter;

        if ((strcmp(checkedMessage, ACCEPTFIRE) == 0) || (strcmp(checkedMessage, ACCEPTPERSON) == 0) || (strcmp(checkedMessage, ACCEPTCOLL) == 0)){
            put_in_help_list(newMessage, amountReplied);
            ++amountReplied;
            printf("Amount replied: %d\n", amountReplied);
        }        

        else if (waitingCounter > 6 && checkedMessage != sentMissionAccept){

            if (strcmp(sentMissionAccept, ACCEPTFIRE) == 0){
                ifSelected = sort_and_choose_helpers(amountReplied, 4);
                printf("Selected or not: %d ", ifSelected);

                if (ifSelected == true){
                    currentState = STATE_FIRE;                    
                }

                else if (ifSelected == false){
                    currentState = lastState;
                }
            }

            else if (strcmp(sentMissionAccept, ACCEPTPERSON) == 0){
                ifSelected = sort_and_choose_helpers(amountReplied, 2);

                if (ifSelected == true){
                    currentState = STATE_PERSON;
                }

                else if (ifSelected == false){
                    currentState = lastState;
                }
            }

            else if (strcmp(sentMissionAccept, ACCEPTCOLL) == 0){
                ifSelected = sort_and_choose_helpers(amountReplied, 1);

                if (ifSelected == true){
                    currentState = STATE_COLLEAGUE;
                }

                else if (ifSelected == false){
                    currentState = lastState;
                }
            }

            waitingCounter = 0;
            amountReplied = 0;
        }
        
        printf("Waiting counter: %d\n", waitingCounter);

        break;
    
    case STATE_WAIT:

        if (strcmp(checkedMessage, ARRIVED) == 0){
            ++amountArrived;
        }

        if (amountArrived == amountHelping){
            amountArrived = 0;

            if (lastState == STATE_FIRE){   // KNAPPTRYCK FÖR ATT MEDDELA ATT ELDEN ÄR SLÄCKT? 
                currentState = STATE_SEARCH;
            }

            else if (lastState == STATE_PERSON){
                currentState = STATE_HELP_PERSON_OUT;
                destX = 0;
                destY = 0;
            }
        }

        break;

    case STATE_HELP_PERSON_OUT:

        lastState = currentState;

        // arrived = walk_outside();
        if (arrived == true){
        }

        else if (arrived == false){
            // inget
        }

        break;    

    case STATE_FIND:
        printf("I FIND\n");
        //    find(); om brandmann hittar något eget

        break;

    }
}