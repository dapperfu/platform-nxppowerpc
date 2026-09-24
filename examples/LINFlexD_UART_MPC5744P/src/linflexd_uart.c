/*
 * linflexd_uart.c
 *
 *  Created on: Mar 2, 2016
 *      Author: B55457
 */



#include "linflexd_uart.h"

unsigned char UARTFirstTransmitFlag_0;
unsigned char UARTFirstTransmitFlag_1;
/*****************************************************************************/
/*
** Baud Rate = LINCLK / (16 x LFDIV)
** LINCLK = BR x (16 x LFDIV)
** LINCLK / (BR x 16) = LFDIV
**
** LFDIV = Mantissa.Fraction.
** Mantissa = LINIBRR
** Fraction = LINFBRR / 16
**
** Baud Rate = LINCLK / (16 x LINIBRR.(LINFBRR / 16))
** LINIBRR.(LINFBRR / 16) = LINCLK / (BR x 16)
** LINIBRR = Mantissa[LINCLK / (BR x 16)]
** Remainder =  LINFBRR / 16
** LINFBRR = Remainder * 16
** The Remainder is < 1, So how does the Remainder work during a divide?
** May be best to use a table?
**
** For Refernce & Quick Tests
** LINFLEX_x.LINIBRR.R = 416;                   // 9600 at 64MHz
** LINFLEX_x.LINFBRR.R = 11;
**
** LINFLEX_x.LINIBRR.R = 781;                   // 9600 at 120MHz
** LINFLEX_x.LINFBRR.R = 4;
*/
/*****************************************************************************/

void initLINFlexD_1 ( unsigned int MegaHertz, unsigned int BaudRate ) {
  unsigned int Fraction;
  unsigned int Integer;

  LINFlexD_1.LINCR1.B.INIT = 1;     /* Enter Initialization Mode */
  LINFlexD_1.LINCR1.B.SLEEP = 0;    /* Exit Sleep Mode */
  LINFlexD_1.UARTCR.B.UART = 1;     /* UART Enable- Req'd before UART config.*/
  LINFlexD_1.UARTCR.R = 0x0033;     /* UART Ena, 1 byte tx, no parity, 8 data*/
  LINFlexD_1.UARTSR.B.SZF = 1;      /* CHANGE THIS LINE   Clear the Zero status bit */
  LINFlexD_1.UARTSR.B.DRFRFE = 1;   /* CHANGE THIS LINE  Clear DRFRFE flag - W1C */

  BaudRate  = (MegaHertz * 1000000) / BaudRate;
  Integer   = BaudRate / 16;
  Fraction  = BaudRate - (Integer * 16);

  LINFlexD_1.LINIBRR.R = Integer;
  LINFlexD_1.LINFBRR.R = Fraction;

  LINFlexD_1.LINCR1.B.INIT = 0;     /* Exit Initialization Mode */

  UARTFirstTransmitFlag_1 = 1;        /* Indicate no Tx has taken place yet */

  /* Configure LINFlexD_1 TxD Pin. */
  SIUL2.MSCR[PD9].B.SSS = 2; //Pad PD9: Set to LINFlex_1 TxD. Must choose this option because F14 leads to LIN PHY of motherboard
  SIUL2.MSCR[PD9].B.OBE = 1; //Enable output buffer
  SIUL2.MSCR[PD9].B.SRC = 3; //Full drive-strength without slew rate control

  /* Configure LINFlexD_1 RxD Pin. */
  SIUL2.MSCR[PD12].B.IBE = 1; //Pad PD12: Enable input buffer
  SIUL2.IMCR[166].B.SSS = 0b0010; //Connect LINFlexD_2 signal to PD12
}
/*****************************************************************************/

char message_1[] = {"\n\rThis is the DEVKIT-MPC5744P UART test.  If you see this in your PC terminal, test was successful.\n\r"};

void testLINFlexD_1( void )  {     /* Display message to terminal */

	int i, size;
	  size = sizeof(message_1);
	  /* Send to LINFlex_1. */
	  for (i = 0; i < size; i++) {
	    txLINFlexD_1(message_1[i]); //Send from LINFlex_1
	  }
}
/*****************************************************************************/

unsigned char rxLINFlexD_1() {
  while (LINFlexD_1.UARTSR.B.DRFRFE == 0); /* Wait for dta reception complete*/
  LINFlexD_1.UARTSR.R &= UART_DRFRFE;      /* Clear data receptoin flag W1C */
  return( LINFlexD_1.BDRM.B.DATA4 );       /* Read byte of Data */
}
/*****************************************************************************/

void txLINFlexD_1( unsigned char Data ) {
  if( UARTFirstTransmitFlag_1 )   {         /* 1st byte transmit after init: */
    UARTFirstTransmitFlag_1 = 0;            /* Clear variable */
  }
  else {                                  /* Normal tranmit (not 1st time): */
    while (LINFlexD_1.UARTSR.B.DTFTFF == 0); /* Wait for data trans complete*/
    LINFlexD_1.UARTSR.R &= UART_DTFTFF;      /* Clear DTFTFF flag - W1C */
  }
  LINFlexD_1.BDRL.B.DATA0 = Data;            /* Transmit 8 bits Data */
}
/*****************************************************************************/

