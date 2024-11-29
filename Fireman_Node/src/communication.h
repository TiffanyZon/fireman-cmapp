#ifndef COMMUNICATION_H
#define COMMUNICATION_H

//#include "state_machine.h"

#define MESSAGE_SIZE 32
#define MAX_HELPERS 10

typedef struct
{
    uint8_t senderMac[6];
    char message[MESSAGE_SIZE];
} Message;

extern Message helpers[MAX_HELPERS]; // Array för att spara de noder som hjälper till


extern uint8_t selfMac[6];
extern int myCoordX;
extern int myCoordY;

void receiveCallback(const uint8_t *macAddr, const uint8_t *incomingData, int dataLen);
Message check_messages();
void mission_reply(Message message, const char *messageType, int personalStatus);
bool sort_and_choose_helpers(int replied, int amountNeeded);
void put_in_help_list(Message message, int number);

void send_message(const uint8_t *macAddr, const char *message);
float calculate_distance(int x, int y);

#endif