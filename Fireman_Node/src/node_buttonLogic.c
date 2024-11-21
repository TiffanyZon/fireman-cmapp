#include <stdio.h>
#include "driver/gpio.h"
#include "node_buttonLogic.h"

volatile bool button_help_pressed = false;
volatile bool button_yes_pressed = false;

void button_isr_handler_help(void *arg)
{
    button_help_pressed = true;
}

void button_isr_handler_yes(void *arg)
{
    button_yes_pressed = true;
}

void setupButtons()
{
    gpio_config_t io_conf_help = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << HELP_BUTTON),
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf_help);

    gpio_config_t io_conf_yes = {
        .intr_type = GPIO_INTR_NEGEDGE,
        .mode = GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << YES_BUTTON),
        .pull_up_en = GPIO_PULLUP_DISABLE,
    };
    gpio_config(&io_conf_yes);

    gpio_install_isr_service(0);

    gpio_isr_handler_add(HELP_BUTTON, button_isr_handler_help, NULL);
    gpio_isr_handler_add(YES_BUTTON, button_isr_handler_yes, NULL);
}

void handleButtonInterrupt(void (*sendBroadcast)(const char *message))
{
    if (button_help_pressed)
    {
        button_help_pressed = false; // Återställ flaggan
        printf("HELP-knappen trycktes!\n");
        sendBroadcast("HELP");
    }

    if (button_yes_pressed)
    {
        button_yes_pressed = false; // Återställ flaggan
        printf("JA-knappen trycktes!\n");
        sendBroadcast("YES");
    }
}
