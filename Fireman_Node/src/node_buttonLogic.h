#ifndef NODE_BUTTON_LOGIC_H
#define NODE_BUTTON_LOGIC_H

#include <stdint.h>

#define HELP_BUTTON GPIO_NUM_34
#define YES_BUTTON GPIO_NUM_35

void setupButtons(void);
void handleButtonInterrupt(void (*sendBroadcast)(const char *message));

void button_isr_handler_help(void *arg);
void button_isr_handler_yes(void *arg);

#endif
