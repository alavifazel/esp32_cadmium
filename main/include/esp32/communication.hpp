#ifndef ESP32COM_H
#define ESP32COM_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "led_strip.h"
#include "sdkconfig.h"
#include <string.h>
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include <string>
#include "esp_netif.h"
#include <sys/param.h>
#include "esp_system.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include <time.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include <string.h>
#include "freertos/event_groups.h"
#include "esp_log.h"
#include <netdb.h>
#include <arpa/inet.h>
#define STATION_MAXIMUM_RETRY 10

// Depending on the AP's authentication mode, uncomment one of the macros below:
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
// #define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK

// Depending on the AP's SAE mode uncomment one group of macros
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""

// #define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
// #define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID

// #define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
// #define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID

static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static int s_retry_num = 0;

//_______________END________________

#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_MAX_STA_CONN 4
#define KEEPALIVE_IDLE 4000
#define KEEPALIVE_INTERVAL 4000
#define KEEPALIVE_COUNT 4000
#define PORT 4000
#define CONFIG_EXAMPLE_IPV4 1

//____________________________________
//__________GPIO_CONFIG_______________
#define GPIO_OUTPUT_IO_0 0
#define GPIO_OUTPUT_IO_1 0
#define GPIO_OUTPUT_PIN_SEL ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0 1
#define GPIO_INPUT_IO_1 1
#define GPIO_INPUT_PIN_SEL ((1ULL << GPIO_INPUT_IO_0) | (1ULL << GPIO_INPUT_IO_1))
#define ESP_INTR_FLAG_DEFAULT 0

static QueueHandle_t gpio_evt_queue = NULL;

//_____________________________________
//_______________END___________________

namespace cadmium::iot
{

    enum Phase
    {
        WAITING_FOR_DATA,
        DATA_RECEIVED
    };

    class ESP32COM
    {
    public:
        ESP32COM(){}

        void log(const char *msg) const
        {
            ESP_LOGI("LOG", "%s", msg);
        }

        void log(std::string msg) const
        {
            ESP_LOGI("LOG", "%s", msg.c_str());
        }

        //____________GPIO______________
        //______________________________

        static void IRAM_ATTR gpio_isr_handler(void *arg)
        {
            uint32_t gpio_num = (uint32_t)arg;
            xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
        }

        // static void gpio_task_example(void* arg)
        // {
        //     uint32_t io_num;
        //     for(;;) {
        //         if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
        //             printf("GPIO[%"PRIu32"] intr, val: %d\n", io_num, gpio_get_level(io_num));
        //         }
        //     }
        // }
        //______________________________
        //_____________END______________

