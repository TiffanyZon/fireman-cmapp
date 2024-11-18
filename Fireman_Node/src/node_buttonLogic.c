#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "node_buttonLogic.h"

#define HELP_BUTTON GPIO_NUM_34

void setupButtons()
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << HELP_BUTTON),
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);
}

void handleButtonPresses(void (*sendBroadcast)(const char *message))
{
    static int lastHelpState = 1;

    int helpState = gpio_get_level(HELP_BUTTON);

    if (helpState == 0 && lastHelpState == 1)
    {
        printf("HELP-knappen är nedtryckt!\n");
        sendBroadcast("HELP");
    }

    lastHelpState = helpState;
}
