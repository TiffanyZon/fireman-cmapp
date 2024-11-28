#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//#include <esp_now.h>
#include <math.h>
#include "communication.h"
#include "state_machine.h"


#define MESSAGE_SIZE 64
#define QUEUE_SIZE 10
#define MAX_HELPERS 10


static int queueStart = 0, queueEnd = 0;
int distance;
int accepting = 0;
bool selected;

uint8_t senderMac[6];
uint8_t macToAll[6];

int personalStatus = 0;
int helpersArrived = 0;
int helpersNeeded = 10;
uint8_t selfMac[6] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};  // Exempel-MAC
uint8_t macToAll[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

Message helpers[MAX_HELPERS];

int myCoordX = 0;
int myCoordY = 0;

static char receivedMessage[15];
static int receivedX = 0;
static int receivedY = 0;

static int destinationX = 0;
static int destinationY = 0;

int receivedDistance = 0;
int receivedStatus = 0;

Message emptyMessage = { {0}, {0} };




/**
 * Används när en annan noden svarar på ett call. För att spara deras (och egen) information
 */

typedef struct {
    uint8_t senderMac[6]; // MAC-adressen för noden (den andras)
    int status; // Status (t.ex. 0 = ledig, 1 = upptagen, etc.)
    float distance;       // Distansen till händelsen
} Response;


Message receivedMessageQueue[QUEUE_SIZE];
Response responseList[8];

//char sentMissionMessage[20];
char sentMissionMessage[20] = "x";


void receiveCallback(const uint8_t *macAddr, const uint8_t *incomingData, int dataLen)
{
    if ((queueEnd + 1) % QUEUE_SIZE == queueStart){
        printf("Queue is full\n");
        return;
    }
    memcpy(receivedMessageQueue[queueEnd].senderMac, macAddr, 6); // MAC-adress 6-byte
    memcpy(receivedMessageQueue[queueEnd].message, incomingData, dataLen);
    receivedMessageQueue[queueEnd].message[dataLen] = '\0'; // Nullterminera
    queueEnd = (queueEnd + 1) % QUEUE_SIZE;

    printf("Message added to queue: %s\n", receivedMessageQueue[(queueEnd - 1 + QUEUE_SIZE) % QUEUE_SIZE].message);
   /*  printf("MAC Address saved: %02X:%02X:%02X:%02X:%02X:%02X\n",
       receivedMessageQueue[(queueEnd - 1 + QUEUE_SIZE) % QUEUE_SIZE].senderMac[0],
       receivedMessageQueue[(queueEnd - 1 + QUEUE_SIZE) % QUEUE_SIZE].senderMac[1],
       receivedMessageQueue[(queueEnd - 1 + QUEUE_SIZE) % QUEUE_SIZE].senderMac[2],
       receivedMessageQueue[(queueEnd - 1 + QUEUE_SIZE) % QUEUE_SIZE].senderMac[3],
       receivedMessageQueue[(queueEnd - 1 + QUEUE_SIZE) % QUEUE_SIZE].senderMac[4],
       receivedMessageQueue[(queueEnd - 1 + QUEUE_SIZE) % QUEUE_SIZE].senderMac[5]); */
}


/**
 * 
 */
Message check_messages(){

  //  char receivedMessage[10];
    char sendMessage[25];
    int x; // senders status
    int y; // senders distans

    if(queueStart != queueEnd){
        printf("Reading a message\n");
        Message message = receivedMessageQueue[queueStart];
        queueStart = (queueStart + 1) % QUEUE_SIZE; // Flytta köpekaren
      //  sscanf(message.message, "%s (%d,%d)", receivedMessage, &receivedX, &receivedY);
        return message;
    }

    else{ // När kön är tom. Vad kan jag returna? 

    return emptyMessage;
    }
}


/**
 * Tar emot messageobjektet som noden mottagit, nodens svar på anropet och nodens aktuella tillståndsstatus. 
 */
void mission_reply(Message message, const char *messageType, int personalStatus)
{
    char sendMessage[20];
    char sent;

    sscanf(message.message, "%s, (%d,%d)",sent, &receivedX, &receivedY);     
    distance = calculate_distance(receivedX,receivedY);
    snprintf(sendMessage, sizeof(sendMessage), "%s (%d,%d)",messageType, personalStatus, distance);
    send_message(macToAll,sendMessage);
}


/**
 * 
 */
void put_in_help_list(Message message, int number)
{
    memcpy(responseList[number].senderMac, message.senderMac, 6);
    sscanf(message.message, "(%d,%d)", &receivedStatus, &receivedDistance);   
    responseList[number].status = receivedStatus;
    responseList[number].distance = receivedDistance;    
}


/**
 * 
 */
bool sort_and_choose_helpers(int replied, int amountNeeded)
{

    char sendMessage[25];
    int contacted = 0;
    bool selected = false;

         printf("HELPERS NEEDED: %d\n",amountNeeded);
     
    for (int i = 0; i < replied - 1; ++i){  

        for (int j = 0; j < replied - i - 1; ++j){ 

            if (responseList[j].status > responseList[j + 1].status){
                Response temp = responseList[j];
                responseList[j] = responseList[j + 1];
                responseList[j + 1] = temp;
            } 

            else if ((responseList[j].status == responseList[j + 1].status) && (responseList[j].distance > responseList[j + 1].distance)){
                Response temp = responseList[j];
                responseList[j] = responseList[j + 1];
                responseList[j + 1] = temp;
            }    
        }
    }

    for (int i = 0; i < replied && contacted < amountNeeded; ++i){ // SKA SKICKA TILL RESPONSELIST ÄR TOM 
         memcpy(helpers[i].senderMac, responseList[i].senderMac, 6);        

        if (memcmp(responseList[i].senderMac, selfMac, 6) == 0){           // Den egna noden är en av de valda hjälparna 
            // PRESS BUTTON .... TO CONFIRM 
            selected = true;
            printf("I AM SELECTED\n");                  
        }

        contacted++;
        printf("HELPERS CONTACTED: %d\n",contacted);
      }

      return selected;    
}


/**
 * 
 */
void send_message(const uint8_t *macAddr, const char *message) 
{
 //   printf("Mock send message\n");    
    receiveCallback(selfMac, (uint8_t *)message, strlen(message));
}


/*
void send_message(const uint8_t *macAddr, const char *message){ 
                     
            esp_err_t result = esp_now_send(macAddr, (const uint8_t *)message, strlen(message));
            if (result == ESP_OK)
            {
                printf("Message sent to %s successfully\n", selfMac);
            }
            else
            {
                printf("Failed to send message to %s\n", selfMac);
            }
}*/


/**
 * Beräknar avstånd
 * Retunerar avstånd i float
 */
float calculate_distance(int x, int y)
{
    int x2 = 3; // egna nodens koordinater
    int y2 = 2;

    float deltaX = x - x2;
    float deltaY = y - y2;
    float distance = sqrt(deltaX * deltaX + deltaY * deltaY);

    printf("My distance is: %f\n", distance);

    return distance;
}