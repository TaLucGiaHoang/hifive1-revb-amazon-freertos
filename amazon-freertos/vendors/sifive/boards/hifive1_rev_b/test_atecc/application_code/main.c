#include <stdlib.h>
#include <stdio.h>
/* Freedom metal driver includes. */
#include <metal/cpu.h>
#include <metal/led.h>
#include "drv_i2c0.h"

#include "aws_dev_mode_key_provisioning.h"

/* The name of the devices for xApplicationDNSQueryHook. */
#define mainDEVICE_NICK_NAME				"hifive1_rev_b" /* FIX ME.*/

/* Assert call defined for debug builds. */
extern void vAssertCalled( const char * pcFile,
                           uint32_t ulLine );
#define configASSERT( x )    if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ )
///**
// * @brief Connects to Wi-Fi.
// */
//static void prvWifiConnect( void );

/**
 * @brief Initializes the board.
 */
static void prvMiscInitialization( void );


static void vToggleLED( void );
#include "hal/atca_hal.h"

/*-----------------------------------------------------------*/
#include "atca_device.h"
#include "atca_iface.h"
#include "hal/atca_hal.h"
static ATCADevice _gDevice_test = NULL;
static struct atca_command g_atcab_command_test;
static struct atca_iface g_atcab_iface_test;
static struct atca_device g_atcab_device_test;

void testATCA( void )
{
	ATCA_STATUS status = ATCA_GEN_FAIL;

	ATCAIfaceCfg interface_config = {
			.iface_type = 0x0,
			.devtype = ATECC608A,
			.atcai2c.slave_address = ATCA_I2C_ECC_ADDRESS,
			.atcai2c.bus = 0,
			.atcai2c.baud = 100000,//400000,
			.wake_delay = 1500, // us
			.rx_retries = 20,
			.cfg_data = 0x0,
	};
	ATCAIfaceCfg *cfg = &interface_config;

	g_atcab_iface_test.mType = ATCA_I2C_IFACE;
	g_atcab_iface_test.mIfaceCFG = &interface_config;

	cfg->devtype = ATECC608A;

	g_atcab_command_test.dt = ATECC608A;
	g_atcab_command_test.clock_divider = 0;

    g_atcab_device_test.mCommands = &g_atcab_command_test;
    g_atcab_device_test.mIface = &g_atcab_iface_test;
    status = initATCADevice(cfg, &g_atcab_device_test);
    if (status != ATCA_SUCCESS)
    {
    	printf2("initATCADevice returns %x\r\n", status);
    }
    // replace initATCADevice
//    ATCAHAL_t hal; // still not initialize
//    status = hal_i2c_init(&hal, &interface_config);
//    status = hal_i2c_wake(&g_atcab_iface_test);
//    if (status != ATCA_SUCCESS)
//    {
//    	printf2("hal_i2c_wake returns %x\r\n", status);
//    }
    ////////////////////

    _gDevice_test = &g_atcab_device_test;


    // atcab_read_bytes_zone
    // atcab_read_zone
    // atca_execute_command
    // awake
    if (cfg->devtype == ATECC608A)
    {
    	ATCADevice ca_dev = &g_atcab_device_test;
        if ((status = atwake(ca_dev->mIface)) != ATCA_SUCCESS)
        {
        	printf2("atwake returns %x\r\n", status);
        }
        _gDevice_test->mCommands->clock_divider &= ATCA_CHIPMODE_CLOCK_DIV_MASK;
    }

//    {
//    	hal_i2c_wake2(&g_atcab_iface_test, 1); // work
//    	hal_i2c_wake2(&g_atcab_iface_test, 2); // work
//    	hal_i2c_wake2(&g_atcab_iface_test, 3); // work
//    	hal_i2c_wake2(&g_atcab_iface_test, 4); // work
//    	hal_i2c_wake2(&g_atcab_iface_test, 5); // work
//    }
}
/*-----------------------------------------------------------*/
///////////////////////////////
/**
 * @brief Application runtime entry point.
 */
int main( void )
{
	prvMiscInitialization();
	printf("main.c\r\n", __LINE__);
//////////
    driver_i2c0_init(100000UL, 1); // i2c 100khz, master mode

    void* p = malloc(8);
    if(p  == NULL) {
    	printf("p %p NULL\r\n", p);
    } else {
    	printf("p %p\r\n", p);
    	free(p);
        if(p  == NULL) {
        	printf("p %p freed\r\n", p);
        }
    }
    vDevModeKeyProvisioning();
    printf("main.c DONE\r\n", __LINE__);
//    while(1)
//    {
//    	vToggleLED();
//    	atca_delay_ms(1000);
//    }

//    testATCA();
//////////////

    return 0;
}
/*-----------------------------------------------------------*/

/* Index to first HART (there is only one). */
#define mainHART_0 		0

