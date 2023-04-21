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
#define BLINK_GPIO CONFIG_BLINK_GPIO
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
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


#define EXAMPLE_ESP_WIFI_CHANNEL 1
#define EXAMPLE_MAX_STA_CONN 4
#define KEEPALIVE_IDLE 4000
#define KEEPALIVE_INTERVAL 4000
#define KEEPALIVE_COUNT 4000
#define PORT 4000
#define CONFIG_EXAMPLE_IPV4 1

//____________________________________
//__________GPIO_CONFIG_______________
#define GPIO_OUTPUT_IO_0    0
#define GPIO_OUTPUT_IO_1    0   
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_0) | (1ULL<<GPIO_OUTPUT_IO_1))
#define GPIO_INPUT_IO_0     1
#define GPIO_INPUT_IO_1     1
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1))
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
        ESP32COM(const char *tag)
            : tag{tag}
        {
        }

        void log(const char *msg) const
        {
            ESP_LOGI(tag, "%s", msg);
        }

        void log(std::string msg) const
        {
            ESP_LOGI(tag, "%s", msg.c_str());
        }

        //____________GPIO______________
        //______________________________

        static void IRAM_ATTR gpio_isr_handler(void* arg)
        {
            uint32_t gpio_num = (uint32_t) arg;
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
        const char *tag;


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

    void gpioOut(int gpio, int delay, uint32_t level) {
        vTaskDelay(delay / portTICK_PERIOD_MS);
        gpio_set_level((gpio_num_t)GPIO_OUTPUT_IO_0, level);
        gpio_set_level((gpio_num_t)GPIO_OUTPUT_IO_1, level);
    }
};
}

#endif