unsigned char checkLINFlexD_1()  {      /* Optional utility for status check */
  return( LINFlexD_1.UARTSR.B.DRFRFE ); /* Return Receive Buffer Status */
}
/*****************************************************************************/

void echoLINFlexD_1() {                 /* Optional utility to echo char. */
  txLINFlexD_1( rxLINFlexD_1() );
}
/*****************************************************************************/

void initLINFlexD_0 ( unsigned int MegaHertz, unsigned int BaudRate ) {
  unsigned int Fraction;
  unsigned int Integer;

  LINFlexD_0.LINCR1.B.INIT = 1;     /* Enter Initialization Mode */
  LINFlexD_0.LINCR1.B.SLEEP = 0;    /* Exit Sleep Mode */
  LINFlexD_0.UARTCR.B.UART = 1;     /* UART Enable- Req'd before UART config.*/
  LINFlexD_0.UARTCR.R = 0x0033;     /* UART Ena, 1 byte tx, no parity, 8 data*/
  LINFlexD_0.UARTSR.B.SZF = 1;      /* CHANGE THIS LINE   Clear the Zero status bit */
  LINFlexD_0.UARTSR.B.DRFRFE = 1;   /* CHANGE THIS LINE  Clear DRFRFE flag - W1C */

  BaudRate  = (MegaHertz * 1000000) / BaudRate;
  Integer   = BaudRate / 16;
  Fraction  = BaudRate - (Integer * 16);

  LINFlexD_0.LINIBRR.R = Integer;
  LINFlexD_0.LINFBRR.R = Fraction;

  LINFlexD_0.LINCR1.B.INIT = 0;     /* Exit Initialization Mode */

  UARTFirstTransmitFlag_0 = 1;        /* Indicate no Tx has taken place yet */

  /* Configure LINFlexD_0 TxD Pin. */
  SIUL2.MSCR[PB2].B.SSS = 0b0001; //Pad PF14: Set to LINFlex_0 TxD. Must choose this option because F14 leads to LIN PHY of motherboard
  SIUL2.MSCR[PB2].B.OBE = 1; //Enable output buffer
  SIUL2.MSCR[PB2].B.SRC = 3; //Full drive-strength without slew rate control

  /* Configure LINFlexD_0 RxD Pin. */
  SIUL2.MSCR[PB3].B.IBE = 1; //Pad PF15: Enable input buffer
  SIUL2.IMCR[165].B.SSS = 0b0001; //Connect LINFlexD_0 signal to PTB3
}
/*****************************************************************************/

//char message[] = "Welcome to MPC5748G! ";

char message_0[] = {"Hello"};

void testLINFlexD_0( void )  {     /* Display message to terminal */
  int i, size, j;
  uint8_t result[sizeof(message_0)] = {0};

  size = sizeof(message_0);
  /* Send to LINFlex_1. */
  for (i = 0; i < size; i++) {
    txLINFlexD_0(message_0[i]); //Send from LINFlex_0
    result[i] = rxLINFlexD_1(); //Get the character from LINFlex_0

    if(result[i] == message_0[i]){ //Flash PA0 5 times if result is the same as the sent character
    	SIUL2.GPDO[PA0].B.PDO = 0;
    	for(j = 0; j < 5; j++){
    		int counter = 0;
    		SIUL2.GPDO[PA0].B.PDO ^= 1; //Toggle LED
    		while(counter < 50000){ //Wait
    			counter++; //increment counter
    		}
    		counter=0;
    	}
    }
  }
}
/*****************************************************************************/

unsigned char rxLINFlexD_0() {
  while (LINFlexD_0.UARTSR.B.DRFRFE == 0); /* Wait for dta reception complete*/
  LINFlexD_0.UARTSR.R &= UART_DRFRFE;      /* Clear data receptoin flag W1C */
  return( LINFlexD_0.BDRM.B.DATA4 );       /* Read byte of Data */
}
/*****************************************************************************/

void txLINFlexD_0( unsigned char Data ) {
  if( UARTFirstTransmitFlag_0 )   {         /* 1st byte transmit after init: */
    UARTFirstTransmitFlag_0 = 0;            /* Clear variable */
  }
  else {                                  /* Normal tranmit (not 1st time): */
    while (LINFlexD_0.UARTSR.B.DTFTFF == 0); /* Wait for data trans complete*/
    LINFlexD_0.UARTSR.R &= UART_DTFTFF;      /* Clear DTFTFF flag - W1C */
  }
  LINFlexD_0.BDRL.B.DATA0 = Data;            /* Transmit 8 bits Data */
}
/*****************************************************************************/

unsigned char checkLINFlexD_0()  {      /* Optional utility for status check */
  return( LINFlexD_0.UARTSR.B.DRFRFE ); /* Return Receive Buffer Status */
}
/*****************************************************************************/

void echoLINFlexD_0() {                 /* Optional utility to echo char. */
  txLINFlexD_0( rxLINFlexD_0() );
}
/*****************************************************************************/

