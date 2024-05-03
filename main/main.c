/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_timer.h"

#include "tamp/compressor.h"

extern const uint8_t enwik8_100kb_start[] asm("_binary_enwik8_100kb_start");
extern const uint8_t enwik8_100kb_end[]   asm("_binary_enwik8_100kb_end");

extern const uint8_t enwik8_100kb_tamp_start[] asm("_binary_enwik8_100kb_tamp_start");
extern const uint8_t enwik8_100kb_tamp_end[]   asm("_binary_enwik8_100kb_tamp_end");

#define WINDOW_BITS 10

uint8_t buffer[1 << WINDOW_BITS];
uint8_t output_buffer[100000];

void app_main(void)
{
    uint64_t start, end;
    {
        /* Print chip information */
        esp_chip_info_t chip_info;
        uint32_t flash_size;
        esp_chip_info(&chip_info);
        printf("This is %s chip with %d CPU core(s), %s%s%s%s, ",
               CONFIG_IDF_TARGET,
               chip_info.cores,
               (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
               (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
               (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
               (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");
        unsigned major_rev = chip_info.revision / 100;
        unsigned minor_rev = chip_info.revision % 100;
        printf("silicon revision v%d.%d, ", major_rev, minor_rev);
        if(esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
            printf("Get flash size failed");
            return;
        }
        printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
               (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");
        printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());
    }
    {
        size_t input_consumed_size, output_written_size;
        TampCompressor compressor;
        TampConf conf = {
            .window = WINDOW_BITS,
            .literal = 8,
            .use_custom_dictionary = false
        };

        tamp_compressor_init(&compressor, &conf, buffer);
        start = esp_timer_get_time();

        tamp_compressor_compress_and_flush(
             &compressor,
             output_buffer, sizeof(output_buffer), &output_written_size,
             enwik8_100kb_start, 100000, &input_consumed_size,
             false
        );

        end = esp_timer_get_time();
        printf("Compression Time (uS): %lld\n", end - start);
        printf("Compression Size (bytes): %d\n", output_written_size);
    }


    for (int i = 5; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
