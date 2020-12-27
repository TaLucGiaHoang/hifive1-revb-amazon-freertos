/**
 * hal_sifive_i2c.c
 *
 *  Created on: Sep 7, 2020
 *      Author: HoangSHC
 */
 #include <string.h>
#include <stdio.h>
#include <metal/machine.h>  // sifive
#include "drv_i2c0.h"  // sifive i2c driver
#include "debug_serial.h"
#include "atca_hal.h"
#include "atca_device.h"
#include "hal_sifive_i2c.h"
//#include "atca_execution.h"
//#include "atca_start_config.h"
//#include "atca_start_iface.h"

#if defined(DEBUG_SERIAL_H_)
#include <stdio.h>
#define DEBUG_LOG(x) debug_puts(x)
#else
#define DEBUG_LOG(x)
#endif


/**
 * \defgroup hal_ Hardware abstraction layer (hal_)
 *
 * \brief
 * These methods define the hardware abstraction layer for communicating with a CryptoAuth device
 *
   @{ */

/**
 * \brief
 * Logical to physical bus mapping structure
 */
static ATCAI2CMaster_t i2c_hal_data[MAX_I2C_BUSES];   // map logical, 0-based bus number to index


/**
 * \brief discover i2c buses available for this hardware
 * this maintains a list of logical to physical bus mappings freeing the application
 * of the a-priori knowledge
 * \param[in] i2c_buses - an array of logical bus numbers
 * \param[in] max_buses - maximum number of buses the app wants to attempt to discover
 * \return ATCA_SUCCESS
 */

ATCA_STATUS hal_i2c_discover_buses(int i2c_buses[], int max_buses)
{
	DEBUG_LOG("hal_i2c_discover_buses - unimplemented\r\n");
    return ATCA_UNIMPLEMENTED;
}

/**
 * \brief discover any CryptoAuth devices on a given logical bus number
 * \param[in]  bus_num  logical bus number on which to look for CryptoAuth devices
 * \param[out] cfg     pointer to head of an array of interface config structures which get filled in by this method
 * \param[out] found   number of devices found on this bus
 * \return ATCA_SUCCESS
 */

ATCA_STATUS hal_i2c_discover_devices(int bus_num, ATCAIfaceCfg cfg[], int *found)
{
	DEBUG_LOG("hal_i2c_discover_devices - unimplemented\r\n");
    return ATCA_UNIMPLEMENTED;
}

/**
 * \brief hal_i2c_init manages requests to initialize a physical interface.  it manages use counts so when an interface
 * has released the physical layer, it will disable the interface for some other use.
 * You can have multiple ATCAIFace instances using the same bus, and you can have multiple ATCAIFace instances on
 * multiple i2c buses, so hal_i2c_init manages these things and ATCAIFace is abstracted from the physical details.
 */

/** \brief initialize an I2C interface using given config
 * \param[in] hal - opaque ptr to HAL data
 * \param[in] cfg - interface configuration
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_init(void *hal, ATCAIfaceCfg *cfg)
{
    DEBUG_LOG("hal_i2c_init\r\n");
    if (cfg->atcai2c.bus >= MAX_I2C_BUSES)
    {
    	DEBUG_LOG("cfg->atcai2c.bus invalid value\r\n");
        return ATCA_COMM_FAIL;
    }
    ATCAI2CMaster_t* data = &i2c_hal_data[0]; //&i2c_hal_data[cfg->atcai2c.bus];

    if (data->ref_ct <= 0)
    {
//        /* Bus isn't being used, enable it */
//        switch (cfg->atcai2c.bus)
//        {
//        case 0:
//            /* set I2C baudrate and enable I2C module 0 */
//            driver_i2c0_init(cfg->atcai2c.baud, 1);  // 1: Master mode, 100000 Hz or 400000 Hz
//            break;
//        default:
//            return ATCA_COMM_FAIL;
//        }

        driver_i2c0_init(cfg->atcai2c.baud, 1);  // 100kHz, Master mode
        // store this for use during the release phase
        data->bus_index = cfg->atcai2c.bus;
        // buses are shared, this is the first instance
        data->ref_ct = 1;
    }
    else
    {
        // Bus is already is use, increment reference counter
        data->ref_ct++;
    }

    ((ATCAHAL_t*)hal)->hal_data = data;

    return ATCA_SUCCESS;
}

/**
 * \brief HAL implementation of I2C post init
 * \param[in] iface  instance
 * \return ATCA_SUCCESS
 */
ATCA_STATUS hal_i2c_post_init(ATCAIface iface)
{
    DEBUG_LOG("hal_i2c_post_init - unimplemented\r\n");
    return ATCA_SUCCESS;
}

