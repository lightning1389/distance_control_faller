#include "I2CMaster.h"
#include <msp430g2553.h>

#define UCA0RXD (BIT1)
#define UCA0TXD (BIT2)

#define slave_adress 0x13
#define COMMAND_0 0x80  // starts measurments, relays data ready info
#define PRODUCT_ID 0x81  // product ID/revision ID, should read 0x11
#define IR_CURRENT 0x83  // sets IR current in steps of 10mA 0-200mA
#define AMBIENT_PARAMETER 0x84  // Configures ambient light measures
#define AMBIENT_RESULT_MSB 0x85  // high byte of ambient light measure
#define AMBIENT_RESULT_LSB 0x86  // low byte of ambient light measure
#define PROXIMITY_RESULT_MSB 0x87  // High byte of proximity measure
#define PROXIMITY_RESULT_LSB 0x88  // low byte of proximity measure
#define PROXIMITY_FREQ 0x89  // Proximity IR test signal freq, 0-3
#define PROXIMITY_MOD 0x8A  // proximity modulator timing

volatile uint16_t    Ack;

static const uint8_t slave_adresse=0x13;

volatile uint16_t I2CStop;

uint8_t *I2CRxBuffer, *I2CTxBuffer;
uint8_t transmitregister[9];
uint8_t receiveregister[3];
uint16_t messwerte;
uint16_t read_zaehler=0;
uint16_t I2CNumBytes =2;

int n;
unsigned short val;
unsigned char* str;
unsigned char cnt=0;
unsigned char ascii_feld[5];


void main (void)
{
	WDTCTL = WDTPW + WDTHOLD;           // Stop Watchdog Timer
	I2CInit();							//Initializierung I2C Master
	serial_communication();
	transmitregister[0]=COMMAND_0;		//Commandregister des VCNL 4000
	transmitregister[1]=0x08;
	transmitregister[2]=PROXIMITY_RESULT_MSB;
	transmitregister[3]=PROXIMITY_RESULT_LSB;
	transmitregister[4]=IR_CURRENT;
	transmitregister[5]=20;
	transmitregister[6]=PROXIMITY_FREQ;
	transmitregister[7]=PROXIMITY_MOD;
	transmitregister[8]=129;


	__enable_interrupt();				//Einschalten des Interrupts
	VCNL4000init();

	while(1)
	{
		messwerte=0;

		I2CTxBuffer=transmitregister;			//Startadresse des Sendebuffers in TxBuffer
		I2CRxBuffer=receiveregister;			//Startadresse des Empfangsbuffers in RxBuffer
		I2CNumBytes =2;							//Anzahl an Bytes die übersendet werden sollen
		I2CWrite(slave_adresse, I2CTxBuffer ,I2CNumBytes);	//Funktion für Schreiben -> 0x80 command Register & 0x08 für eine Proximity Messung
		while(UCB0STAT & UCBBUSY);				//Überprüfen ob die Kommunikation auch physikalisch abgeschlossen ist
		I2CTxBuffer=&transmitregister[2];
		I2CNumBytes=1;							//Anzahl Bytes die übertragen werden sollen
		I2CWrite(slave_adresse, I2CTxBuffer ,I2CNumBytes);	//Funktion zum schreiben
		while(UCB0STAT & UCBBUSY);					//Überprüfen ob die Kommunikation auch physikalisch abgeschlossen ist
		//_delay_cycles(10000ul);					//Start der Wartezeit, die der Sensor benötigt für die Messung

		I2CNumBytes=1;
		I2CRead(slave_adresse, I2CRxBuffer, I2CNumBytes);	//Funktion zum Lesen
		while(UCB0STAT & UCBBUSY);						//Physikalische beendet?

		messwerte=(receiveregister[0]<<8);				//Einschreiben des MSB in ein 16 Bit Wort
		I2CNumBytes=1;									//Anzahl Bytes zu lesen
		I2CRead(slave_adresse,I2CRxBuffer, I2CNumBytes);
		while(UCB0STAT & UCBBUSY);
		messwerte |=receiveregister[1];//LSB in das 16 Bit Wort
		cnt=5;
		str=&ascii_feld[5];
		ushort2Ascii(messwerte,&ascii_feld[5],cnt);

		n=0;
		for(n=0; n<=5; n++)
		{

			while(!(IFG2 & UCA0TXIFG));

			UCA0TXBUF=ascii_feld[n];
			IFG2 &=~UCA0TXIFG;
			//_delay_cycles(10000ul);
			while(UCA0STAT & UCBUSY);
		}
		UCA0TXBUF=10;
		IFG2 &=~UCA0TXIFG;
		while(UCA0STAT & UCBUSY);
		UCA0TXBUF=13;
		IFG2 &=~UCA0TXIFG;
		while(UCA0STAT & UCBUSY);
	}
}
void I2CInit( void )
{
    P1SEL |= BIT6 + BIT7;                    // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;
    UCB0CTL1 |= UCSWRST;                    // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;    // 7-bit addressing, single-master environment, I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB0BR0 = 16;                    // fSCL = SMCLK/UCB0BR1
    UCB0BR1 = 0;
    UCB0I2CIE |= UCNACKIE;                    // Enable not-acknowledge interrupt
    UCB0I2CSA=slave_adress;
    UCB0CTL1 &= ~UCSWRST;                    // Clear SW reset, resume operation
    IE2 |= UCB0TXIE + UCB0RXIE;                // Enable TX&RX interrupts

}

