/*
 * clock.h
 *
 *  Created on: 16.01.2014
 *      Author: ´Ilhan
 */
/*
 * clock.h
 *
 *  Created on: 15.12.2013
 *      Author: Andreas
 */

#ifndef CLOCK_H_
#define CLOCK_H_

#define DCO_MHZ 1 // choice of 1, 8, 12 or 16 MHz
#define LAUNCHPAD // Use this definition if the uC sits on the Launchpad




#if DCO_MHZ == 1
 #define CALBC1 CALBC1_1MHZ
 #define CALDCO CALDCO_1MHZ
#elif DCO_MHZ == 8
 #define CALBC1 CALBC1_8MHZ
 #define CALDCO CALDCO_8MHZ
#elif DCO_MHZ == 12
 #define CALBC1 CALBC1_12MHZ
 #define CALDCO CALDCO_12MHZ
#elif DCO_MHZ == 16
 #define CALBC1 CALBC1_16MHZ
 #define CALDCO CALDCO_16MHZ
#else
 #error "Bad Value for DCO_MHZ"
#endif


// Pre calculated values for using maximum SPICLK frequency < 10 MHz
#if DCO_MHZ == 1 // SPICLK 1 MHz
 #define SPIBR0 (0x01)
 #define SPIBR1 (0x00)
#elif DCO_MHZ == 8 // SPICLK 8 MHz
 #define SPIBR0 (0x01)
 #define SPIBR1 (0x00)
#elif DCO_MHZ == 12 // SPICLK 6 MHz
 #define SPIBR0 (0x02)
 #define SPIBR1 (0x00)
#elif DCO_MHZ == 16 // SPICLK 8 MHz
 #define SPIBR0 (0x02) // Divide SMLK by 2 to get 8 MHz (0x02)
 #define SPIBR1 (0x00) // UCB0BR0 + UCB0BR1 * 256 forms prescaler value
#endif

// Pre calculated values for UART with 115200 Baud (From MSP430x2xx Family User's Guide, 15.3.13)
#ifdef LAUNCHPAD // Launchpads retarded UART->USB only supports 9600 Baud
#if DCO_MHZ == 1
 #define UARTBR0 (0x68) // 1 MHz/9600 = 104.2
 #define UARTBR1 (0x00) // UCB0BR0 + UCB0BR1 * 256 forms prescaler value
 #define UARTMCTL (UCBRS0) // Modulation UCBRSx = 1
#elif DCO_MHZ == 8
 #define UARTBR0 (0x41) // 8 MHz/9600 = 833.3
 #define UARTBR1 (0x03)
 #define UARTMCTL (UCBRS1) // Modulation UCBRSx = 2
#elif DCO_MHZ == 12
 #define UARTBR0 (0xE2) // 12 MHz/9600 = 1250
 #define UARTBR1 (0x04)
 #define UARTMCTL (0) // Modulation UCBRSx = 0
#elif DCO_MHZ == 16
 #define UARTBR0 (0x82) // 16 MHz/9600 = 1666.7
 #define UARTBR1 (0x06)
 #define UARTMCTL (UCBRS2 | UCBRS1) // Modulation UCBRSx = 6
#endif
#else
#if DCO_MHZ == 1
 #define UARTBR0 (0x08) // 1 MHz/115200 = ~8.7
 #define UARTBR1 (0x00)
 #define UARTMCTL (UCBRS2 | UCBRS1) // Modulation UCBRSx = 6
#elif DCO_MHZ == 8
 #define UARTBR0 (0x45) // 8 MHz/115200 = ~69.4
 #define UARTBR1 (0x00)
 #define UARTMCTL (UCBRS2) // Modulation UCBRSx = 4
#elif DCO_MHZ == 12
 #define UARTBR0 (0x68) // 12 MHz/115200 = ~104.2
 #define UARTBR1 (0x00)
 #define UARTMCTL (UCBRS0) // Modulation UCBRSx = 1
#elif DCO_MHZ == 16
 #define UARTBR0 (0x8A) // 16 MHz/115200 = ~138.9
 #define UARTBR1 (0x00)
 #define UARTMCTL (UCBRS2 | UCBRS1 | UCBRS0) // Modulation UCBRSx = 7
#endif
#endif


#endif /* CLOCK_H_ */
