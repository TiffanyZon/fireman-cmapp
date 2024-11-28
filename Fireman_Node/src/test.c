#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include "test.h"
#include "communication.h"
#include "state_machine.h"

/*NodeJojjo test1 = {"esp1", 3, {0x24, 0x62, 0xAB, 0xF3, 0xA8, 0x80}, 0, {3,6}};
NodeJojjo test2 = {"esp2", 6, {0xA8, 0x42, 0xE3, 0xAB, 0xBB, 0x08}, 0, {2,1}};
NodeJojjo test3 = {"esp3", 3, {0xD8, 0xBC, 0x38, 0xE4, 0x51, 0x44}, 1, {6,8}};
NodeJojjo test4 = {"esp4", 9, {0x30, 0xC6, 0xF7, 0x30, 0x38, 0xC0}, 0, {4,2}}; 

uint8_t selfMac[6] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC};              */

    double step_x = 1.0; // Hur mycket noden rör sig i x-led
    double step_y = 0.5; // Hur mycket noden rör sig i y-led

void test_fire_call() {
    // Simulerar ett inkommande FIRE-meddelande
    uint8_t senderMac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    char fireMessage[] = "FIRE (5,8)";  // Exempelkoordinater
    receiveCallback(senderMac, (uint8_t *)fireMessage, strlen(fireMessage));   
}
void test_fire_accept1() {
    // Simulerar ett inkommande FIRE-meddelande
    uint8_t senderMac[6] = {0x24, 0x62, 0xAB, 0xF3, 0xA8, 0x80};
    char fireMessage[] = "ACCEPTFIRE (3,6)";  // Exempelkoordinater
    receiveCallback(senderMac, (uint8_t *)fireMessage, strlen(fireMessage));   
}

void test_fire_accept2() {
    // Simulerar ett inkommande FIRE-meddelande
    uint8_t senderMac[6] = {0xA8, 0x42, 0xE3, 0xAB, 0xBB, 0x08};
    char fireMessage[] = "ACCEPTFIRE (2,1)";  // Exempelkoordinater
    receiveCallback(senderMac, (uint8_t *)fireMessage, strlen(fireMessage));   
}

void test_fire_accept3() {
    // Simulerar ett inkommande FIRE-meddelande
    uint8_t senderMac[6] = {0xD8, 0xBC, 0x38, 0xE4, 0x51, 0x44};
    char fireMessage[] = "ACCEPTFIRE (6,8)";  // Exempelkoordinater
    receiveCallback(senderMac, (uint8_t *)fireMessage, strlen(fireMessage));   
}

void test_fire_accept4() {
    // Simulerar ett inkommande FIRE-meddelande
    uint8_t senderMac[6] = {0x30, 0xC6, 0xF7, 0x30, 0x38, 0xC0};
    char fireMessage[] = "ACCEPTFIRE (4,2)";  // Exempelkoordinater
    receiveCallback(senderMac, (uint8_t *)fireMessage, strlen(fireMessage));   
}


void walkAround(){
    
        myCoordX += step_x;
        myCoordY += step_y;
}