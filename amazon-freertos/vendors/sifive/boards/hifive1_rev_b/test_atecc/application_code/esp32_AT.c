#include <string.h>
#include <time.h>
#include <metal/led.h>
#include <metal/gpio.h>
#include <metal/timer.h>
#include <metal/spi.h>
#include <metal/clock.h>
#include <metal/uart.h> // serial uart
#include <metal/machine.h> // serial uart
#include "debug_serial.h"
#include "drv_spi1.h"
#include "drv_esp32.h"
#include "esp32_AT.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

#if (ESP32_AT_PRINT_LOG_ENABLE == 1)
#define ESP32_AT_PRINT_LOG(x)    debug_puts(x)
#else
#define ESP32_AT_PRINT_LOG(x)
#endif
#define delay_s(s)    vTaskDelay( ( TickType_t )(1000*s) / portTICK_PERIOD_MS )
#define delay_ms(ms)    vTaskDelay( ( TickType_t )ms / portTICK_PERIOD_MS )


enum {
//	MSG_AT = 0,
//	MSG_RESET,
//	MSG_ECHO_OFF,
//	MSG_WIFI_CONNECT,
    MSG_SEND_STRING,
	MSG_RECV_STRING,
};

struct ESP32_ST
{
    int id;
    void *data;
    int len;
} s_esp32;

#if (USE_ALLOCATED_HEAP == 1)
static char *messageReceive;  // dynamic buffer
#else
static char messageReceive[ESP_AT_BUFFER_SIZE]; // static buffer
#endif

static QueueHandle_t xQueueRx, xQueueTx, xQueueReturnValue;
static SemaphoreHandle_t xSemaphore_spi_1 = NULL; // spi1 semaphore
static int esp32_ret = AT_ERROR;

static void prvESP32Task(void* pvParameters);
static void prvSpiRxTask(void* pvParameters);


static void queue_send_cmd(int id, void* data, int len);
static int read_xQueueRx(char* s, int s_len);
static int receive_1line(char* dst, char* src, size_t len);
static int process_message(char* msg, int msg_len);

// Get 1 line in receive buffer
static int receive_1line(char* dst, char* src, size_t len)
{
    size_t i = 0;
    dst[0] = 0;
    for (i = 0; i < len; i++) {
		dst[i] = src[i];
		if(dst[i] == '\n')
		{
			i++;
			break;
		}
		if(dst[i] == 0)
		{
			break;
		}
    }
    dst[i] = 0;
    return i;
}

static int process_message(char* msg, int msg_len)
{
    // Parse AT response to get the result
    int count;
    char *s = msg;
    int len = msg_len;

    do {
    	char tmp[100];
    	count = receive_1line(tmp, s , len);
    	if(count == 0)
    		break;

    	s += count; // move to next line

        // Process each line
        if (memcmp(tmp, "OK", 2) == 0) {
        	ESP32_AT_PRINT_LOG("..");ESP32_AT_PRINT_LOG(tmp);ESP32_AT_PRINT_LOG("..\r\n");
        	return AT_OK;
        } else if (memcmp(tmp, "ERROR", 5) == 0) {
        	ESP32_AT_PRINT_LOG("..");ESP32_AT_PRINT_LOG(tmp);ESP32_AT_PRINT_LOG("..\r\n");
        	return AT_ERROR;
        } else if (memcmp(tmp, "ERR CODE:", 9) == 0) {
        	ESP32_AT_PRINT_LOG("..OK..\n");
        	// ERR CODE:0x010b0000
        	// ERR CODE:0x01030000
        	ESP32_AT_PRINT_LOG("..");ESP32_AT_PRINT_LOG(tmp);ESP32_AT_PRINT_LOG("..\r\n");
        	return AT_ERROR;
        } /*else if (memcmp(tmp, "CONNECT", 2) == 0) {
        	s_is_connect = AT_CONNECT;
        } else if (memcmp(tmp, "DISCONNECT", 5) == 0) {
        	s_is_connect = AT_DISCONNECT;
        } else if (memcmp(tmp, "SEND OK", 5) == 0) {
        	return = AT_OK;
        }*/
        len -= count;
    }
    while (len > 0);
    return AT_NULL;
}

void esp32_create_tasks(void)
{
#if (USE_ALLOCATED_HEAP == 1)
	messageReceive = ( char * ) pvPortMalloc( ESP_AT_BUFFER_SIZE );
	if( messageReceive == NULL )
	{
		ESP32_AT_PRINT_LOG("messageReceive was not created\r\n");
	}
#endif
	xSemaphore_spi_1 = xSemaphoreCreateMutex();
	if ( ( xSemaphore_spi_1 ) != NULL )
	  xSemaphoreGive( ( xSemaphore_spi_1 ) );

    xQueueRx = xQueueCreate( ESP32_RX_QUEUE_LENGTH, sizeof( char ) );
    if( xQueueRx == NULL )
    {
        /* Queue was not created and must not be used. */
        ESP32_AT_PRINT_LOG("xQueueRx was not created\r\n");
    }

    xQueueTx = xQueueCreate( 2, sizeof( s_esp32 ) );
    if( xQueueTx == NULL )
    {
        /* Queue was not created and must not be used. */
    	ESP32_AT_PRINT_LOG("xQueueTx was not created\r\n");
    }

    xQueueReturnValue = xQueueCreate( 1, sizeof( int ) );
    if( xQueueReturnValue == NULL )
    {
        /* Queue was not created and must not be used. */
    	ESP32_AT_PRINT_LOG("xQueueReturnValue was not created\r\n");
    }

    xTaskCreate(prvESP32Task, "prvESP32Task", 120, NULL, 1, NULL);
    xTaskCreate(prvSpiRxTask, "prvSpiRxTask", 120, NULL, 1, NULL);
}


