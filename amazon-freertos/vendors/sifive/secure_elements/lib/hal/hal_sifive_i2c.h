/**
 * hal_sifive_i2c.h
 *
 *  Created on: Sep 7, 2020
 *      Author: HoangSHC
 */
#ifndef HAL_SAMD21_I2C_START_H_
#define HAL_SAMD21_I2C_START_H_

#include <stdlib.h>

//#ifndef METAL__DRIVERS__SIFIVE_I2C0_H
//struct __metal_driver_sifive_i2c0 {
//    // struct metal_i2c i2c;
//    unsigned int init_done;
//    unsigned int baud_rate;
//    metal_clock_callback pre_rate_change_callback;
//    metal_clock_callback post_rate_change_callback;
//};
//#endif

/** \defgroup hal_ Hardware abstraction layer (hal_)
 *
 * \brief
 * These methods define the hardware abstraction layer for communicating with a CryptoAuth device
 *
   @{ */


#define MAX_I2C_BUSES   1   // Sifive HiFive1 Rev B has 1 I2C

/** \brief this is the hal_data for ATCA HAL for Atmel START SERCOM
 */
typedef struct atcaI2Cmaster
{
    // struct __metal_driver_sifive_i2c0 i2c_master_instance;
    // uint32_t               sercom_core_freq;
    int                    ref_ct;  // reference counter
    // for conveniences during interface release phase
    int bus_index;
} ATCAI2CMaster_t;

void change_i2c_speed(ATCAIface iface, uint32_t speed);

/** @} */

#endif /* HAL_SAMD21_I2C_START_H_ */
