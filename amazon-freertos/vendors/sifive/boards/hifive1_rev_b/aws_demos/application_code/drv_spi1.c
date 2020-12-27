/*
 * drv_spi1.c
 *
 *  Created on: Sep 25, 2020
 *      Author: HoangSHC
 */

#include "drv_spi1.h"

//#define USE_FREERTOS
#ifdef USE_FREERTOS
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "timers.h"
static int wait_rx_fifo(unsigned long timeout, unsigned long *rxdata)
{
	TickType_t endwait = xTaskGetTickCount() + timeout;

	while ((*rxdata = spi1_rxdata) & METAL_SPI_RXDATA_EMPTY) {
		if (xTaskGetTickCount() > endwait) {
			/* If timeout, deassert the CS */
			spi1_csmode &= ~(METAL_SPI_CSMODE_MASK);
			return -1;
		}
	}

	return 0;
}
#else
static int wait_rx_fifo(unsigned long timeout, unsigned long *rxdata)
{
    /* Declare time_t variables to break out of infinite while loop */
    // time_t endwait;
	volatile unsigned long endwait = timeout;

	while ((*rxdata = spi1_rxdata) & METAL_SPI_RXDATA_EMPTY) {
		//__asm__ volatile ("nop");
		endwait--;
		if (endwait < 0) {
			/* If timeout, deassert the CS */
			spi1_csmode &= ~(METAL_SPI_CSMODE_MASK);
            return -1;
		}

	}
	return 0;
}
#endif

void driver_spi1_init(void)
{
	spi1_txmark = 1;
	spi1_rxmark = 0;
	spi1_ie = 3;   // SPI1 interrupts enable
	gpio0_input_en |= (1 << WF_INT); // Set GPIO10 to input for Wifi interrupt (WF_INT)
}


int driver_spi1_transfer(size_t len, char *tx_buf, char *rx_buf)
{
    int rc = 0;
    size_t i = 0;

    /* Hold the chip select line for all len transferred */
    spi1_csmode &= ~(METAL_SPI_CSMODE_MASK);
    spi1_csmode |= METAL_SPI_CSMODE_HOLD;
    unsigned long rxdata;

    for (; i < len; i++) {
        /* Master send bytes to the slave */

        /* Wait for TXFIFO to not be full */
        while (spi1_txdata & METAL_SPI_TXDATA_FULL);

        /* Transfer byte by modifying the least significant byte in the TXDATA register */
        if (tx_buf) {
        	spi1_txdata = tx_buf[i];
        } else {
            /* Transfer a 0 byte if the sending buffer is NULL */
        	spi1_txdata = 0;
        }

        /* Master receives bytes from the RX FIFO */

        /* Wait for RXFIFO to not be empty, but break the nested loops if timeout
         * this timeout method  needs refining, preferably taking into account
         * the device specs */
        if(wait_rx_fifo(100, &rxdata) != 0) // METAL_SPI_RXDATA_TIMEOUT
        {
            /* If timeout, return error code 1 immediately */
            return 1;
        }

        /* Only store the dequeued byte if the receive_buffer is not NULL */
        if (rx_buf) {
            rx_buf[i] = (char) (rxdata & METAL_SPI_TXRXDATA_MASK);
        }
    }

    /* On the last byte, set CSMODE to auto so that the chip select transitions back to high
     * The reason that CS pin is not deasserted after transmitting out the byte buffer is timing.
     * The code on the host side likely executes faster than the ability of FIFO to send out bytes.
     * After the host iterates through the array, fifo is likely not cleared yet. If host deasserts
     * the CS pin immediately, the following bytes in the output FIFO will not be sent consecutively.
     * There needs to be a better way to handle this. */
    spi1_csmode &= ~(METAL_SPI_CSMODE_MASK);

    return 0;
}
