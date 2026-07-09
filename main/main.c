/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "tamp/compressor.h"
#include "tamp/decompressor.h"

extern const uint8_t enwik8_100kb_start[] asm("_binary_enwik8_100kb_start");
extern const uint8_t enwik8_100kb_end[]   asm("_binary_enwik8_100kb_end");

extern const uint8_t enwik8_100kb_tamp_start[] asm("_binary_enwik8_100kb_tamp_start");
extern const uint8_t enwik8_100kb_tamp_end[]   asm("_binary_enwik8_100kb_tamp_end");

#define WINDOW_BITS 10

uint8_t window_buffer[1 << WINDOW_BITS];
uint8_t compressed_buffer[60000];
uint8_t decompressed_buffer[100000];

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
#ifdef CONFIG_TAMP_ESP32
        printf("Tamp ESP32 optimizations: enabled\n");
#else
        printf("Tamp ESP32 optimizations: disabled\n");
#endif
    }
    size_t compressed_size;
    {
        /* COMPRESSION */
        size_t input_consumed_size, output_written_size;
        TampCompressor compressor;
        TampConf conf = {
            .window = WINDOW_BITS,
            .literal = 8,
            .use_custom_dictionary = false
        };

        tamp_compressor_init(&compressor, &conf, window_buffer);
        start = esp_timer_get_time();

        tamp_compressor_compress_and_flush(
             &compressor,
             compressed_buffer, sizeof(compressed_buffer), &output_written_size,
             enwik8_100kb_start, 100000, &input_consumed_size,
             false
        );

        end = esp_timer_get_time();
        compressed_size = output_written_size;
        printf("Compression Time (uS): %lld\n", end - start);
        printf("Compression Size (bytes): %zu\n", output_written_size);
        size_t expected_size = enwik8_100kb_tamp_end - enwik8_100kb_tamp_start;
        if(output_written_size != expected_size || memcmp(compressed_buffer, enwik8_100kb_tamp_start, expected_size)){
            printf("Unexpected compressed data. ");
            if(output_written_size != expected_size){
                printf("Size mismatch: expected %zu, got %zu. ", expected_size, output_written_size);
            }
            size_t min_size = expected_size < output_written_size ? expected_size : output_written_size;
            for(size_t i = 0; i < min_size; i++){
                if(compressed_buffer[i] != enwik8_100kb_tamp_start[i]){
                    printf("First difference at position %zu: expected 0x%02x, got 0x%02x\n",
                           i, enwik8_100kb_tamp_start[i], compressed_buffer[i]);
                    break;
                }
            }
        }
        else{
            printf("Compressed to expected tamp data.\n");
        }
    }
    {
        /* DECOMPRESSION OF KNOWN GOOD COMPRESSED DATA */
        memset(decompressed_buffer, 0, sizeof(decompressed_buffer));
        size_t input_consumed_size, output_written_size;
        TampDecompressor decompressor;
        tamp_decompressor_init(&decompressor, NULL, window_buffer, WINDOW_BITS);
        start = esp_timer_get_time();
        tamp_decompressor_decompress(
                &decompressor,
                decompressed_buffer,
                sizeof(decompressed_buffer),
                &output_written_size,
                enwik8_100kb_tamp_start,
                enwik8_100kb_tamp_end - enwik8_100kb_tamp_start,
                &input_consumed_size
                );
        end = esp_timer_get_time();
        printf("\nDecompression of known good data:\n");
        printf("Decompression Time (uS): %lld\n", end - start);
        printf("Decompression Size (bytes): %zu\n", output_written_size);

        if(memcmp(decompressed_buffer, enwik8_100kb_start, 100000)){
            printf("Unexpected decompressed data.\n");
        }
        else{
            printf("Decompression successful.\n");
        }
    }
    {
        /* DECOMPRESSION OF OUR COMPRESSED DATA */
        memset(decompressed_buffer, 0, sizeof(decompressed_buffer));
        size_t input_consumed_size, output_written_size;
        TampDecompressor decompressor;
        tamp_decompressor_init(&decompressor, NULL, window_buffer, WINDOW_BITS);
        start = esp_timer_get_time();
        tamp_decompressor_decompress(
                &decompressor,
                decompressed_buffer,
                sizeof(decompressed_buffer),
                &output_written_size,
                compressed_buffer,
                compressed_size,
                &input_consumed_size
                );
        end = esp_timer_get_time();
        printf("\nDecompression of our compressed data:\n");
        printf("Decompression Time (uS): %lld\n", end - start);
        printf("Decompression Size (bytes): %zu\n", output_written_size);
        printf("Input consumed: %zu\n", input_consumed_size);

        if(output_written_size != 100000 || memcmp(decompressed_buffer, enwik8_100kb_start, 100000)){
            printf("Round-trip compression/decompression failed. ");
            if(output_written_size != 100000){
                printf("Size mismatch: expected 100000, got %zu. ", output_written_size);
            }
            size_t min_size = output_written_size < 100000 ? output_written_size : 100000;
            for(size_t i = 0; i < min_size; i++){
                if(decompressed_buffer[i] != enwik8_100kb_start[i]){
                    printf("First difference at position %zu: expected 0x%02x, got 0x%02x\n",
                           i, enwik8_100kb_start[i], decompressed_buffer[i]);
                    break;
                }
            }
        }
        else{
            printf("Round-trip compression/decompression successful.\n");
        }
    }
    {
        /* REPETITIVE DATA (long/extended matches dominate) */
        const size_t repetitive_size = sizeof(decompressed_buffer);

        /* Generate the synthetic pattern in-place; no dedicated buffer needed. */
        #define REPETITIVE_BYTE(i) ((i) % 257 == 0 ? '!' : (uint8_t)('A' + ((i) % 61)))
        for (size_t i = 0; i < repetitive_size; i++) {
            decompressed_buffer[i] = REPETITIVE_BYTE(i);
        }

        size_t input_consumed_size, output_written_size;
        TampCompressor compressor;
        TampConf conf = {
            .window = WINDOW_BITS,
            .literal = 8,
            .use_custom_dictionary = false
        };
        tamp_compressor_init(&compressor, &conf, window_buffer);
        start = esp_timer_get_time();
        tamp_compressor_compress_and_flush(
             &compressor,
             compressed_buffer, sizeof(compressed_buffer), &output_written_size,
             decompressed_buffer, repetitive_size, &input_consumed_size,
             false
        );
        end = esp_timer_get_time();
        printf("\nRepetitive data:\n");
        printf("Compression Time (uS): %lld\n", end - start);
        printf("Compression Size (bytes): %zu\n", output_written_size);

        size_t comp_size = output_written_size;
        /* Decompress back into the same buffer we compressed from, then verify
         * against the deterministic pattern regenerated on the fly. */
        memset(decompressed_buffer, 0, sizeof(decompressed_buffer));
        TampDecompressor decompressor;
        tamp_decompressor_init(&decompressor, NULL, window_buffer, WINDOW_BITS);
        start = esp_timer_get_time();
        tamp_decompressor_decompress(
                &decompressor,
                decompressed_buffer,
                sizeof(decompressed_buffer),
                &output_written_size,
                compressed_buffer,
                comp_size,
                &input_consumed_size
                );
        end = esp_timer_get_time();
        printf("Decompression Time (uS): %lld\n", end - start);
        printf("Decompression Size (bytes): %zu\n", output_written_size);

        bool repetitive_ok = (output_written_size == repetitive_size);
        for (size_t i = 0; repetitive_ok && i < repetitive_size; i++) {
            if (decompressed_buffer[i] != REPETITIVE_BYTE(i)) {
                repetitive_ok = false;
            }
        }
        if (repetitive_ok) {
            printf("Repetitive round-trip successful.\n");
        }
        else {
            printf("Repetitive round-trip FAILED.\n");
        }
        #undef REPETITIVE_BYTE
    }

    for (int i = 5; i >= 0; i--) {
        printf("Restarting in %d seconds...\n", i);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    printf("Restarting now.\n");
    fflush(stdout);
    esp_restart();
}
