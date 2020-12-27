/*
 * esp32_AT.h
 *
 *  Created on: Nov 4, 2019
 *      Author: HoangSHC
 */

#ifndef ESP32_AT_H_
#define ESP32_AT_H_

//#define WIFI_SSID "SHCVN02"
//#define WIFI_PASS "khongduoc"

#define USE_ALLOCATED_HEAP  (1)

#define ESP_AT_BUFFER_SIZE    (1024)
#define ESP32_RX_QUEUE_LENGTH (1024)

#define ESP32_AT_PRINT_LOG_ENABLE (1)
#define PRINT_ESP32_RESPONSE_ENABLE (1)

enum {
	AT_OK = 0,
	AT_ERROR,
	AT_CONNECT,
	AT_DISCONNECT,
	AT_SEND_OK,
	AT_NULL,
};

void esp32_create_tasks(void);
void esp32_reset_module(void);
void esp32_echo_off(void);
void esp32_wifi_conn(const char* ssid, const char* pssid, int wait_ms);
void esp32_send_cmd(const char* cmd, int wait_ms);
void esp32_recv(char* msg, int len);


#endif /* ESP32_AT_H_ */
