#ifndef NODE_BUTTON_LOGIC_H
#define NODE_BUTTON_LOGIC_H

#include <stdint.h>

void setupButtons(void);
void handleButtonInterrupt(void (*sendBroadcast)(const char *message));

void button_isr_handler(void *arg);

#endif