void ushort2Ascii(unsigned short val, unsigned char *str, unsigned char cnt)
{;
	do
	{
		cnt--;
		*--str = (val % 10) | '0';
		val /= 10;
	} while( cnt && val );
	for( ; cnt; cnt-- ) { *--str = ' '; }
}

void serial_communication(void)
{
	P1DIR |= UCA0TXD;					 //Output TX
	P1DIR &=~UCA0RXD;					//Input RX
	P1SEL |=UCA0RXD  | UCA0TXD; 		//Pin Selection
    P1SEL2 |= UCA0RXD | UCA0TXD;

    UCA0CTL1 |= UCSSEL_2; // Derive clock from SMCLK
    UCA0BR0 = UARTBR0; // Use clock divider from clock.h
    UCA0BR1 = UARTBR1;
    UCA0MCTL = UARTMCTL; // Use modulation from clock.h
    UCA0CTL1 &= ~UCSWRST; // Initialize USCI state machine
    IE2 |= UCA0RXIE; // Enable USCI_A0 RX interrupt
}

uint16_t I2CWrite( uint8_t sladdr , uint8_t *data , uint16_t n )
{

    Ack = 1;                            // Return value
    I2CTxBuffer = data;                    // TX array start address
    I2CNumBytes = n;                      // Update counter
    //UCB0I2CSA = sladdr;                  // Slave address (Right justified, bits6-0)
    UCB0CTL1 |= UCTR + UCTXSTT;            // Send I2C start condition, I2C TX mode
    while( UCB0CTL1 & UCTXSTP );        // I2C stop condition sent?
    return Ack;
}

uint16_t I2CRead( uint8_t sladdr , uint8_t *data , uint16_t n )
{
    Ack = 1;                        // Return value
    I2CRxBuffer = data;                // Start of RX buffer
   // UCB0I2CSA = sladdr;                // Slave address (Right justified, bits6-0)
    UCB0CTL1 &= ~UCTR;                // I2C RX mode
    read_zaehler++;

    if( n == 1 )
    {
        I2CNumBytes = 0;                // Update counter
        __disable_interrupt();
        UCB0CTL1 |= UCTXSTT;            // Send I2C start condition, I2C RX mode
        while( UCB0CTL1 & UCTXSTT );    // I2C start condition sent?
        UCB0CTL1 |= UCTXSTP;            // Send I2C stop condition
        I2CStop = 1;                    // I2C stop condition sent
        __enable_interrupt();
    }
    else if( n > 1 )
    {
        I2CStop = 0;                    // I2C stop condition not sent yet
        I2CNumBytes = n - 2;            // Update counter
        UCB0CTL1 |= UCTXSTT;            // Send I2C start condition
    }
    while( UCB0CTL1 & UCTXSTP );        // I2C stop condition sent?
    return Ack;
}

