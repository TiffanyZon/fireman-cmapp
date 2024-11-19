#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "state_machine.h"
#define STATE_SEARCH 0
#define STATE_FIND 1
#define STATE_HANDLE_SITUATON 2
#define STATE_HELP_COLLEAGUE 3
#define STATE_WALK_PERSON_OUT 4

#define FIRE 'F'
#define PERSON 'P'
#define EMPTY 'X'
#define QUEUE_SIZE 10
#define MESSAGE_SIZE 64

static int state = STATE_SEARCH; // Startstate
static char cell = EMPTY;        // brandmannens egna cellstatus
static int help_arrived = 0;     // Antal som kommit till undsättning
static int contact_amount = 0;   // Antal som kontaktats för att hjälpa till
static int nodeX_status = 0; // 0 ledig, 1 eld, 2 person
static int queueStart = 0, queueEnd = 0;

typedef struct
{
    uint8_t macAddr[6];
    char message[MESSAGE_SIZE];
} Message;

typedef struct
{
    char name[10];
    int distance;
    uint8_t macAddress[6];
    int status; // 0 = ledig 1 = upptagen
    int coordinates[2];
} Node;

Node esp1 = {"esp1", 3, {0x24, 0x62, 0xAB, 0xF3, 0xA8, 0x80}, 0, {5,8}};
Node esp2 = {"esp2", 6, {0xA8, 0x42, 0xE3, 0xAB, 0xBB, 0x08}, 0, {5,8}};
Node esp3 = {"esp3", 3, {0xD8, 0xBC, 0x38, 0xE4, 0x51, 0x44}, 1, {5,8}};
Node esp4 = {"esp4", 9, {0x30, 0xC6, 0xF7, 0x30, 0x38, 0xC0}, 0, {5,8}};

Node nodes[4];
void initializeNodes() {
    nodes[0] = esp1;
    nodes[1] = esp2;
    nodes[2] = esp3;
    nodes[3] = esp4;
}

Node *helpers[4]; // Array för att spara de noder som hjälper till
Message messageQueue[QUEUE_SIZE];

Node espX = {"espX", 0, {0x30, 0xC6, 0xF7, 0x30, 0x38, 0xC0}, 0, {5,8}};

/**
 *  Var ska denna funktionen ligga?
 */
void registerPeers()
{
    for (int i = 0; i < sizeof(nodes) / sizeof(nodes[0]); ++i)
    {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, nodes[i].macAddress, 6);
        if (!esp_now_is_peer_exist(nodes[i].macAddress))
        {
            if (esp_now_add_peer(&peerInfo) == ESP_OK)
            {
                printf("Registered peer: %s\n", nodes[i].name);
            }
            else
            {
                printf("Failed to register peer: %s\n", nodes[i].name);
            }
        }
    }
}


// esp_now_register_recv_cb(receiveCallbac); Denna ligger i main!
void receiveCallback(const uint8_t *macAddr, const uint8_t *incomingData, int dataLen)
{
    if ((queueEnd + 1) % QUEUE_SIZE == queueStart)
    {
        printf("Queue is full\n");
        return;
    }
    memcpy(messageQueue[queueEnd].macAddr, macAddr, 6); // MAC-adress 6-byte
    memcpy(messageQueue[queueEnd].message, incomingData, dataLen);
    messageQueue[queueEnd].message[dataLen] = '\0'; // Nullterminera
    queueEnd = (queueEnd + 1) % QUEUE_SIZE;

    printf("Message put in que: %02X:%02X:%02X:%02X:%02X:%02X - %s\n",
           macAddr[0], macAddr[1], macAddr[2], macAddr[3], macAddr[4], macAddr[5],
           messageQueue[(queueEnd - 1 + QUEUE_SIZE) % QUEUE_SIZE].message);
}


void search() // OM BÅDA HÄNDER SAMTIDIGT. VAD GÖR VI DÅ? VAD ÄR PRIO? 
{
    if(queueStart != queueEnd){ // tagit emot meddelande genom callback
        Message msg = messageQueue[queueStart];
        
         if (strncmp(msg.message, "HELP", 4) == 0) { // Läser men tar inte bort meddelandet
            state = STATE_HELP_COLLEAGUE;
            nodeX_status = 1; // upptagen
         }
    }

    if (cell != EMPTY)
    {
       
        state = STATE_FIND;
    }
}


