#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "lora/lora.h"

static const char *TAG = "LORA_RECEIVER";

void app_main(void)
{
    ESP_LOGD(TAG, "Starting LoRa initialization");
    if (lora_init() == 0) {
        ESP_LOGE(TAG, "LoRa init failed!");
        return;
    }
    ESP_LOGD(TAG, "LoRa initialized successfully");

    lora_set_frequency(433000000);
    lora_set_spreading_factor(7);
    lora_set_bandwidth(125E3);
    lora_set_coding_rate(8);
    lora_enable_crc();
    lora_set_preamble_length(12);
    lora_set_sync_word(0x34);

    ESP_LOGI(TAG, "LoRa Receiver Ready!");
    lora_receive();

    uint32_t count = 0;

    while (1) {
        if (lora_received()) {
            ESP_LOGD(TAG, "DIO0 triggered, reading packet");
            uint8_t buffer[255];
            int len = lora_receive_packet(buffer, sizeof(buffer));
            if (len > 0) {
                count++;
                // In dữ liệu dưới dạng hex vì 0xAA không phải ký tự in được
                char hex_str[512];
                int offset = 0;
                for (int i = 0; i < len; i++) {
                    offset += snprintf(hex_str + offset, sizeof(hex_str) - offset, "%02X ", buffer[i]);
                }
                ESP_LOGI(TAG, "Received packet %lu (len=%d): %s", count, len, hex_str);
                ESP_LOGI(TAG, "RSSI: %d dBm, SNR: %.2f dB", lora_packet_rssi(), lora_packet_snr());
            } else {
                ESP_LOGW(TAG, "Received empty or invalid packet (len=%d)", len);
            }
            lora_receive();
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}