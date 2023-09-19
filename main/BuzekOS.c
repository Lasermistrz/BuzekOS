#include "interface.h"

uint8_t pos = 1;           // chosen option position (menu iterator)
uint8_t actPos = 1;        // first displayed option (not used for now)
uint8_t itePass = 32;      // char iterator for input password
uint8_t itePassLength = 0; // stores password length
uint8_t pass[20] = {'\0'}; // stores password
uint8_t iteHost = 32;      // char iterator for input host address
uint8_t iteHostLength = 0; // stores ping host address length
uint8_t host[20] = {'\0'}; // stores ping host address
/**
 * @brief chosenOption is variable that stores info about chosen option like option 1 (connect to wifi)
 * or option 2 = input wifi password, 3 = send ping input host address
 *
 *  0 = nothing chosen,
 *  1 = connect to wifi,
 *  2 = input wifi password,
 *  3 = send ping input host address
 */
uint8_t chosenOption = 0;
struct wifiList wList; // stores list of ssids

extern TaskHandle_t statusTaskHandle;
extern TaskHandle_t wifiStatusTaskHandle;
extern uint8_t connectedFlag;
extern void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data);

button_config_t gpio_btn_cfg_down = {
    .type = BUTTON_TYPE_GPIO,
    .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
    .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
    .gpio_button_config = {
        .gpio_num = BUTTON_GPIO_DOWN_PIN,
        .active_level = 0,
    },
};

button_config_t gpio_btn_cfg_up = {
    .type = BUTTON_TYPE_GPIO,
    .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
    .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
    .gpio_button_config = {
        .gpio_num = BUTTON_GPIO_UP_PIN,
        .active_level = 0,
    },
};

button_config_t gpio_btn_cfg_select = {
    .type = BUTTON_TYPE_GPIO,
    .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
    .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
    .gpio_button_config = {
        .gpio_num = BUTTON_GPIO_SELECT_PIN,
        .active_level = 0,
    },
};

void button_single_click_cb_up(void *arg, void *usr_data)
{
    if (chosenOption == 0 || chosenOption == 1)
    {
        if (pos > 1)
            pos--;
        lcd_put_cur(pos, 0);
    }
    else if (chosenOption == 2)
    {
        pass[itePassLength] = itePass;
        itePass = 32;
        itePassLength++;
        lcd_put_cur(2, itePassLength);
        lcd_send_data(itePass);
        lcd_put_cur(2, itePassLength);
    }
    else if (chosenOption == 3)
    {
        host[iteHostLength] = iteHost;
        iteHost = 32;
        iteHostLength++;
        lcd_put_cur(2, iteHostLength);
        lcd_send_data(iteHost);
        lcd_put_cur(2, iteHostLength);
    }
}

void button_single_click_cb_down(void *arg, void *usr_data)
{
    if (chosenOption == 0)
    {
        if (pos < 2)
            pos++;
        lcd_put_cur(pos, 0);
    }
    else if (chosenOption == 1)
    {
        if (pos < 3 && wList.list[pos][0] != '\0')
            pos++;
        lcd_put_cur(pos, 0);
    }
    else if (chosenOption == 2)
    {
        lcd_send_data(++itePass);
        lcd_put_cur(2, itePassLength);
    }
    else if (chosenOption == 3)
    {
        lcd_send_data(++iteHost);
        lcd_put_cur(2, iteHostLength);
    }
}

void button_single_click_cb_select(void *arg, void *usr_data)
{
    if (chosenOption == 0)
    {
        if (pos == 1)
        {
            wList = wifi_scan_ssids();
            lcd_print_ssids(wList);
            chosenOption = 1; // scanned wifi
        }
        else if (pos == 2 && connectedFlag)
        {
            lcd_ping_input();
            chosenOption = 3;
            for (int i = 0; i < 20; i++)
            { // clear current phost address to reenter address
                host[i] = '\0';
            }
            //inputFun("host address", arg, usr_data);
        }
        else if (pos == 2 && !connectedFlag)
        {
            lcd_ping_not_connected();
            usleep(5000000);
            lcd_print_menu(pos, actPos, connectedFlag);
        }
    }
    else if (chosenOption == 1)
    {
        lcd_input_password(pos, wList);
        chosenOption = 2;
        itePassLength = 0;
        for (int i = 0; i < 20; i++)
        { // clear current password -> necessary if you entered incorrect password and want to try again
            pass[i] = '\0';
        }

        //inputFun("passphrase", arg, usr_data);
    }
    else if (chosenOption == 2)
    {
        ESP_LOGW("Input pass", "%s", pass);
        wifi_connect(wList.list[pos - 1], &pass);
    }
    else if (chosenOption == 3)
    {
        ping_send(host);
    }
}

void app_main(void)
{
    if (init_Driver_i2c() != ESP_OK)
        ESP_LOGW("I2C Driver", "Driver not installed\n");
    button_handle_t gpio_btn_down = iot_button_create(&gpio_btn_cfg_down);
    button_handle_t gpio_btn_up = iot_button_create(&gpio_btn_cfg_up);
    button_handle_t gpio_btn_select = iot_button_create(&gpio_btn_cfg_select);
    iot_button_register_cb(gpio_btn_up, BUTTON_SINGLE_CLICK, button_single_click_cb_up, NULL);
    iot_button_register_cb(gpio_btn_down, BUTTON_SINGLE_CLICK, button_single_click_cb_down, NULL);
    iot_button_register_cb(gpio_btn_select, BUTTON_SINGLE_CLICK, button_single_click_cb_select, NULL);
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    // ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    led_init();
    usleep(2000); // wait 2 ms
    xTaskCreatePinnedToCore(led_set_working_status, "working_status", 4096, NULL, 10, &statusTaskHandle, 1);
    vTaskSuspend(statusTaskHandle);
    usleep(2000);
    led_set_color(0, 31, 0);

    lcd_print_opening_scene();
    usleep(5000000);

    lcd_print_menu(pos, actPos, connectedFlag);
}
