#include "interface.h"

#define DEFAULT_SCAN_LIST_SIZE 3 //max that can be displayed with header

TaskHandle_t statusTaskHandle;
TaskHandle_t wifiStatusTaskHandle;
wifi_config_t wifi_config;
EventGroupHandle_t s_wifi_event_group;
extern int connectedFlag;

void event_handler(void *arg, esp_event_base_t event_base,
                   int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        connectedFlag=0;
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        connectedFlag=1;
    }
}

struct wifiList wifi_scan_ssids(void)
{
    vTaskResume(statusTaskHandle);
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    struct wifiList temp;

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    memset(ap_info, 0, sizeof(ap_info));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    if (ap_count)
        memcpy(temp.list[0], ap_info[0].ssid, 33);
    if (ap_count > 1)
        memcpy(temp.list[1], ap_info[1].ssid, 33);
    else
        temp.list[1][0] = '\0';
    if (ap_count > 2)
        memcpy(temp.list[2], ap_info[2].ssid, 33);
    else
        temp.list[2][0] = '\0';

    vTaskSuspend(statusTaskHandle);
    led_set_color(0, 63, 0);
    return temp;
}

void wifi_connect(uint8_t *ssid, uint8_t *pass)
{
    esp_wifi_disconnect();
    esp_wifi_stop();
    esp_wifi_deinit();

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_ERROR_CHECK(esp_netif_init());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    strcpy((char *)wifi_config.sta.ssid, (char *)ssid);
    strcpy((char *)wifi_config.sta.password, (char *)pass);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI("TAG", "wifi_init_sta finished.");
    ESP_ERROR_CHECK(esp_wifi_connect());

    vTaskResume(statusTaskHandle);
    xTaskCreatePinnedToCore(led_connect_status, "connection_status", 4096, NULL, 10, &wifiStatusTaskHandle, 1);

}