/* Registers used to initialise the PLIC. */
#define mainPLIC_PENDING_0 ( * ( ( volatile uint32_t * ) 0x0C001000UL ) )
#define mainPLIC_PENDING_1 ( * ( ( volatile uint32_t * ) 0x0C001004UL ) )
#define mainPLIC_ENABLE_0  ( * ( ( volatile uint32_t * ) 0x0C002000UL ) )
#define mainPLIC_ENABLE_1  ( * ( ( volatile uint32_t * ) 0x0C002004UL ) )

#define I2C_BAUDRATE (100000UL) // 100kHz
#define METAL_I2C_SLAVE (0)
#define METAL_I2C_MASTER (1)

/*
 * Used by the Freedom Metal drivers.
 */
static struct metal_led *pxRedLED = NULL;
static struct metal_led *pxGreenLED = NULL;
static struct metal_led *pxBlueLED = NULL;

static void prvMiscInitialization( void )
{
    /* FIX ME: Perform any hardware initializations, that don't require the RTOS to be 
     * running, here.
     */
    struct metal_cpu *pxCPU;
    struct metal_interrupt *pxInterruptController;

    /* Initialise the red LED. */
    pxRedLED = metal_led_get_rgb( "LD0", "red" );
    configASSERT( pxRedLED );
    metal_led_enable( pxRedLED );
     metal_led_on( pxRedLED );

     /* Initialise the green LED. */
     pxGreenLED = metal_led_get_rgb( "LD0", "green" );
     configASSERT( pxGreenLED );
     metal_led_enable( pxGreenLED );
     metal_led_on( pxGreenLED );
    
    /* Initialise the blue LED. */
     pxBlueLED = metal_led_get_rgb( "LD0", "blue" );
     configASSERT( pxBlueLED );
     metal_led_enable( pxBlueLED );
     metal_led_on( pxBlueLED );

    /* Initialise the interrupt controller. */
    pxCPU = metal_cpu_get( mainHART_0 );
    configASSERT( pxCPU );
    pxInterruptController = metal_cpu_interrupt_controller( pxCPU );
    configASSERT( pxInterruptController );
    metal_interrupt_init( pxInterruptController );

//    /* WiFi is automatically initialized */
//    drv_esp32_init();

    /* Set all interrupt enable bits to 0. */
    mainPLIC_ENABLE_0 = 0UL;
    mainPLIC_ENABLE_1 = 0UL;

    /* Clear all pending interrupts. */
    mainPLIC_PENDING_0 = 0UL;
    mainPLIC_PENDING_1 = 0UL;
}

/*-----------------------------------------------------------*/
void vToggleLED( void )
{
	metal_led_toggle( pxBlueLED );
}


/**
 * @brief User defined assertion call. This function is plugged into configASSERT.
 * See FreeRTOSConfig.h to define configASSERT to something different.
 */
void vAssertCalled(const char * pcFile,
	uint32_t ulLine)
{
//    const uint32_t ulLongSleep = 1000UL;
//    volatile uint32_t ulBlockVariable = 0UL;
    volatile char * pcFileName = (volatile char *)pcFile;
    volatile uint32_t ulLineNumber = ulLine;
     static struct metal_led *pxRedLED = NULL;
     volatile uint32_t ul;
     const uint32_t ulNullLoopDelay = 0x1ffffUL;

    (void)pcFileName;
    (void)ulLineNumber;

    printf("vAssertCalled %s, %ld\n", pcFile, (long)ulLine);

//    /* Setting ulBlockVariable to a non-zero value in the debugger will allow
//    * this function to be exited. */
//    taskDISABLE_INTERRUPTS();
    {
        /* Flash the red LED to indicate that assert was hit - interrupts are off
        here to prevent any further tick interrupts or context switches, so the
        delay is implemented as a crude loop instead of a peripheral timer. */
//        while (ulBlockVariable == 0UL)
//        {
//            vTaskDelay( pdMS_TO_TICKS( ulLongSleep ) );
//            metal_led_toggle( pxRedLED );
//        }

        for( ul = 0; ul < ulNullLoopDelay; ul++ )
        {
            __asm volatile( "nop" );
        }
        metal_led_toggle( pxRedLED );
    }
//    taskENABLE_INTERRUPTS();
}

/**
 * @brief Implements libc calloc semantics using the FreeRTOS heap
 */
void * pvCalloc( size_t xNumElements,
				 size_t xSize )
{
//	void * pvNew = pvPortMalloc( xNumElements * xSize );
//
//	if( NULL != pvNew )
//	{
//		memset( pvNew, 0, xNumElements * xSize );
//	}
//
//	return pvNew;

	return calloc(xNumElements, xSize);
}

_write(int file, const void *ptr, size_t len)
{
  const char *bptr = ptr;
  for (size_t i = 0; i < len; ++i)
	  debug_putc(bptr[i]);
  return 0;
}

//void* pvPortMalloc(size_t size)
//{
//	printf("pvPortMalloc %d\r\n", size);
//	return malloc(size);
//}
