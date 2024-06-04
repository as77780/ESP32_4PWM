#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/ledc.h"
#include "lwip/err.h"
#include "lwip/sys.h"
/*
#include "nvs_flash.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_device.h"
*/

#define OUTPUT_PIN_1 16 // Переназначенный выход для ШИМ #1
#define OUTPUT_PIN_2 17 // Переназначенный выход для ШИМ #2
#define OUTPUT_PIN_3 18 // Переназначенный выход для ШИМ #1
#define OUTPUT_PIN_4 19 // Переназначенный выход для ШИМ #2

static const char *TAG = "wifi_pwm";

static void init_pwm(void) {
    ledc_timer_config_t ledc_timer_0 = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_10_BIT,  // Разрешение 10 бит
        .freq_hz          = 1000,               // Начальная частота 1 кГц
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer_0);

    ledc_timer_config_t ledc_timer_1 = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .timer_num        = LEDC_TIMER_1,
        .duty_resolution  = LEDC_TIMER_10_BIT,  // Разрешение 10 бит
        .freq_hz          = 1000,               // Начальная частота 1 кГц
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer_1);

    ledc_timer_config_t ledc_timer_2 = {
          .speed_mode       = LEDC_HIGH_SPEED_MODE,
          .timer_num        = LEDC_TIMER_2,
          .duty_resolution  = LEDC_TIMER_10_BIT,  // Разрешение 10 бит
          .freq_hz          = 1000,               // Начальная частота 1 кГц
          .clk_cfg          = LEDC_AUTO_CLK
      };
      ledc_timer_config(&ledc_timer_2);

    ledc_timer_config_t ledc_timer_3 = {
               .speed_mode       = LEDC_HIGH_SPEED_MODE,
               .timer_num        = LEDC_TIMER_3,
               .duty_resolution  = LEDC_TIMER_10_BIT,  // Разрешение 10 бит
               .freq_hz          = 1000,               // Начальная частота 1 кГц
               .clk_cfg          = LEDC_AUTO_CLK
           };
           ledc_timer_config(&ledc_timer_3);

    ledc_channel_config_t ledc_channel_0 = {
        .gpio_num   = OUTPUT_PIN_1,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_0,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 512,  // Половина максимального значения для 10-битного разрешения
        .hpoint     = 0
    };
    ledc_channel_config(&ledc_channel_0);

    ledc_channel_config_t ledc_channel_1 = {
        .gpio_num   = OUTPUT_PIN_2,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel    = LEDC_CHANNEL_1,
        .timer_sel  = LEDC_TIMER_1,
        .duty       = 512,  // Половина максимального значения для 10-битного разрешения
        .hpoint     = 0
    };
    ledc_channel_config(&ledc_channel_1);

    ledc_channel_config_t ledc_channel_2 = {
           .gpio_num   = OUTPUT_PIN_3,
           .speed_mode = LEDC_HIGH_SPEED_MODE,
           .channel    = LEDC_CHANNEL_2,
           .timer_sel  = LEDC_TIMER_2,
           .duty       = 512,  // Половина максимального значения для 10-битного разрешения
           .hpoint     = 0
       };
       ledc_channel_config(&ledc_channel_2);

       ledc_channel_config_t ledc_channel_3 = {
              .gpio_num   = OUTPUT_PIN_4,
              .speed_mode = LEDC_HIGH_SPEED_MODE,
              .channel    = LEDC_CHANNEL_3,
              .timer_sel  = LEDC_TIMER_3,
              .duty       = 512,  // Половина максимального значения для 10-битного разрешения
              .hpoint     = 0
          };
          ledc_channel_config(&ledc_channel_3);
}

static esp_err_t pwm_get_handler(httpd_req_t *req) {
    char* buf;
    size_t buf_len;
    char output[4], freq[8];

    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) {
        buf = malloc(buf_len);
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK) {
            httpd_query_key_value(buf, "output", output, sizeof(output));
            httpd_query_key_value(buf, "freq", freq, sizeof(freq));
            int channel = atoi(output);
            int frequency = atoi(freq);
            if (frequency < 100 || frequency > 10000) {
                frequency = 1000;  // Ограничение частоты в допустимых пределах
            }
            if (channel == 0) {
                ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_0, frequency);
            }else if (channel == 1) {
                ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_1, frequency);
            }else if (channel == 2) {
                ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_2, frequency);
            }else if (channel == 3) {
                ledc_set_freq(LEDC_HIGH_SPEED_MODE, LEDC_TIMER_3, frequency);
            }
            free(buf);
        }
    }
    const char resp[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<!DOCTYPE html><html><body><h1>PWM Updated</h1></body></html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t root_get_handler(httpd_req_t *req) {
    const char resp[] = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n"
                        "<!DOCTYPE html><html><head><title>ESP32 PWM Control</title></head><body>"
                        "<h1>ESP32 PWM Control</h1>"
                        "<p>Control the PWM signals using the sliders below:</p>"
                        "<input type='range' min='100' max='10000' value='1000' id='freq1' onchange='updatePWM(0)'><br>"
                        "<input type='range' min='100' max='10000' value='1000' id='freq2' onchange='updatePWM(1)'><br>"
    		            "<input type='range' min='100' max='10000' value='1000' id='freq3' onchange='updatePWM(2)'><br>"
    		            "<input type='range' min='100' max='10000' value='1000' id='freq4' onchange='updatePWM(3)'><br>"
                        "<script>"
                        "function updatePWM(output) {"
                        "  var freq = document.getElementById('freq' + (output + 1)).value;"
                        "  var xhr = new XMLHttpRequest();"
                        "  xhr.open('GET', '/pwm?output=' + output + '&freq=' + freq, true);"
                        "  xhr.send();"
                        "}"
                        "</script>"
                        "</body></html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static const httpd_uri_t pwm = {
    .uri       = "/pwm",
    .method    = HTTP_GET,
    .handler   = pwm_get_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t root = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = root_get_handler,
    .user_ctx  = NULL
};

static void start_webserver(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &root);
        httpd_register_uri_handler(server, &pwm);
    }
}

void app_main(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "Keenetic-6779",
            .password = "pr9hpEVv"
        }
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());

    init_pwm();
    start_webserver();
}
