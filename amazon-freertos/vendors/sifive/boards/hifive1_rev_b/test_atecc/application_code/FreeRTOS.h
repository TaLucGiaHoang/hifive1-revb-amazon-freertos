#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H
////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h> /* READ COMMENT ABOVE. */


#define vPortFree( x ) free( x )
#define pvPortMalloc( x )  malloc( x )
//void* pvPortMalloc(size_t);

/* Legacy type definitions. */
#define portCHAR		char
#define portFLOAT		float
#define portDOUBLE		double
#define portLONG		long
#define portSHORT		short

#define portBASE_TYPE	int
typedef portBASE_TYPE BaseType_t;

typedef unsigned int UBaseType_t;

#define configPRINTF   printf
///////////////////
#endif /* INC_FREERTOS_H */