#ifdef    I2C_PING
uint16_t I2CPing( uint8_t sladdr )
{
    //
    UCB0I2CSA = sladdr;                      // Slave address (Right justified, bits6-0)
    //
    __disable_interrupt();
    UCB0CTL1 |= UCTR + UCTXSTT + UCTXSTP;    // I2C start condition, I2C TX mode, I2C stop condition
    while( UCB0CTL1 & UCTXSTP );            // I2C stop condition sent?
    Ack = !(UCB0STAT & UCNACKIFG);            // I2C start condition akd'd or not?
    __enable_interrupt();
    //
    return Ack;
}
#endif    /* !I2C_PING */

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{
     if( IFG2 & UCB0RXIFG )        // RX mode
     {
        if( I2CNumBytes == 0 )
        {
            // I2CStop is used just  to make sure that we leave LPM0 at the right time and not
            // before
            if( I2CStop )
            {
                //_low_power_mode_off_on_exit( );        // Exit LPM0
            }
            else
            {
                UCB0CTL1 |= UCTXSTP;                // I2C stop condition
                I2CStop = 1;                        // I2C stop condition sent
            }
        }
        else
        {
            I2CNumBytes--;                            // Decrement counter
        }
        *I2CRxBuffer++ = UCB0RXBUF;                    // Read RX data. This automatically clears UCB0RXIFG
     }
     else                        // TX mode
     {
        if( I2CNumBytes )                            // Check counter
        {
            UCB0TXBUF = *I2CTxBuffer++;             // Load TX buffer. This automatically clears UCB0TXIFG
            I2CNumBytes--;                            // Decrement counter
        }
        else
        {
            UCB0CTL1 |= UCTXSTP;                    // I2C stop condition
            IFG2 &= ~UCB0TXIFG;                     // Clear USCI_B0 TX int flag
        }
     }
}

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
    // Not-acknowledge interrupt. This flag is set when an acknowledge is expected
    // but is not received. UCNACKIFG is automatically cleared when a START condition
    // is received.
    if( UCB0STAT & UCNACKIFG )
    {
        UCB0CTL1 |= UCTXSTP;            // I2C stop condition
        Ack = 0;                        // Return value
        UCB0STAT &= ~UCNACKIFG;            // Clear interrupt flag
    }
    // Arbitration-lost. When UCALIFG is set the UCMST bit is cleared and the I2C
    // controller becomes a slave. This can only happen in a multimaster environment

    // Start condition detected interrupt. UCSTTIFG only used in slave mode.

    // Stop condition detected interrupt. UCSTPIFG only used in slave mode.
}

void VCNL4000init(void)
{
	/*Stromwert für die IR-LED. Wird mit 10 multipliziert 20mA x 10 = 200mA*/

	I2CTxBuffer=&transmitregister[4];
	I2CNumBytes =2;							//Anzahl an Bytes die übersendet werden sollen
	I2CWrite(slave_adresse, I2CTxBuffer ,I2CNumBytes);	//Funktion für Schreiben -> 0x80 command Register & 0x08 für eine Proximity Messung
	while(UCB0STAT & UCBBUSY);				//Überprüfen ob die Kommunikation auch physikalisch abgeschlossen ist

	I2CTxBuffer=&transmitregister[7];
	I2CNumBytes =1;							//Anzahl an Bytes die übersendet werden sollen
	I2CWrite(slave_adresse, I2CTxBuffer ,I2CNumBytes);	//Funktion für Schreiben -> 0x80 command Register & 0x08 für eine Proximity Messung
	while(UCB0STAT & UCBBUSY);				//Überprüfen ob die Kommunikation auch physikalisch abgeschlossen ist


}
