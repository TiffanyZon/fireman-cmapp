#include <stdint.h>
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H 

void search();
void find();
void registerPeers();
void recieveCallback(const uint8_t *macAddr, const uint8_t *incomingData, int dataLen);
void ask_for_help();
void handle_situation();
void walk_person_out();
void state_machine();
void send_message();
void help_colleauge();

#endif