void find() // OM KNAPP 1
{
    
    if(cell == FIRE) // OM KNAPP 2
    {
        nodeX_status = 1;
        contact_amount = 4;       
    }
    if(cell == PERSON) // OM KNAPP 3
    {        
        nodeX_status = 2;
        contact_amount = 2;
    }

    ask_for_help();    
    state = STATE_HANDLE_SITUATON;
}


void send_message(const uint8_t *macAddress, const char *message){
                     
            esp_err_t result = esp_now_send(macAddress, (const uint8_t *)message, strlen(message));
            if (result == ESP_OK)
            {
                printf("Message sent to %s successfully\n", macAddress);
            }
            else
            {
                printf("Failed to send message to %s\n", macAddress);
            }
}


void ask_for_help()
{
    
    int contacted = 0; // hur många som kontaktats för att hjälpa till 

    /** 
     * Listan med alla tillgängliga noder hämtas först. 
     * (Avståndet till samtliga noder räknas ut med Hanans algoritm) 
     * Sorterar alla noder baserat på avstånd och sätter dessa först i listan 
     */

    for (int i = 0; i < 4 - 1; ++i)
    {
        for (int j = 0; j < 4 - i - 1; ++j)
        {
            if (nodes[j].distance > nodes[j + 1].distance)
            {
                Node temp = nodes[j];
                nodes[j] = nodes[j + 1];
                nodes[j + 1] = temp;
            }
        }
    }

    // Kollar status på noderna och skickar request till första lediga
    for (int i = 0; i < 4 && contacted < contact_amount; ++i)
    { // går igenom loopen tills rätt antal kontaktats
        if (nodes[i].status == 0)
        { 
            char sendMessage[50];
            snprintf(sendMessage, sizeof(sendMessage), "HELP (%d,%d)", espX.coordinates[0], espX.coordinates[1]);

            send_message(helpers[i]->macAddress, sendMessage);

            helpers[contacted] = &nodes[i];
            contacted++;
        }
    } 
}


void handle_situation()
{
    if (queueStart != queueEnd)
    {
        Message msg = messageQueue[queueStart];
        queueStart = (queueStart + 1) % QUEUE_SIZE;

        if (strstr(msg.message, "ARRIVED"))
        {
            help_arrived++;
            printf("Helper has confirmed arrival. Total arrived: %d\n", help_arrived);
        }
    }
    if (help_arrived == contact_amount) // När alla hjälpande noder skickat bekräftelse att de är framme
    { 
        char sendMessage[50];
        printf("All helpers have arrived!!");
        if (cell==PERSON)
        {
            snprintf(sendMessage, sizeof(sendMessage), "WALK (%d,%d)", espX.coordinates[0], espX.coordinates[1]);
            state = STATE_WALK_PERSON_OUT;
        }
        else
        {
            snprintf(sendMessage, sizeof(sendMessage), "DONE (%d,%d)", espX.coordinates[0], espX.coordinates[1]);
            nodeX_status = 0;
            state = STATE_SEARCH;
        }

        for (int i = 0; i < contact_amount; ++i)
        {
        send_message(helpers[i]->macAddress, sendMessage);
          
        }
    }
    help_arrived = 0;
}

void help_colleauge(){
  // få information om att hjälpa till. Ska gå till kollegan som behöver hjälp. 
  // 
        Message msg = messageQueue[queueStart]; // Läs meddelandet från kön
        queueStart = (queueStart + 1) % QUEUE_SIZE; // Flytta köpekaren

}

void walk_person_out()
{
    // Gå till utgång, här behövs samspel mellan noderna som går ut tillsammans
    // när framme
    nodeX_status = 0;
    state = STATE_SEARCH;
}


void state_machine()
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        switch (state)
        {

        case STATE_SEARCH:
            search();
            break;

        case STATE_FIND:
            find();
            break;

        case STATE_HANDLE_SITUATON:
            handle_situation();
            break;

        case STATE_HELP_COLLEAGUE:
            help_colleauge();
            break;

        case STATE_WALK_PERSON_OUT:
            walk_person_out();
            break;
        }
    }
}