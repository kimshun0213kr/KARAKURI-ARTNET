#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "protocol_examples_common.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "esp_task_wdt.h"

#define PORT 3333
#define BROADCAST_IP "192.168.4.2"

static const char *TAG = "udp_ap_sender";

char broadcast_ip_list[8][16];

void check_connect(void)
{
    for(int i=1;i<=8;i++){
        sprintf(broadcast_ip_list[i],"%s%d","192.168.4.",i);
    }
    for(int j=1;j<=8;j++){
        ESP_LOGI(TAG,"%s\n",broadcast_ip_list[j]);
    }
}

void wifi_init_softap(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = "ESP32_AP",
            .ssid_len = 0,
            .channel = 1,
            .password = "12345678",
            .max_connection = 4,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen((char *)wifi_config.ap.password) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi AP started. SSID:%s", wifi_config.ap.ssid);
}

// void udp_send_task(void *pvParameters)
// {
//     esp_task_wdt_add(NULL);
//     const char *ip_addr = (const char *)pvParameters;
    
//     int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
//     if (sock < 0) {
//         ESP_LOGE(TAG, "Failed to create socket");
//         vTaskDelete(NULL);
//         return;
//     }

//     struct sockaddr_in dest_addr = {
//         .sin_addr.s_addr = inet_addr(ip_addr),
//         .sin_family = AF_INET,
//         .sin_port = htons(PORT),
//     };

//     TickType_t xLastWakeTime = xTaskGetTickCount();
//     const TickType_t xFrequency = pdMS_TO_TICKS(20);

//     while (1) {
//         char *payload = "Hello from AP!";
//             int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
//             if (err < 0) {
//                 ESP_LOGE(TAG, "Error sending UDP packet to %s: errno %d",ip_addr, errno);
//             } else {
//                 ESP_LOGI(TAG, "Message sent: %s\tTo:%s", payload,ip_addr);
//             }
//         // vTaskDelay(pdMS_TO_TICKS(20));  // 1/10秒ごとに送信
//         esp_task_wdt_reset();
//         vTaskDelayUntil(&xLastWakeTime,xFrequency);
//     }

//     close(sock);
//     vTaskDelete(NULL);
// }

void udp_send_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0) {
        ESP_LOGE(TAG, "Failed to create socket");
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in dest_addr[4];
    const char* ip_list[] = {"192.168.4.2", "192.168.4.3", "192.168.4.4", "192.168.4.5"};

    for (int i = 0; i < 4; i++) {
        dest_addr[i].sin_family = AF_INET;
        dest_addr[i].sin_port = htons(PORT);
        dest_addr[i].sin_addr.s_addr = inet_addr(ip_list[i]);
    }

    const char *payload = "Hello from AP!";
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(20); // 20ms間隔

    while (1) {
        for (int i = 0; i < 4; i++) {
            int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr[i], sizeof(dest_addr[i]));
            if (err < 0) {
                ESP_LOGE(TAG, "Send error to %s: %d", ip_list[i], errno);
            } else {
                ESP_LOGI(TAG, "Sent to %s: %s", ip_list[i], payload);
            }
            esp_task_wdt_reset();
        }
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }

    close(sock);
    vTaskDelete(NULL);
}


void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init_softap();

    xTaskCreate(udp_send_task, "udp_send_task", 4096, NULL, 5, NULL);
}
