#include <string.h>
#include <stdlib.h>
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

#define maxDataLength 512
#define LOOP_WAIT_TIME 22

static const char *TAG = "DMX_TRANSPORTER";

char broadcast_ip_list[8][16];

void createData(uint8_t data[maxDataLength])
{
    srand((unsigned int)time(NULL));
    data[0] = rand() * 255 + 1;
    for (int i = 1; i < maxDataLength; i++)
    {
        data[i] = i % 256;
    }
}

void random16(uint8_t tmpData[maxDataLength], char *outBuffer)
{
    char *p = outBuffer;
    for (int j = 0; j < maxDataLength; j++)
    {
        p += sprintf(p, "%02X", tmpData[j]);
    }
    *p = '\0';
}

void check_connect(void)
{
    for (int i = 1; i <= 8; i++)
    {
        sprintf(broadcast_ip_list[i], "%s%d", "192.168.4.", i);
    }
    for (int j = 1; j <= 8; j++)
    {
        ESP_LOGI(TAG, "%s\n", broadcast_ip_list[j]);
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
            .authmode = WIFI_AUTH_WPA_WPA2_PSK},
    };
    if (strlen((char *)wifi_config.ap.password) == 0)
    {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi AP started. SSID:%s", wifi_config.ap.ssid);
}

void udp_send_task(void *pvParameters)
{
    esp_task_wdt_add(NULL);

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sock < 0)
    {
        ESP_LOGE(TAG, "Failed to create socket");
        vTaskDelete(NULL);
        return;
    }

    struct sockaddr_in dest_addr[4];
    const char *ip_list[] = {"192.168.4.2", "192.168.4.3", "192.168.4.4", "192.168.4.5"};

    for (int i = 0; i < 4; i++)
    {
        dest_addr[i].sin_family = AF_INET;
        dest_addr[i].sin_port = htons(PORT);
        dest_addr[i].sin_addr.s_addr = inet_addr(ip_list[i]);
    }

    const char *payload = "Hello from AP!";

    char buffer[maxDataLength * 2 + 1];
    uint8_t intData[maxDataLength];

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(LOOP_WAIT_TIME); // 20ms間隔

    while (1)
    {
        for (int i = 0; i < 4; i++)
        {
            createData(intData);
            random16(intData, buffer);
            // int err = sendto(sock, payload, strlen(payload), 0, (struct sockaddr *)&dest_addr[i], sizeof(dest_addr[i]));
            int err = sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&dest_addr[i], sizeof(dest_addr[i]));
            if (err < 0)
            {
                ESP_LOGE(TAG, "Send error to %s: %d", ip_list[i], errno);
            }
            else
            {
                // ESP_LOGI(TAG, "Sent to %s: %s", ip_list[i], payload);
                ESP_LOGI(TAG, "Sent to %s", ip_list[i]);
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
