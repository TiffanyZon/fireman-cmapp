#include <stdio.h>
#include "driver/gpio.h"
#include "node_buttonLogic.h"

#define HELP_BUTTON GPIO_NUM_34
#define YES_BUTTON GPIO_NUM_35

volatile bool button_pressed = false;

void button_isr_handler(void *arg)
{
    button_pressed = true;
}

void setupButtons()
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << HELP_BUTTON) | (1ULL << YES_BUTTON),
        .pull_up_en = GPIO_PULLUP_ENABLE,
    };
    gpio_config(&io_conf);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(HELP_BUTTON, button_isr_handler, NULL);
}

void handleButtonInterrupt(void (*sendBroadcast)(const char *message))
{
    if (button_pressed)
    {
        button_pressed = false;
        printf("HELP-knappen trycktes!\n");
        sendBroadcast("HELP");
    }
}