/** \brief HAL implementation of I2C send over START
 * \param[in] iface     instance
 * \param[in] txdata    pointer to space to bytes to send
 * \param[in] txlength  number of bytes to send
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_send(ATCAIface iface, uint8_t *txdata, int txlength)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);

    // for this implementation of I2C with CryptoAuth chips, txdata is assumed to have ATCAPacket format

    // other device types that don't require i/o tokens on the front end of a command need a different hal_i2c_send and wire it up instead of this one
    // this covers devices such as ATSHA204A and ATECCx08A that require a word address value pre-pended to the packet
    // txdata[0] is using _reserved byte of the ATCAPacket
    txdata[0] = 0x03;   // insert the Word Address Value, Command token
    txlength++;         // account for word address value byte.

    if (driver_i2c0_write(cfg->atcai2c.slave_address, txlength, (unsigned char*)txdata, METAL_I2C_STOP_ENABLE) != 0)
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/**
 * \brief HAL implementation of I2C receive function for START I2C
 * \param[in]    iface     Device to interact with.
 * \param[out]   rxdata    Data received will be returned here.
 * \param[inout] rxlength  As input, the size of the rxdata buffer.
 *                         As output, the number of bytes received.
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */
ATCA_STATUS hal_i2c_receive(ATCAIface iface, uint8_t *rxdata, uint16_t *rxlength)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    int count;
    int retries = cfg->rx_retries;
    int status = !ATCA_SUCCESS;
    uint16_t rxdata_max_size = *rxlength;

    *rxlength = 0;
    if (rxdata_max_size < 1)
    {
        return ATCA_SMALL_BUFFER;
    }

    /* Get number of received bytes */
    while (retries-- > 0 && status != ATCA_SUCCESS)
    {
        if (driver_i2c0_read(cfg->atcai2c.slave_address, 1, (unsigned char*)rxdata, METAL_I2C_STOP_ENABLE) != 0)
        {
            status = ATCA_COMM_FAIL;
        }
        else
        {
            status = ATCA_SUCCESS;
        }
    }

    if (status != ATCA_SUCCESS)
    {
        return ATCA_COMM_FAIL;
    }
    if (rxdata[0] < ATCA_RSP_SIZE_MIN)
    {
        return ATCA_INVALID_SIZE;
    }
    if (rxdata[0] > rxdata_max_size)
    {
        return ATCA_SMALL_BUFFER;
    }

    //Update receive length with first byte received and set to read rest of the data
    count  = rxdata[0] - 1;

    /* Receive data */
    if (driver_i2c0_read(cfg->atcai2c.slave_address, count, (unsigned char*)&rxdata[1], METAL_I2C_STOP_ENABLE) != 0)
    {
        return ATCA_COMM_FAIL;
    }

    *rxlength = rxdata[0];

    return ATCA_SUCCESS;
}

/**
 * \brief method to change the bus speed of I2C
 * \param[in] iface  interface on which to change bus speed
 * \param[in] speed  baud rate (typically 100000 or 400000)
 */

void change_i2c_speed(ATCAIface iface, uint32_t speed)
{
    // set I2C baudrate
    driver_i2c0_set_baud_rate(speed);
}

/**
 * \brief wake up CryptoAuth device using I2C bus
 * \param[in] iface  interface to logical device to wakeup
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_wake(ATCAIface iface)
{
	DEBUG_LOG("hal_i2c_wake\r\n");
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    uint32_t bdrt = cfg->atcai2c.baud;
    uint8_t data[4];

    if (bdrt != 100000)    // if not already at 100KHz, change it
    {
        change_i2c_speed(iface, 100000);
    }

    // Send 0x00 address as wake pulse
    // Dummy Write
    // This command will always return NACK.
    // So, the return code is being ignored.
    driver_i2c0_write(0x00, 0, (unsigned char*)NULL, METAL_I2C_STOP_ENABLE);

    atca_delay_ms(3); // wait tWHI + tWLO which is configured based on device type and configuration structure
    //atca_delay_us(cfg->wake_delay);

    // if necessary, revert baud rate to what came in.
    if (bdrt != 100000)
    {
        change_i2c_speed(iface, bdrt);
    }

    // receive the wake up response
    driver_i2c0_read(cfg->atcai2c.slave_address, 4, (unsigned char*)data, METAL_I2C_STOP_ENABLE);

    return hal_check_wake(data, 4);
}

/**
 * \brief idle CryptoAuth device using I2C bus
 * \param[in] iface  interface to logical device to idle
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_idle(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    uint8_t data[4];

    data[0] = 0x02;  // idle word address value
    
    if (driver_i2c0_write(cfg->atcai2c.slave_address, 1, (unsigned char*)data, METAL_I2C_STOP_ENABLE) != 0)
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/**
 * \brief sleep CryptoAuth device using I2C bus
 * \param[in] iface  interface to logical device to sleep
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_sleep(ATCAIface iface)
{
    ATCAIfaceCfg *cfg = atgetifacecfg(iface);
    uint8_t data[4];

    data[0] = 0x01;  // sleep word address value

    if (driver_i2c0_write(cfg->atcai2c.slave_address, 1, (unsigned char*)data, METAL_I2C_STOP_ENABLE) != 0)
    {
        return ATCA_COMM_FAIL;
    }

    return ATCA_SUCCESS;
}

/**
 * \brief manages reference count on given bus and releases resource if no more refences exist
 * \param[in] hal_data - opaque pointer to hal data structure - known only to the HAL implementation
 * \return ATCA_SUCCESS on success, otherwise an error code.
 */

ATCA_STATUS hal_i2c_release(void *hal_data)
{
    ATCAI2CMaster_t *hal = (ATCAI2CMaster_t*)hal_data;

    //if the use count for this bus has gone to 0 references, disable it.  protect against an unbracketed release
    if (hal && --(hal->ref_ct) <= 0)
    {
        hal->ref_ct = 0;
    }

    return ATCA_SUCCESS;
}

/** @} */
