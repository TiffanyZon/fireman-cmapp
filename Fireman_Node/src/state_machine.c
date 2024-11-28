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
#define MESSAGE_SIZE 64


static int currentState = STATE_SEARCH;
static int lastState;
static int waitingCounter = 0;           // Väntar 5 ticks innan svarsmeddelanden på uppdrag hanteras
char checkedMessage[15];
bool ifSelected = false;

int amountReplied = 0;
int amountHelping;
int amountArrived = 0;

bool arrived = false;

char sentMissionAccept[20] = "x";




void search()
{ // OM BÅDA HÄNDER SAMTIDIGT. VAD GÖR VI DÅ? VAD ÄR PRIO?
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

/*void find_path(){
   int grid[GRID_SIZE][GRID_SIZE];
    create_grid(grid); 
    Point start ={espX.coordinates[0],espX.coordinates[1]};
    Point start = {0, 3}; 
    Point goal = {3, 3};  
    run_astar_algorithm(grid, start, goal);   
}*/

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
    sscanf(newMessage.message, "%s", checkedMessage);
    printf("Message says: %s \n",checkedMessage);
    }

    switch (currentState){

    case STATE_SEARCH:
    printf("I STATE_SEARCH\n");

        lastState = currentState;

        if (strcmp(checkedMessage, FIRE) == 0){
            strcpy(sentMissionAccept, ACCEPTFIRE);
            mission_reply(newMessage, sentMissionAccept, 0);
            currentState = STATE_REQUEST;
        }

        else if (strcmp(checkedMessage, PERSON) == 0){
            strcpy(sentMissionAccept, ACCEPTPERSON);
            mission_reply(newMessage, sentMissionAccept, 0);
            currentState = STATE_REQUEST;
        }

        else if (strcmp(checkedMessage, COLLEAGUE) == 0){
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

        if (strcmp(checkedMessage, PERSON) == 0){
            strcpy(sentMissionAccept, ACCEPTPERSON);
            mission_reply(newMessage, sentMissionAccept, 1);
            currentState = STATE_REQUEST;
        }

        else if (strcmp(checkedMessage, COLLEAGUE) == 0){
            strcpy(sentMissionAccept, ACCEPTCOLL);
            mission_reply(newMessage, sentMissionAccept, 1);
            currentState = STATE_REQUEST;
        }

        else{
           // arrived = walk_to_destination();
            if (arrived == true){
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

            if (lastState == STATE_FIRE){
                currentState = STATE_SEARCH;
            }

            else if (lastState == STATE_PERSON){
                currentState = STATE_HELP_PERSON_OUT;
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