static void prvSpiRxTask( void *pvParameters )
{
	/* Prevent the compiler warning about the unused parameter. */
	( void ) pvParameters;
	char c;
//	int i = 0;
	char *s = messageReceive;
	int len = 0;
	int ret = 0;
	memset(s, 0, sizeof(messageReceive));
	xSemaphoreTake(xSemaphore_spi_1, portMAX_DELAY);
	ESP32_AT_PRINT_LOG("Start ");ESP32_AT_PRINT_LOG(__func__);ESP32_AT_PRINT_LOG("\r\n");
	xSemaphoreGive(xSemaphore_spi_1);
	for( ;; )
	{

		if( xQueueRx != 0 )
		{
			s = messageReceive;
			s[0] = 0;
			len = 0;

			xSemaphoreTake(xSemaphore_spi_1, portMAX_DELAY);
//			ret = drv_esp32_recv(s, &len);
			if(drv_esp32_recv_get_len(&len) != 0)
			{
				ESP32_AT_PRINT_LOG("drv_esp32_recv_get_len error\r\n");
			}
			ret = drv_esp32_recv_get_buf(s, len);
			xSemaphoreGive(xSemaphore_spi_1);

			if( ret != 0)
			{
				// recv error
			}
			else
			{
				if(len > 0) {
					s[len] = 0;
#if (PRINT_ESP32_RESPONSE_ENABLE == 1)
					ESP32_AT_PRINT_LOG("esp32> ");ESP32_AT_PRINT_LOG(s);
#endif
					// send to queue
					while(*s) {
						c = *(s);
						if( xQueueSend( xQueueRx, ( void * ) &c, ( TickType_t )2000 ) != pdPASS )
						{
							/* Failed to post the message. */
							ESP32_AT_PRINT_LOG("xQueueRx is full --> clear\n");
							read_xQueueRx(0,0); // clear queue after 2000 ticks
							xQueueSend( xQueueRx, ( void * ) &c, ( TickType_t )0 ); // re-send queue
						}
						s++;
					}
				}
			}

			memset(s, 0, sizeof(messageReceive));
			delay_ms(100); // sleep 100ms
		}
	}

    vTaskDelete(NULL);
//    vAssertCalled();
}

static void prvESP32Task(void* pvParameters)
{
	struct ESP32_ST esp32;
	/* Remove compiler warning about unused parameter. */
	( void ) pvParameters;
	xSemaphoreTake(xSemaphore_spi_1, portMAX_DELAY);
	ESP32_AT_PRINT_LOG("Start ");ESP32_AT_PRINT_LOG(__func__);ESP32_AT_PRINT_LOG("\r\n");
	xSemaphoreGive(xSemaphore_spi_1);
	for(;;)
	{
		if( xQueueTx != 0 )
		{
			if( xQueueReceive( xQueueTx, ( void * )&esp32 , ( TickType_t ) 100 ) != pdPASS )
			{
			    /* Failed to get the message. */
			} else
			{
				xSemaphoreTake(xSemaphore_spi_1, portMAX_DELAY);
				int id = s_esp32.id;
				switch (id)
				{
				case MSG_SEND_STRING:
				{
//					int len = esp32.len;
					drv_esp32_send((char*)esp32.data);
					break;
				}
				case MSG_RECV_STRING:
				{
					int recv_len = 0;
					recv_len = read_xQueueRx((char*)esp32.data, esp32.len);
					break;
				}
				default:
				{
					break;
				};
				}
				xSemaphoreGive(xSemaphore_spi_1);
			}
		} else
		{
			ESP32_AT_PRINT_LOG("xQueueTx was not created\r\n");
		}

	}

    vTaskDelete(NULL);
//    vAssertCalled();
}

static void queue_send_cmd(int id, void* data, int len)
{
    s_esp32.id = id;
    s_esp32.data = data;
    s_esp32.len = len;
    if( xQueueSend( xQueueTx, ( void * ) &s_esp32,( TickType_t ) portMAX_DELAY ) != pdPASS )
    {
      /* Failed to post the message */
    }
}

static int read_xQueueRx(char* s, int s_len)
{
	char c;
	int i = 0;
	do
	{
		if( xQueueReceive( xQueueRx, ( void * ) &c, ( TickType_t ) 0 ) != pdPASS )
		{
			break; // queue is empty
		}

		if(s)
			s[i] = c;

		if(s_len != 0)
			if(i > s_len)
				break;

		i++;
	}while(1);

	return i;
}

void esp32_send_cmd(const char* cmd, int wait_ms)
{
	queue_send_cmd( MSG_SEND_STRING, (void*)cmd, (int)strlen(cmd) );
	delay_ms(wait_ms + 50);
}

void esp32_recv(char* msg, int len)
{
	queue_send_cmd( MSG_RECV_STRING, (void*)msg, len);
	delay_ms(0 + 50);
}

void esp32_wifi_conn(const char* ssid, const char* pssid, int wait_ms)
{
//	esp32_send_cmd("AT+CWMODE=1\r\n", 100);
//	esp32_send_cmd("AT+CWJAP=\"" WIFI_SSID "\",\"" WIFI_PASS "\"\r\n", 6000);
	esp32_send_cmd("AT+CWJAP=\"", 0);
	esp32_send_cmd(ssid, 0);
	esp32_send_cmd("\",\"", 0);
	esp32_send_cmd(pssid, 0);
	esp32_send_cmd("\"\r\n", wait_ms);
}

void esp32_reset_module(void)
{
	esp32_send_cmd("AT+RST\r\n", 1000);  // reset ESP32-SOLO-1
}

void esp32_echo_off(void)
{
	esp32_send_cmd("ATE0\r\n", 1000);  // echo off
}



