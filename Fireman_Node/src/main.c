#include <stdio.h>
#include <string.h>
//#include "esp_now.h"
//#include "esp_wifi.h"
//#include <esp_timer.h>
//#include <nvs_flash.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
//#include "node_buttonLogic.h"
#include "state_machine.h"
#include "communication.h"
#include "test.h"
#include "a_star.h"


#define MAX_NODES 16
#define BROADCAST_MAC "\xFF\xFF\xFF\xFF\xFF\xFF"
#define TIMEOUT 50000000

/* typedef struct
{
    uint8_t mac[6];
    int64_t last_seen;
} Node;

Node nodes[MAX_NODES]; */

typedef struct
{
    char name[10];
    int distance;
    uint8_t macAddress[6];
    int status; // 0 = ledig 1 = upptagen
    int coordinates[2];
} NodeJojjo;

NodeJojjo test1 = {"esp1", 3, {0x24, 0x62, 0xAB, 0xF3, 0xA8, 0x80}, 0, {3,6}};
NodeJojjo test2 = {"esp2", 6, {0xA8, 0x42, 0xE3, 0xAB, 0xBB, 0x08}, 0, {2,1}};
NodeJojjo test3 = {"esp3", 3, {0xD8, 0xBC, 0x38, 0xE4, 0x51, 0x44}, 1, {6,8}};
NodeJojjo test4 = {"esp4", 9, {0x30, 0xC6, 0xF7, 0x30, 0x38, 0xC0}, 0, {4,2}};

NodeJojjo nodesJojjo[4];
void initializeNodes() {
    nodesJojjo[0] = test1;
    nodesJojjo[1] = test2;
    nodesJojjo[2] = test3;
    nodesJojjo[3] = test4;
}



//uint8_t selfMac[6] = {0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC}; 

int main()
{
    printf("HELLO IN MAIN");
 
    state_machine();
    test_fire_call();
        test_fire_accept1(); 
        test_fire_accept2(); 
        test_fire_accept3(); 
        state_machine();
        state_machine();
        state_machine();
        state_machine();
        state_machine();
        state_machine();
        state_machine();
        state_machine();
        state_machine();
        state_machine();
        

    return 0;

}

//gcc src/main.c src/state_machine.c src/communication.c src/test.c src/a_star.c -o test_program
// ./test_program


