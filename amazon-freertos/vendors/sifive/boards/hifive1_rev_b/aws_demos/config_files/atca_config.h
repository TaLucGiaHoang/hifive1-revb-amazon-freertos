/**
 * \file
 * \brief Cryptoauthlib Configuration Defines
 *
 * \copyright (c) 2015-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip software
 * and any derivatives exclusively with Microchip products. It is your
 * responsibility to comply with third party license terms applicable to your
 * use of third party software (including open source software) that may
 * accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT,
 * SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE
 * OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF
 * MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE
 * FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL
 * LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED
 * THE AMOUNT OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR
 * THIS SOFTWARE.
 */

#ifndef _ATCA_CONFIG_H
#define _ATCA_CONFIG_H

/** Use I2C */
#define ATCA_HAL_I2C

/** Use the following address for ECC devices */
#define ATCA_I2C_ECC_ADDRESS    0x36  // ATECC608A-TFLXTLS address, SiFive HiFive1 RevB supports 7-bit only
  // used by pkcs11_slot_config() in vendors\microchip\secure_elements\lib\pkcs11\pkcs11_slot.c
  // used by pkcs11_token_init() in vendors\microchip\secure_elements\lib\pkcs11\pkcs11_token.c

/** Define if cryptoauthlib is to use the maximum execution time method */
#define ATCA_NO_POLL

/**
 * ATCA_NO_HEAP can be used to remove the use of malloc/free from the main
 * library. This can be helpful for smaller MCUs that don't have a heap
 * implemented. If just using the basic API, then there shouldn't be any code
 * changes required. The lower-level API will no longer use the new/delete
 * functions and the init/release functions should be used directly.
 */
//#define ATCA_NO_HEAP // comment out for pre-processor define

/** Use RTOS timers (i.e. delays that yield) */
#define ATCA_USE_RTOS_TIMER


#endif
