#include "interface.h"
static const char *TAG = "i2c";

i2c_config_t conf = {
	.mode = I2C_MODE_MASTER,
	.sda_io_num = I2C_MASTER_SDA_IO,
	.scl_io_num = I2C_MASTER_SCL_IO,
	.sda_pullup_en = GPIO_PULLUP_ENABLE,
	.scl_pullup_en = GPIO_PULLUP_ENABLE,
	.master.clk_speed = I2C_MASTER_FREQ_HZ,
};

esp_err_t init_Driver_i2c()
{
	i2c_param_config(I2C_NUM, &conf);
	return i2c_driver_install(I2C_NUM, conf.mode, 0, 0, 0);
};

void lcd_send_cmd(char cmd)
{
	uint8_t data_t[4];
	data_t[0] = (cmd & 0xf0) | 0x0C;		// en=1, rs=0
	data_t[1] = (cmd & 0xf0) | 0x08;		// en=0, rs=0
	data_t[2] = ((cmd << 4) & 0xf0) | 0x0C; // en=1, rs=0
	data_t[3] = ((cmd << 4) & 0xf0) | 0x08; // en=0, rs=0
	if (i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000) != 0)
		ESP_LOGW(TAG, "Error in sending command");
	usleep(100);
}

void lcd_send_data(char data)
{
	uint8_t data_t[4];
	data_t[0] = (data & 0xf0) | 0x0D;		 // en=1, rs=0
	data_t[1] = (data & 0xf0) | 0x09;		 // en=0, rs=0
	data_t[2] = ((data << 4) & 0xf0) | 0x0D; // en=1, rs=0
	data_t[3] = ((data << 4) & 0xf0) | 0x09; // en=0, rs=0
	if (i2c_master_write_to_device(I2C_NUM, SLAVE_ADDRESS_LCD, data_t, 4, 1000) != 0)
		ESP_LOGW(TAG, "Error in sending data");
	usleep(100);
}

void lcd_clear(void)
{
	lcd_send_cmd(0x01);
	usleep(2500);
}

void lcd_put_cur(uint8_t row, uint8_t col)
{
	switch (row)
	{
	case 0:
		col |= 0x80;
		break;
	case 1:
		col |= 0xC0;
		break;
	case 2:
		col += 0x94;
		break;
	case 3:
		col += 0xD4;
		break;
	}

	lcd_send_cmd(col);
}

void lcd_init(void)
{
	ESP_LOGI(TAG, "LCD initialization!");
	usleep(50000);
	for (int j = 0; j < 3; j++)
	{
		lcd_send_cmd(0b00110000);
		usleep(2500);
	}
	lcd_send_cmd(0b00100000);
	usleep(2000);
	lcd_send_cmd(0b00101000); // Display Line ON, 4 bit mode, 2 rows, 5x7 chars
	usleep(2000);
	lcd_send_cmd(0b00001101); // Display ON, Cursor ON, Blink ON
	usleep(2000);
	lcd_send_cmd(0b00000001); // clear display
	usleep(2000);
	lcd_send_cmd(0b00000100); // entry mode
	usleep(2000);
}

void lcd_send_string(char *str)
{
	while (*str)
		lcd_send_data(*str++);
}

void lcd_print_opening_scene()
{
	lcd_init();
	lcd_clear();
	lcd_put_cur(0, 6);
	lcd_send_string("Buzek OS");
	lcd_put_cur(1, 6);
	lcd_send_string("Ver. 0.3");
	lcd_put_cur(2, 6);
	lcd_send_string("By Bu zek");
}

void lcd_print_menu(uint8_t pos, uint8_t actPos, uint8_t ifConnected)
{
	lcd_clear();
	if (ifConnected)
	{
		lcd_put_cur(0, 2);
		lcd_send_string("Menu - Connected");
	}
	else
	{
		lcd_put_cur(0, 0);
		lcd_send_string("Menu - Disconnected");
	}

	lcd_put_cur(1, 1);
	lcd_send_string(OPTION_WIFI);
	lcd_put_cur(2, 1);
	lcd_send_string(OPTION_PING);
	lcd_put_cur(pos, 0);
}

void lcd_print_ssids(struct wifiList wList)
{
	int i = 0;
	lcd_clear();
	lcd_put_cur(0, 3);
	lcd_send_string("Scanned ssids");
	lcd_put_cur(1, 1);
	while (i < 19 && wList.list[0][i] != '\0')
	{
		lcd_send_data((char *)wList.list[0][i]);
		i++;
	}
	i = 0;
	lcd_put_cur(2, 1);
	while (i < 19 && wList.list[1][i] != '\0')
	{
		lcd_send_data((char *)wList.list[1][i]);
		i++;
	}
	i = 0;
	lcd_put_cur(3, 1);
	while (i < 19 && wList.list[2][i] != '\0')
	{
		lcd_send_data((char *)wList.list[2][i]);
		i++;
	}
	lcd_put_cur(1, 0);
}

void lcd_input_password(uint8_t pos, struct wifiList wList)
{
	int i = 0;
	lcd_clear();
	lcd_put_cur(0, 3);
	lcd_send_string("Enter password");
	lcd_put_cur(1, 0);
	while (i < 20 && wList.list[pos - 1][i] != '\0')
	{
		lcd_send_data((char *)wList.list[pos - 1][i]);
		i++;
	}
	i = 0;
	lcd_put_cur(2, 0);
	lcd_send_data(32);
	lcd_put_cur(2, 0);
}

void lcd_ping_not_connected(void)
{
	lcd_clear();
	lcd_put_cur(1, 0);
	lcd_send_string("Wifi not connected");
}

void lcd_ping_input(void)
{
	lcd_clear();
	lcd_put_cur(1, 0);
	lcd_send_string("Input host address");
	lcd_put_cur(2, 0);
	lcd_send_data(32);
	lcd_put_cur(2, 0);
}

void lcd_ping_result(uint32_t transmitted, uint32_t received, uint32_t loss, uint32_t total_time_ms, ip_addr_t target_addr)
{
	char temp[20] = {' '};
	lcd_clear();
	lcd_put_cur(0, 0);
	lcd_send_string("Ping session ended");
	lcd_put_cur(1, 3);
	sprintf(temp, "%s", inet_ntoa(*ip_2_ip4(&target_addr)));
	lcd_send_string(temp);
	for (int i = 0; i < 20; i++)
		temp[i] = ' ';
	lcd_put_cur(2, 0);
	sprintf(temp, "%ld/%ld received", received, transmitted);
	lcd_send_string(temp);
	for (int i = 0; i < 20; i++)
		temp[i] = ' ';
	lcd_put_cur(3, 0);
	sprintf(temp,"%ld%% pl, time %ldms", loss, total_time_ms);
	lcd_send_string(temp);
}

void lcd_dns_error(struct addrinfo *res, int err){
	lcd_clear();
	lcd_put_cur(1, 1);
	lcd_send_string("DNS lookup failed");
	char temp[20] = {' '};
	sprintf(temp,"err=%d res=%p", err ,res);
	lcd_put_cur(2, 1);
	lcd_send_string(temp);
}