/*
 * wifi.c
 *
 *  Created on: Sep 24, 2020
 *      Author: HoangSHC
 */

#include <string.h>
#include "debug_serial.h"
#include "esp32_AT.h"
#include "wifi.h"

#define WIFI_PRINT_LOG_ENABLE
#ifdef WIFI_PRINT_LOG_ENABLE
#define PRINT_LOG(x)    debug_puts(x)
#else
#define PRINT_LOG(x)
#endif

//static int receive_1line(char* dst, char* src, size_t len);
static int process_message(char* msg, int msg_len);

// Parse AT response to get the result
static int process_message(char* s, int len)
{
    int count;

    if(!s)
    {
    	return AT_NULL;
    }

    do {
    	char *tmp = s;
    	count = 0;
    	while(count < len)
    	{
    		debug_putc(*(tmp+count));
    		/* exit loop if NULL character */
    		if(*(tmp+count) == 0)
    		{
    			break;
    		}

    		/* count '\n' and exit loop */
    		if(*(tmp+count) == '\n' )
    		{
    			count++;
    			break;
    		}

    		count++;
    	}

    	if(count == 0)
    		break;

    	s += count; // move to next line

        // Process each line
        if (memcmp(tmp, "OK", 2) == 0) {
        	return AT_OK;
        } else if (memcmp(tmp, "ERROR", 5) == 0) {
        	return AT_ERROR;
        } else if (memcmp(tmp, "ERR CODE:", 9) == 0) {
        	// ERR CODE:0x010b0000
        	// ERR CODE:0x01030000
        	return AT_ERROR;
        }
        len -= count;
    }
    while (len > 0);
    return AT_NULL;
}

int AT_cmd(const char* cmd, int wait_ms, char* out_msg, size_t out_msg_len)
{
	/* Clear queue before sending AT command */
	if(out_msg && out_msg_len > 0)
	{
		esp32_recv(0,0); // clear esp32 queue message
		//memset(out_msg, 0, out_msg_len); // clear buffer
	}

	/* Send AT command */
	esp32_send_cmd(cmd, wait_ms);

	/* Get esp32 return */
	if(out_msg && out_msg_len > 0)
	{
		memset(out_msg, 0, out_msg_len); // clear buffer
		esp32_recv(out_msg, out_msg_len); // get esp32 queue message and store to buffer
		return process_message(out_msg, out_msg_len); // process buffer
	} else
	{
		return AT_NULL;
	}
}

// reset ESP32-SOLO-1
int AT_reset(void)
{
	char msg[50] = {0};

	memset(msg, 0, sizeof(msg)); // clear buffer
	esp32_recv(0,0); // clear esp32 queue message
	esp32_send_cmd("AT+RST\r\n", 1000);  // send AT command
	esp32_recv(msg, sizeof(msg)); // get esp32 queue message and store to buffer
	return process_message(msg, sizeof(msg)); // process buffer

//	return AT_cmd("AT+RST\r\n", 1000, msg, sizeof(msg));
}

int AT_echo_off(void)
{
	char msg[50] = {0};

//	memset(msg, 0, sizeof(msg)); // clear buffer
//	esp32_recv(0,0); // clear esp32 queue message
//	esp32_send_cmd("ATE0\r\n", 100); // send AT command
//	esp32_recv(msg, sizeof(msg)); // get esp32 queue message and store to buffer
//	return process_message(msg, sizeof(msg)); // process buffer

	return AT_cmd("ATE0\r\n", 100, msg, sizeof(msg));
}

int AT_at(void)
{
	char msg[50] = {0};

//	memset(msg, 0, sizeof(msg)); // clear buffer
//	esp32_recv(0,0); // clear esp32 queue message
//	esp32_send_cmd("AT\r\n", 100); // send AT command
//	esp32_recv(msg, sizeof(msg)); // get esp32 queue message and store to buffer
//	return process_message(msg, sizeof(msg)); // process buffer

	return AT_cmd("AT\r\n", 100, msg, sizeof(msg));
}

int AT_wifi_mode_station(void)
{
	char msg[50] = {0};

//	memset(msg, 0, sizeof(msg)); // clear buffer
//	esp32_recv(0,0); // clear esp32 queue message
//	esp32_send_cmd("AT+CWMODE=1\r\n", 100); // send AT command
//	esp32_recv(msg, sizeof(msg)); // get esp32 queue message and store to buffer
//	return process_message(msg, sizeof(msg)); // process buffer

	return AT_cmd("AT+CWMODE=1\r\n", 100, msg, sizeof(msg));
}

int AT_wifi_conn(const char* ssid, const char* pssid, int wait_ms, char* out_msg, size_t out_msg_len)
{
	if( !out_msg || !(out_msg_len > 0) )
		return AT_NULL;

	memset(out_msg, 0, sizeof(out_msg)); // clear buffer
	esp32_recv(0,0); // clear esp32 queue message

	// send AT command
	esp32_send_cmd("AT+CWJAP=\"", 0);
	esp32_send_cmd(ssid, 0);
	esp32_send_cmd("\",\"", 0);
	esp32_send_cmd(pssid, 0);
	esp32_send_cmd("\"\r\n", wait_ms);

	esp32_recv(out_msg, out_msg_len); // get esp32 queue message and store to buffer
	return process_message(out_msg, out_msg_len); // process buffer
}
