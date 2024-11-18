#ifndef NODE_BUTTON_LOGIC_H
#define NODE_BUTTON_LOGIC_H

#include <stdint.h>

void setupButtons(void);

void handleButtonPresses(void (*sendBroadcast)(const char *message));

#endif
