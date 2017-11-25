/*
 * I2CMaster.h
 *
 *  Created on: 10.01.2014
 *      Author: ´Ilhan
 */
#include "clock.h"
#include <stdint.h>
#ifndef I2CMASTER_H_
#define I2CMASTER_H_

void I2CInit( void );
void serial_communication(void);
void VCNL4000init(void);
void ushort2Ascii(unsigned short val, unsigned char *str, unsigned char cnt);
uint16_t I2CWrite( uint8_t sladdr , uint8_t *data , uint16_t n );
uint16_t I2CRead( uint8_t sladdr , uint8_t *data , uint16_t n );

#endif /* I2CMASTER_H_ */
