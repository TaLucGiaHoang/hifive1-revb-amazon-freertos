/*
 * FreeRTOS Wi-Fi V1.0.0
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file iot_wifi.c
 * @brief Wi-Fi Interface.
 */

/* Socket and Wi-Fi interface includes. */
#include "FreeRTOS.h"
#include "iot_wifi.h"

/* Wi-Fi configuration includes. */
#include "aws_wifi_config.h"

/* on-board Wifi of SiFive HiFive1 Rev B */
#include "esp32_AT.h"
#include "debug_serial.h"
/*-----------------------------------------------------------*/


static int esp32_ret = AT_ERROR;
static int s_is_connect = AT_DISCONNECT;
static char s_msg[50];
static char s_ret[20];
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
        	esp32_ret = AT_OK;
        } else if (memcmp(tmp, "ERROR", 5) == 0) {
        	esp32_ret = AT_ERROR;
        } else if (memcmp(tmp, "ERR CODE:", 9) == 0) {
        	// ERR CODE:0x010b0000
        	// ERR CODE:0x01030000
        	esp32_ret = AT_ERROR;
        } else if (memcmp(tmp, "CONNECT", 2) == 0) {
        	s_is_connect = AT_CONNECT;
        } else if (memcmp(tmp, "DISCONNECT", 5) == 0) {
        	s_is_connect = AT_DISCONNECT;
        } else if (memcmp(tmp, "SEND OK", 5) == 0) {
        	esp32_ret = AT_OK;
        } else if (memcmp(tmp, "+IPD,4:", 7) == 0)
        {
//        	get_IPD(tmp);
        }
        len -= count;
    }
    while (len > 0);
    return 0;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_On( void )
{
    /* FIX ME. */
	debug_puts("WIFI_On\r\n");
	/* Restart esp32 module */
	esp32_reset_module();  // reset esp32
	esp32_echo_off();  // switch echo off
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Off( void )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_ConnectAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    /* FIX ME. */
	 /* @return @ref eWiFiSuccess if connection is successful, failure code otherwise.
	 *
	 * **Example**
	 * @code
	 * WIFINetworkParams_t xNetworkParams;
	 * WIFIReturnCode_t xWifiStatus;
	 * xNetworkParams.pcSSID = "SSID String";
	 * xNetworkParams.ucSSIDLength = SSIDLen;
	 * xNetworkParams.pcPassword = "Password String";
	 * xNetworkParams.ucPasswordLength = PassLength;
	 * xNetworkParams.xSecurity = eWiFiSecurityWPA2;
	 */
	const char* ssid = pxNetworkParams->pcSSID;
	const char* pssid = pxNetworkParams->pcPassword;
	/* Connect to Wifi */
	memset(s_msg, 0, sizeof(s_msg)); // clear buffer
	esp32_recv(0,0); // clear esp32 queue message

//	esp32_wifi_conn(ssid, pssid, 6000); // connect to wifi
	esp32_send_cmd("AT+CWMODE=1\r\n", 100); // set Station Mode (1)
	esp32_send_cmd("AT+CWJAP=\"", 0);
	esp32_send_cmd(ssid, 0);
	esp32_send_cmd("\",\"", 0);
	esp32_send_cmd(pssid, 0);
	esp32_send_cmd("\"\r\n", 6000);

	esp32_recv(s_msg, sizeof(s_msg)); // get esp32 queue message and store to buffer
	esp32_ret = AT_ERROR; // reset esp32 return
	process_message(s_msg, sizeof(s_msg)); // process buffer
	configASSERT( esp32_ret == AT_OK );  // exit if disconnect to wifi

//    return eWiFiFailure;
	return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Disconnect( void )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiFailure;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Reset( void )
{
    /* FIX ME. */
	s_is_connect = AT_DISCONNECT;  // reset global variable
	debug_puts("WIFI_Reset\r\n");
	esp32_send_cmd("AT+RST\r\n", 1000);  // reset ESP32-SOLO-1
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Scan( WIFIScanResult_t * pxBuffer,
                            uint8_t ucNumNetworks )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts("\r\n");
	esp32_send_cmd("AT+CWLAP\r\n", 5000); // list all avaiable acces points
    return eWiFiSuccess;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetMode( WIFIDeviceMode_t xDeviceMode )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts("\r\n");

//  0: Null mode, WiFi disabled
//	1: Station mode
//	2: SoftAP mode
//	3: SoftAP+Station mode

	memset(s_ret, 0, sizeof(s_msg)); // clear buffer

	/* Set WiFi mode */
	switch(xDeviceMode)
	{
	case eWiFiModeStation:
		esp32_send_cmd("AT+CWMODE=1\r\n", 100); // Station mode
		return eWiFiSuccess;
		break;
	case eWiFiModeAP:
		esp32_send_cmd("AT+CWMODE=2\r\n", 100); // SoftAP mode
		return eWiFiSuccess;
		break;
//	case eWiFiModeP2P:
//		esp32_send_cmd("AT+CWMODE=3\r\n", 100); // SoftAP+Station mode
//		break;
//	case eWiFiModeNotSupported:
	default:
		esp32_send_cmd("AT+CWMODE=0\r\n", 100); // Null mode, WiFi disabled
		break;
	}

    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMode( WIFIDeviceMode_t * pxDeviceMode )
{
    /* FIX ME. */
	// <mode>:
	//  0: Null mode, WiFi disabled
	//	1: Station mode
	//	2: SoftAP mode
	//	3: SoftAP+Station mode
	/*
	 * Response:
	 *
	 *   +CWMODE:<mode>
	 *   OK
	 */
	debug_puts(__func__);debug_puts(" not implemented\r\n");

    if (pxDeviceMode == NULL) {
        return eWiFiFailure;
    }

	char msg[10] = "+CWMODE:x";
	esp32_recv(0,0); // clear esp32 queue message
	esp32_send_cmd("AT+CWMODE=?\r\n", 200);
	esp32_recv(msg, sizeof(msg)); // get esp32 queue message and store to buffer

	char c_mode = msg[9];

	/* Set WiFi mode */
	switch(c_mode)
	{
	case '1':
		*pxDeviceMode = eWiFiModeStation;
		return eWiFiSuccess;
		break;
	case '2':
		*pxDeviceMode = eWiFiModeAP;
		return eWiFiSuccess;
		break;
	case '3':
		*pxDeviceMode = eWiFiModeNotSupported;
		return eWiFiNotSupported;
		break;
	case '0':
		*pxDeviceMode = eWiFiModeNotSupported;
		return eWiFiNotSupported;
		break;
	default:
		return eWiFiFailure;
		break;
	}
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkAdd( const WIFINetworkProfile_t * const pxNetworkProfile,
                                  uint16_t * pusIndex )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkGet( WIFINetworkProfile_t * pxNetworkProfile,
                                  uint16_t usIndex )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_NetworkDelete( uint16_t usIndex )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_Ping( uint8_t * pucIPAddr,
                            uint16_t usCount,
                            uint32_t ulIntervalMS )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetIP( uint8_t * pucIPAddr )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");

    WIFIReturnCode_t xRetVal = eWiFiSuccess;
    int sRetCode;

    configASSERT( pucIPAddr != NULL );

    unsigned long ulDestinationIP = 0;
    unsigned long ulSubMask = 0;
    unsigned long ulDefGateway = 0;
    unsigned long ulDns = 0;

    /* Get IP address. */
//    sRetCode = ( int16_t ) Network_IF_IpConfigGet( &ulDestinationIP,
//                                                   &ulSubMask,
//                                                   &ulDefGateway,
//                                                   &ulDns );
    if( sRetCode != 0 )
    {
        xRetVal = eWiFiFailure;
    }
    else
    {
//        /*fill the return buffer.*/
//        *( pucIPAddr ) = SL_IPV4_BYTE( ulDestinationIP, 3 );
//        *( pucIPAddr + 1 ) = SL_IPV4_BYTE( ulDestinationIP, 2 );
//        *( pucIPAddr + 2 ) = SL_IPV4_BYTE( ulDestinationIP, 1 );
//        *( pucIPAddr + 3 ) = SL_IPV4_BYTE( ulDestinationIP, 0 );
    }

    return xRetVal;

    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetMAC( uint8_t * pucMac )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetHostIP( char * pcHost,
                                 uint8_t * pucIPAddr )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StartAP( void )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_StopAP( void )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_ConfigureAP( const WIFINetworkParams_t * const pxNetworkParams )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_SetPMMode( WIFIPMMode_t xPMModeType,
                                 const void * pvOptionValue )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_GetPMMode( WIFIPMMode_t * pxPMModeType,
                                 void * pvOptionValue )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/

BaseType_t WIFI_IsConnected(void)
{
	/* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
	return pdFALSE;
}
/*-----------------------------------------------------------*/

WIFIReturnCode_t WIFI_RegisterNetworkStateChangeEventCallback( IotNetworkStateChangeEventCallback_t xCallback )
{
    /* FIX ME. */
	debug_puts(__func__);debug_puts(" not implemented\r\n");
    return eWiFiNotSupported;
}
/*-----------------------------------------------------------*/
