#include <esp_err.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <string.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include <unistd.h>
#include "driver/i2c.h"
#include <iot_button.h>
#include "driver/gpio.h"
#include <led_strip.h>
#include <esp_system.h>
#include <esp_event.h>
#include <esp_wifi.h>

#define I2C_MASTER_SDA_IO 1
#define I2C_MASTER_SCL_IO 2
#define I2C_MASTER_FREQ_HZ 40000
#define SLAVE_ADDRESS_LCD 0x27
#define I2C_NUM I2C_NUM_0
#define LED_GPIO_NUMBER 48
#define BUTTON_GPIO_SELECT_PIN 39
#define BUTTON_GPIO_UP_PIN 38
#define BUTTON_GPIO_DOWN_PIN 35
#define OPTION_WIFI "Connect to Wifi"
#define OPTION_PING "Send Ping"

struct wifiList{
    uint8_t list[3][33];
};

esp_err_t init_Driver_i2c();

/***
 * @brief initialize lcd
*/
void lcd_init (void);
/***
 * @brief send command to lcd display
 * 
 * @param cmd char that repesents command
*/
void lcd_send_cmd (char cmd);
/***
 * @brief send data to lcd display
 * 
 * @param data char that repesents data
*/
void lcd_send_data (char data);
/***
 * @brief send data as string to lcd display
 * 
 * @param str string that represents data
*/
void lcd_send_string (char *str);
/***
 * @brief put cursor at the entered position row (0, 1, 2 or 3), col (0-19)
 * 
 * @param row number of row where cursor will be, values are 0,1,2,3
 * @param col number of column where cursor will be, values are 0-19
*/
void lcd_put_cur(uint8_t row, uint8_t col);
/***
 * @brief clear cld display
*/
void lcd_clear (void);
/***
 * @brief opening scene with current version
*/
void lcd_print_opening_scene(void);
/***
 * @brief used to move cursor as selected option
 * 
 * @param pos chosen option position (menu iterator)
 * @param actPos first displayed option (not used for now or ever)
 * @param ifConnected flag that tells if ESP is connected to wifi
*/
void lcd_print_menu(uint8_t pos, uint8_t actPos,uint8_t ifConnected);
/***
 * @brief used input password for selected ssid
 * 
 * @param pos chosen option position (menu iterator), in this case seleced ssid position in wifi list
 * @param wList list of scanned ssids
*/
void lcd_input_password(uint8_t pos,struct wifiList wList); 
/***
 * @brief print scanned wifi lists
 * 
 * @param wList list of scanned ssids
 */
void lcd_print_ssids(struct wifiList wList);
/**
 * @brief prints information about not connected device
 * 
 */
void lcd_ping_not_connected(void);
/**
 * @brief print input page for host address
 * 
 */
void lcd_ping_input(void);
/**
 * @brief used to get list of detected networks
 * 
 * @return list of scanned ssids in form of struct wifiList
 */
struct wifiList wifi_scan_ssids(void);
/**
 * @brief used to connect to wifi with defined ssid and password
 * 
 * @param ssid ssid in uint8_t array
 * @param pass password in uint8_t array
 */
void wifi_connect(uint8_t *ssid, uint8_t* pass);
/**
 * @brief initialization of built-in led
 */
void led_init(void);
/**
 * @brief turn off built-in led
 */
void led_clear(void);
/**
 * @brief set color for built-in led
 * 
 * @param red red color value
 * @param green green color value
 * @param blue blue color value
 */
void led_set_color(uint8_t red,  uint8_t green, uint8_t blue);
/**
 * @brief set working status (blinking blue light) used as task
 */
void led_set_working_status(void);
/**
 * @brief check if ESP is connected to wifi
 */
void led_connect_status(void);
/**
 * @brief send ping to host address
 * 
 * @param host host address
 */
void ping_send(uint8_t *host);