    protected:
        void printChipInfo()
        {
            /* Print chip information */
            esp_chip_info_t chip_info;
            uint32_t flash_size;
            esp_chip_info(&chip_info);
            printf("This is %s chip with %d CPU core(s), WiFi%s%s%s, ",
                   CONFIG_IDF_TARGET,
                   chip_info.cores,
                   (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
                   (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
                   (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

            unsigned major_rev = chip_info.revision / 100;
            unsigned minor_rev = chip_info.revision % 100;
            printf("silicon revision v%d.%d, ", major_rev, minor_rev);
            if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
            {
                printf("Get flash size failed");
                return;
            }

            printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
                   (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

            printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
        }

        void gpioOut(int gpio, int delay, uint32_t level)
        {
            vTaskDelay(delay / portTICK_PERIOD_MS);
            gpio_set_level((gpio_num_t)GPIO_OUTPUT_IO_0, level);
            gpio_set_level((gpio_num_t)GPIO_OUTPUT_IO_1, level);
        }

        static void event_handler(void *arg, esp_event_base_t event_base,
                                  int32_t event_id, void *event_data)
        {
            if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
            {
                esp_wifi_connect();
            }
            else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
            {
                if (s_retry_num < STATION_MAXIMUM_RETRY)
                {
                    esp_wifi_connect();
                    s_retry_num++;
                    ESP_LOGI("Station", "retry to connect to the AP");
                }
                else
                {
                    xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                }
                ESP_LOGI("Station", "connect to the AP fail");
            }
            else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
            {
                ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
                ESP_LOGI("Station", "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
                s_retry_num = 0;
                xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            }
        }

        void wifi_init_sta(const char *ssid, const char *pwd, int channel = 1, int maxSTAConn = 4)
        {
            s_wifi_event_group = xEventGroupCreate();

            ESP_ERROR_CHECK(esp_netif_init());

            ESP_ERROR_CHECK(esp_event_loop_create_default());
            esp_netif_create_default_wifi_sta();

            wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
            ESP_ERROR_CHECK(esp_wifi_init(&cfg));

            esp_event_handler_instance_t instance_any_id;
            esp_event_handler_instance_t instance_got_ip;
            ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                                ESP_EVENT_ANY_ID,
                                                                &event_handler,
                                                                NULL,
                                                                &instance_any_id));
            ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                                IP_EVENT_STA_GOT_IP,
                                                                &event_handler,
                                                                NULL,
                                                                &instance_got_ip));

            wifi_config_t wifi_config = {};
            strcpy((char *)wifi_config.sta.ssid, ssid);
            strcpy((char *)wifi_config.sta.password, pwd);
            wifi_config.sta.threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD;
            wifi_config.sta.sae_pwe_h2e = ESP_WIFI_SAE_MODE;

            //     .sta = {
            //         .ssid = EXAMPLE_ESP_WIFI_SSID,
            //         .password = EXAMPLE_ESP_WIFI_PASS,
            //         .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            //         .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
            //         .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
            //     },
            // };
            ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
            ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
            ESP_ERROR_CHECK(esp_wifi_start());

            ESP_LOGI("Station", "wifi_init_sta finished.");
            EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                                   WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                                   pdFALSE,
                                                   pdFALSE,
                                                   portMAX_DELAY);
            if (bits & WIFI_CONNECTED_BIT)
            {
                ESP_LOGI("Station", "connected to ap SSID:%s password:%s",
                         ssid, pwd);
            }
            else if (bits & WIFI_FAIL_BIT)
            {
                ESP_LOGI("Station", "Failed to connect to SSID:%s, password:%s",
                         ssid, pwd);
            }
            else
            {
                ESP_LOGE("Station", "UNEXPECTED EVENT");
            }
        }
        void connectToWifi(const char *ssid, const char *pwd, int channel = 1, int maxSTAConn = 4)
        {
            esp_err_t ret = nvs_flash_init();
            if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
            {
                ESP_ERROR_CHECK(nvs_flash_erase());
                ret = nvs_flash_init();
            }
            ESP_ERROR_CHECK(ret);

            ESP_LOGI("Station", "ESP_WIFI_MODE_STA");
            wifi_init_sta(ssid, pwd, channel, maxSTAConn);
        }

        void tcpClient(std::string hostIP, int port, std::string payload)
        {
            char rx_buffer[128];
            char host_ip[16];
            strcpy(host_ip, hostIP.c_str());

            int addr_family = 0;
            int ip_protocol = 0;

            while (1)
            {
                struct sockaddr_in dest_addr;
                inet_pton(AF_INET, host_ip, &dest_addr.sin_addr);
                dest_addr.sin_family = AF_INET;
                dest_addr.sin_port = htons(port);
                addr_family = AF_INET;
                ip_protocol = IPPROTO_IP;

                int sock = socket(addr_family, SOCK_STREAM, ip_protocol);
                if (sock < 0)
                {
                    ESP_LOGE("TCPClient", "Unable to create socket: errno %d", errno);
                    break;
                }
                ESP_LOGI("TCPClient", "Socket created, connecting to %s:%d", host_ip, port);

                int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
                if (err != 0)
                {
                    ESP_LOGE("TCPClient", "Socket unable to connect: errno %d", errno);
                    break;
                }
                ESP_LOGI("TCPClient", "Successfully connected");

                while (1)
                {
                    int err = send(sock, payload.c_str(), strlen(payload.c_str()), 0);
                    if (err < 0)
                    {
                        ESP_LOGE("TCPClient", "Error occurred during sending: errno %d", errno);
                        break;
                    }

                    int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
                    // Error occurred during receiving
                    if (len < 0)
                    {
                        ESP_LOGE("TCPClient", "recv failed: errno %d", errno);
                        break;
                    }
                    // Data received
                    else
                    {
                        rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
                        ESP_LOGI("TCPClient", "Received %d bytes from %s:", len, host_ip);
                        ESP_LOGI("TCPClient", "%s", rx_buffer);
                    }
                }

                if (sock != -1)
                {
                    ESP_LOGE("TCPClient", "Shutting down socket and restarting...");
                    shutdown(sock, 0);
                    close(sock);
                }
            }
        }
    };
}

#endif