/*void initESPNow();
void sendBroadcast(const char *message);
void handleIncomingMessage(const esp_now_recv_info_t *info, const uint8_t *data, int len);
void updateNode(const uint8_t *mac);
void removeInactiveNodes();
void printActiveNodes();

void app_main()
{
    printf("HELLO IN MAIN");


    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_wifi_init(&(wifi_init_config_t)WIFI_INIT_CONFIG_DEFAULT()));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, selfMac));
    printf("Egen MAC-adress: %02X:%02X:%02X:%02X:%02X:%02X\n",
           selfMac[0], selfMac[1], selfMac[2],
           selfMac[3], selfMac[4], selfMac[5]);

    initESPNow();

    setupButtons();
    sendBroadcast("CONNECT");

    int64_t last_heartbeat_time = esp_timer_get_time();
    int64_t last_general_check_time = esp_timer_get_time();

    while (1)
    {
        int64_t now = esp_timer_get_time();
        handleButtonInterrupt(sendBroadcast);
        if (now - last_heartbeat_time >= 6 * TIMEOUT)
        {
            sendBroadcast("HEARTBEAT");
            last_heartbeat_time = now;
        }

        if (now - last_general_check_time >= TIMEOUT)
        {

            removeInactiveNodes();
            printActiveNodes();
            last_general_check_time = now;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void initESPNow()
{
    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(handleIncomingMessage));
    printf("ESP-NOW initialized.\n");

    esp_now_peer_info_t peerInfo = {};
    memcpy(peerInfo.peer_addr, BROADCAST_MAC, 6);
    peerInfo.channel = 0;
    peerInfo.ifidx = WIFI_IF_STA;
    peerInfo.encrypt = false;

    ESP_ERROR_CHECK(esp_now_add_peer(&peerInfo));
}


 * Jag tror vi behöver både den ovan och den under (?)

void registerPeers()
{
    for (int i = 0; i < sizeof(nodesJojjo) / sizeof(nodesJojjo[0]); ++i)
    {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, nodesJojjo[i].macAddress, 6);
        if (!esp_now_is_peer_exist(nodesJojjo[i].macAddress))
        {
            if (esp_now_add_peer(&peerInfo) == ESP_OK)
            {
                printf("Registered peer: %s\n", nodesJojjo[i].name);
            }
            else
            {
                printf("Failed to register peer: %s\n", nodesJojjo[i].name);
            }
        }
    }
}

void sendBroadcast(const char *message)
{
    ESP_ERROR_CHECK(esp_now_send((uint8_t *)BROADCAST_MAC, (uint8_t *)message, strlen(message)));
    printf("Broadcast skickat: %s\n", message);
}

void handleIncomingMessage(const esp_now_recv_info_t *info, const uint8_t *data, int len)
{
    char message[32];
    snprintf(message, sizeof(message), "%.*s", len, data);

    printf("Meddelande mottaget från MAC: %02X:%02X:%02X:%02X:%02X:%02X, Meddelande: %s\n",
           info->src_addr[0], info->src_addr[1], info->src_addr[2],
           info->src_addr[3], info->src_addr[4], info->src_addr[5], message);

    if (strcmp(message, "CONNECT") == 0)
    {
        int prev_count = node_count;
        updateNode(info->src_addr);

        if (node_count > prev_count)
        {
            printf("Ny nod upptäckt, skickar 'HEARTBEAT'-broadcast...\n");
            sendBroadcast("HEARTBEAT");
        }
    }
    else if (strcmp(message, "HEARTBEAT") == 0)
    {
        updateNode(info->src_addr);
    }
    else if (strcmp(message, "HELP") == 0)
    {
        printf("Hjälp begärd från MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
               info->src_addr[0], info->src_addr[1], info->src_addr[2],
               info->src_addr[3], info->src_addr[4], info->src_addr[5]);
    }
}

void updateNode(const uint8_t *mac)
{
    int64_t now = esp_timer_get_time();

    for (int i = 0; i < node_count; i++)
    {
        if (memcmp(nodes[i].mac, mac, 6) == 0)
        {
            nodes[i].last_seen = now;
            return;
        }
    }

    if (node_count < MAX_NODES)
    {
        memcpy(nodes[node_count].mac, mac, 6);
        nodes[node_count].last_seen = now;
        node_count++;
        printf("Ny nod tillagd: %02X:%02X:%02X:%02X:%02X:%02X\n",
               mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    }
}

void removeInactiveNodes()
{
    int64_t now = esp_timer_get_time();

    for (int i = 0; i < node_count; i++)
    {
        if (now - nodes[i].last_seen > TIMEOUT)
        {
            printf("Tar bort inaktiv nod: %02X:%02X:%02X:%02X:%02X:%02X\n",
                   nodes[i].mac[0], nodes[i].mac[1], nodes[i].mac[2],
                   nodes[i].mac[3], nodes[i].mac[4], nodes[i].mac[5]);

            nodes[i] = nodes[node_count - 1];
            node_count--;
            i--;
        }
    }
}

// EXTRA --> Ska ev synas på display
void printActiveNodes()
{
    printf("Aktiva noder:\n");
    for (int i = 0; i < node_count; i++)
    {
        printf("- %02X:%02X:%02X:%02X:%02X:%02X\n",
               nodes[i].mac[0], nodes[i].mac[1], nodes[i].mac[2],
               nodes[i].mac[3], nodes[i].mac[4], nodes[i].mac[5]);
    }
}
*/