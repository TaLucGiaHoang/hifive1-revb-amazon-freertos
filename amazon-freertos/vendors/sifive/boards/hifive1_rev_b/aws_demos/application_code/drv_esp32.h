/*
 * drv_esp32.h
 *
 *  Created on: Sep 25, 2020
 *      Author: HoangSHC
 */

#ifndef DRV_ESP32_H_
#define DRV_ESP32_H_

#define ESP_AT_BUFFER_SIZE (1024)
void drv_esp32_init(void);
int drv_esp32_send(const char* cmd);
int drv_esp32_recv(char* recvbuf, int* len);
int drv_esp32_recv_get_len(int* len);
int drv_esp32_recv_get_buf(char* recvbuf, int recv_len);
#endif /* DRV_ESP32_H_ */
