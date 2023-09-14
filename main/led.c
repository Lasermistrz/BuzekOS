#include "interface.h"

static const char *TAG = "built-in led";
static led_strip_handle_t led_strip;
uint8_t connectedFlag = 0;
extern uint8_t pos;
extern uint8_t actPos;
extern uint8_t chosenOption;
extern TaskHandle_t statusTaskHandle;
extern TaskHandle_t wifiStatusTaskHandle;

void led_init(void)
{
    ESP_LOGI(TAG, "LED initialization!");
    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO_NUMBER,
        .max_leds = 1,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    led_strip_clear(led_strip);
}

void led_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    led_strip_set_pixel(led_strip, 0, red, green, blue);
    led_strip_refresh(led_strip);
}

void led_clear(void)
{
    led_strip_clear(led_strip);
}

void led_set_working_status(void)
{
    uint8_t colBlue = 0;
    uint8_t flag = 1;
    while (1)
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if (flag)
        {
            led_set_color(0, 0, colBlue);
            colBlue++;
        }
        else
        {
            led_set_color(0, 0, colBlue);
            colBlue--;
        }
        if (colBlue == 255)
            flag = 0;
        if (colBlue == 0)
        {
            flag = 1;
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }
}

void led_connect_status(void)
{

    uint8_t time = 0;
    while (connectedFlag == 0 && time < 50)
    {
        vTaskDelay(200 / portTICK_PERIOD_MS);
        time++;
    }
    vTaskDelay(200 / portTICK_PERIOD_MS);
    vTaskSuspend(statusTaskHandle);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    if (connectedFlag == 1)
    {
        led_set_color(0, 31, 0);
    }
    else
    {
        led_set_color(31, 0, 0);
    }
    pos = 1;
    chosenOption = 0;
    lcd_print_menu(pos, actPos, connectedFlag);
    vTaskDelete(wifiStatusTaskHandle);
}