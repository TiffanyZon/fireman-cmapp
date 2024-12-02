#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//#include <esp_now.h>
#include <math.h>
#include "communication.h"
#include "state_machine.h"


#define MESSAGE_SIZE 32
#define QUEUE_SIZE 25
#define LIST_SIZE 10

static int queueStart = 0, queueEnd = 0; // För meddelandekön 
bool selected; // Boolean som visar om noden blivit vald till uppdrag eller inte
int personalStatus = 0; // Visar nodens egna status vid tilldelning av uppdrag
int helpersNeeded = 0; // Hur många noder som behövs för ett specifikt uppdrag 

int receivedX; // Uppdragets x-koordinat
int receivedY; // Uppdragets y-koordinat
float distance = 0; // Nodens avstånd till uppdraget
float deltaX = 0; // Används vid uträkning av avstånd
float deltaY = 0; // Används vid uträkning av avstånd

int receivedDistance = 0; // Distans till uppdrag för nod som svarat på uppdragsmeddelande
int receivedStatus = 0; // Status på nod som svarat på uppdragsmeddelande
int contacted; // Antal noder som kontaktats för att hjälpa till på uppdrag

int myCoordX = 0; // Används just nu för test
int myCoordY = 0; // // Används just nu för test

char irrelevantMessage; // Del av meddelande, när den delen inte används i funktionen
char sendMessage[MESSAGE_SIZE];

/**
 * Används när en nod svarar på en förfrågan. För att spara information som behövs för att prioritera hjälp.
 */
typedef struct {
    uint8_t senderMac[6]; // Nodens MAC-adress
    int status;           // Nodens status 
    float distance;       // Nodens distans till händelsen
} Response;

Response responseList[LIST_SIZE];
Message helpers[];
Message receivedMessageQueue[QUEUE_SIZE]; // Kön för inkommande meddelanden
Message emptyMessage = { {0}, {0} }; // Ett tomt messageobjekt. Används när kön är tom

uint8_t selfMac[6] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};  // Exempel-MAC, för test
uint8_t macToAll[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}; //Används för test




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
 * Kallas på varje "tick" i tillståndsmaskinen innan ett state startat. Meddelandets innehåll avgör vad som ska hända i tillståndet.
 */
Message check_messages(){

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
 * 
 * Kallar på funktion som beräknar avstånd till händelsens koordinater. 
 * 
 * Skickar meddelande till samtliga noder. Meddelandet innehåller nodens svar på anropet, aktuell tillståndsstatus och avstånd till händelsen. 
 */
void mission_reply(Message message, const char *messageType, int personalStatus)
{
    char sendMessage[MESSAGE_SIZE];
    
    sscanf(message.message, "%s, (%d,%d)",irrelevantMessage, &receivedX, &receivedY);     
    distance = calculate_distance(receivedX,receivedY);
    snprintf(sendMessage, sizeof(sendMessage), "%s (%d,%d)",messageType, personalStatus, distance);
    send_message(macToAll,sendMessage);
}


/**
 * Samtliga noder som skickar ett acceptmeddelande av samma typ som aktuell nod sparas i en lista. Nodernas MAC-adress, status och avstånd till händelsen sparas.
 * Aktuell nods information sparas också i listan.
 */
void put_in_help_list(Message message, int number)
{
    memcpy(responseList[number].senderMac, message.senderMac, 6);
    sscanf(message.message, "(%d,%d)", &receivedStatus, &receivedDistance);   
    responseList[number].status = receivedStatus;
    responseList[number].distance = receivedDistance;    
}


/**
 * Samtliga noder som svarat på uppdrag, som därav ligger i listan responseList[], sorteras utifrån två parametrar. Första sortering är utifrån nodernas aktuella status.
 * Noder med status 0 hamnar högst upp på listan, status 1 efter det etc.
 * Därefter sorteras noderna efter avstånd till händelsen. Dvs noderna hamnar i ordningen: alla med status 0 från minsta avstånd till största, alla med status 1 från minsta avstånd till största etc.
 * 
 * Om aktuell nod blir utvald att hjälpa till så sparas den och de andra noderna som ska hjälpa till i listan helpers[]. Detta för att noden vid senare tillfälle ska kunna kommunicera med endast de noderna.  
 */
bool sort_and_choose_helpers(int replied, int helpersNeeded)
{

    char sendMessage[MESSAGE_SIZE];
    contacted = 0;
    selected = false;

         printf("HELPERS NEEDED: %d\n",helpersNeeded);
     
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

    for (int i = 0; i < replied && contacted < helpersNeeded; ++i){ 
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
 * Just denna används endast för test.
 */
void send_message(const uint8_t *macAddr, const char *message) 
{
 //   printf("Mock send message\n");    
    receiveCallback(selfMac, (uint8_t *)message, strlen(message));
}


/**
 * Används när noden ska skicka ett meddelande. 
 */
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
float calculate_distance(int receivedX, int receivedY)
{
   // int x2 = 3; // egna nodens koordinater för test 
   // int y2 = 2; // egna nodens koordinater för test 

    deltaX = receivedX - myCoordX;
    deltaY = receivedY - myCoordY;
    distance = sqrt(deltaX * deltaX + deltaY * deltaY);

    printf("My distance is: %f\n", distance);

    return distance;
}
