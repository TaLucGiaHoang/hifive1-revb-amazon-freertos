/*
 * debug_serial.h
 *
 *  Created on: Nov 4, 2019
 *      Author: HoangSHC
 */

#ifndef DEBUG_SERIAL_H_
#define DEBUG_SERIAL_H_

void debug_init(void);
void debug_putc(int c);
int debug_puts(const char *s);
extern int printf2(const char *fmt, ...);
#endif /* DEBUG_SERIAL_H_ */
