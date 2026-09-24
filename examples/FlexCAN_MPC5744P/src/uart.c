/*******************************************************************************
* Freescale Semiconductor Inc.
* (c) Copyright 2010 Freescale Semiconductor, Inc.
* ALL RIGHTS RESERVED.
********************************************************************************
Services performed by FREESCALE in this matter are performed AS IS and without
any warranty. CUSTOMER retains the final decision relative to the total design
and functionality of the end product. FREESCALE neither guarantees nor will be
held liable by CUSTOMER for the success of this project.
FREESCALE DISCLAIMS ALL WARRANTIES, EXPRESSED, IMPLIED OR STATUTORY INCLUDING,
BUT NOT LIMITED TO, IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
A PARTICULAR PURPOSE ON ANY HARDWARE, SOFTWARE ORE ADVISE SUPPLIED 
TO THE PROJECT BY FREESCALE, AND OR NAY PRODUCT RESULTING FROM FREESCALE 
SERVICES. IN NO EVENT SHALL FREESCALE BE LIABLE FOR INCIDENTAL OR CONSEQUENTIAL 
DAMAGES ARISING OUT OF THIS AGREEMENT.
CUSTOMER agrees to hold FREESCALE harmless against any and all claims demands 
or actions by anyone on account of any damage, or injury, whether commercial,
contractual, or tortuous, rising directly or indirectly as a result 
of the advise or assistance supplied CUSTOMER in connection with product, 
services or goods supplied under this Agreement.
********************************************************************************
* File:             uart.c
* Owner:            PetrS
* Version:          1.0
* Date:             Mar-20-2015
* Classification:   General Business Information
* Brief:            terminal IO. 
********************************************************************************
* Detailed Description: 
* 
* Implements GHS ind_io library function calls to do printf() on LINFlexD0
* Functions "read" and "write" defined in this module replace the ones from the
* library so finally we send printf() to LINFlexD0
*
* ------------------------------------------------------------------------------
* Test HW:  
* Target :  
* Terminal: 
* Fsys:     200MHz
*
********************************************************************************
Revision History:
1.0     Mar-20-2015     PetrS  Initial Version
*******************************************************************************/

#include "MPC5744P.h"
#include "can.h"
#include "uart.h"

/*******************************************************************************
* Global variables
*******************************************************************************/

/*******************************************************************************
* Constants and macros
*******************************************************************************/

/*******************************************************************************
* Local types
*******************************************************************************/

/*******************************************************************************
* Local function prototypes
*******************************************************************************/

/*******************************************************************************
* Local variables
*******************************************************************************/

/*******************************************************************************
* Local functions
*******************************************************************************/ 

/*******************************************************************************
Function Name : LINFlexD_0_Init
Engineer      : PetrS
Date          : Mar-20-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : initializes MPC5744P's LINFlexD_0 module for UART mode
Issues        : expecting baud clock is 100MHz (HALFSYS_CLK)
*******************************************************************************/
void LINFlexD_0_Init (void)
{
	/* enter INIT mode */
	LINFlexD_0.LINCR1.R = 0x0081; /* SLEEP=0, INIT=1 */
	
	/* wait for the INIT mode */
	while (0x1000 != (LINFlexD_0.LINSR.R & 0xF000)) {}
		
	/* configure pads */
	/* Configure pad PB2 for AF1 func: LIN0TX */
	SIUL2.MSCR[18].B.SSS = 1;    /* Pad PB2: Source signal is LIN0_TXD  */
	SIUL2.MSCR[18].B.OBE = 1;    /* Pad PB2: Output Buffer Enable */
	SIUL2.MSCR[18].B.SRC = 3;    /* Pad PB2: Maximum slew rate */
	/* Configure pad PB3 for LIN0RX */
	SIUL2.MSCR[19].B.IBE = 1;    /* Pad PB3: Enable pad for input - LIN0_RXD */
	SIUL2.IMCR[165].B.SSS = 1;   /* LIN0_RXD: connected to pad PB3 */

	
	/* configure for UART mode */
	LINFlexD_0.UARTCR.R = 0x0001; /* set the UART bit first to be able to write the other bits */
	
	LINFlexD_0.UARTCR.R = 0x0033; /* 8bit data, no parity, Tx and Rx enabled, UART mode */
								 /* Transmit buffer size = 1 (TDFL = 0 */
								 /* Receive buffer size = 1 (RDFL = 0) */

	/* assuming 80 MHz baud clock (halfsys_clk below)*/
	/* LFDIV = halfsys_clk / (16 * desired baudrate)
	   LINIBRR = integer part of LFDIV
	   LINFBRR = 16 * fractional part of LFDIV (after decimal point)

	   for instance:
	   LFDIV = 80e6/(16*19200) = 260.417
	   LINIBRR = 260
	   LINFBRR = 16*0.417 = 6.672
	*/

	LINFlexD_0.LINFBRR.R = 6;
	LINFlexD_0.LINIBRR.R = 260;
		
	/* enter NORMAL mode */
	LINFlexD_0.LINCR1.R = 0x0080; /* INIT=0 */
}

/*******************************************************************************
Function Name : LINFlexD_1_Init
Engineer      : DavidC
Date          : Dec-1-2016
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : initializes MPC5744P's LINFlexD_1 module for UART mode
Issues        : expecting baud clock is 100MHz (HALFSYS_CLK)
*******************************************************************************/
void LINFlexD_1_Init (void)
{
	/* enter INIT mode */
	LINFlexD_1.LINCR1.R = 0x0081; /* SLEEP=0, INIT=1 */

	/* wait for the INIT mode */
	while (0x1000 != (LINFlexD_1.LINSR.R & 0xF000)) {}

	/* configure pads */
	/* Configure pad PD9 for AF1 func: LIN1TX */
	SIUL2.MSCR[57].B.SSS = 2;    /* Pad PD9: Source signal is LIN1_TXD  */
	SIUL2.MSCR[57].B.OBE = 1;    /* Pad PD9: Output Buffer Enable */
	SIUL2.MSCR[57].B.SRC = 3;    /* Pad PD9: Maximum slew rate */
	/* Configure pad PD12 for LIN1RX */
	SIUL2.MSCR[60].B.IBE = 1;    /* Pad PD12: Enable pad for input - LIN1_RXD */
	SIUL2.IMCR[166].B.SSS = 2;   /* LIN1_RXD: connected to pad PD12 */


	/* configure for UART mode */
	LINFlexD_1.UARTCR.R = 0x0001; /* set the UART bit first to be able to write the other bits */

	LINFlexD_1.UARTCR.R = 0x0033; /* 8bit data, no parity, Tx and Rx enabled, UART mode */
								 /* Transmit buffer size = 1 (TDFL = 0 */
								 /* Receive buffer size = 1 (RDFL = 0) */

	/* assuming 80 MHz baud clock (halfsys_clk below)*/
	/* LFDIV = halfsys_clk / (16 * desired baudrate)
	   LINIBRR = integer part of LFDIV
	   LINFBRR = 16 * fractional part of LFDIV (after decimal point)

	   for instance:
	   LFDIV = 80e6/(16*19200) = 260.417
	   LINIBRR = 260
	   LINFBRR = 16*0.417 = 6.672
	*/

	LINFlexD_1.LINFBRR.R = 6;
	LINFlexD_1.LINIBRR.R = 260;

	/* enter NORMAL mode */
	LINFlexD_1.LINCR1.R = 0x0080; /* INIT=0 */
}


/*******************************************************************************
Function Name : TransmitData
Engineer      : B48683
Date          : Dec-1-2016
Parameters    : pBuf - input string. won't be modified by the function
              : cnt  - input number of characters to be transmitted
Modifies      : NONE
Returns       : NONE
Notes         : Tx data on LINFlexD_1. polled mode.
Issues        : NONE 
*******************************************************************************/
void TransmitData(const char *pBuf, uint32_t cnt)
{
    uint8_t	j = 0; // Dummy variable
    
#if DEVKIT
    for (j=0; j< cnt; j++) 
    {  // Loop for character string
   	    LINFlexD_1.BDRL.B.DATA0 = *(pBuf+j);
   	      //write character to transmit buffer
	    while (1 != LINFlexD_1.UARTSR.B.DTFTFF) {}
	      // Wait for data transmission completed flag
	    LINFlexD_1.UARTSR.R = 0x0002;
	      // clear the DTF flag and not the other flags
    }
#else
    for (j=0; j< cnt; j++)
    {  // Loop for character string
   	    LINFlexD_0.BDRL.B.DATA0 = *(pBuf+j);
   	      //write character to transmit buffer
	    while (1 != LINFlexD_0.UARTSR.B.DTFTFF) {}
	      // Wait for data transmission completed flag
	    LINFlexD_0.UARTSR.R = 0x0002;
	      // clear the DTF flag and not the other flags
    }
#endif

    
}

/*******************************************************************************
Function Name : ReceiveData
Engineer      : B48683
Date          : Dec-1-2016
Parameters    : pBuf - address of a char where the received char is written to
                       the address (pBuf) doesn't change in the function
Modifies      : NONE
Returns       : NONE
Notes         : Rx data on LINFlexD_1. polled mode.
Issues        : NONE 
*******************************************************************************/
void ReceiveData(char *pBuf) 
{
    
    int32_t rx_data;
#if DEVKIT
    	/* wait for DRF */
	while (1 != LINFlexD_1.UARTSR.B.DRFRFE) {}  /* Wait for data reception completed flag */
		
	/* wait for RMB */
	while (1 != LINFlexD_1.UARTSR.B.RMB) {}  /* Wait for Release Message Buffer */
	
	/* get the data */
	rx_data = LINFlexD_1.BDRM.R; // read whole register due to erratum e4897PS
    
	*pBuf = (uint8_t)rx_data; // take 
	
	/* clear the DRF and RMB flags by writing 1 to them */
	LINFlexD_1.UARTSR.R = 0x0204;
#else
	/* wait for DRF */
	while (1 != LINFlexD_0.UARTSR.B.DRFRFE) {}  /* Wait for data reception completed flag */

	/* wait for RMB */
	while (1 != LINFlexD_0.UARTSR.B.RMB) {}  /* Wait for Release Message Buffer */

	/* get the data */
	rx_data = LINFlexD_0.BDRM.R; // read whole register due to erratum e4897PS
    
	*pBuf = (uint8_t)rx_data; // take

	/* clear the DRF and RMB flags by writing 1 to them */
	LINFlexD_0.UARTSR.R = 0x0204;
#endif
}


/*******************************************************************************
* Global functions
*******************************************************************************/ 

/*
Methods called by GHS ind_io libraries to perform console IO:
*/


int read (int fno, void *buf, int size)
{
	int count;
	
	for (count = 0; count < size; count++)
        {
	    ReceiveData( (char *)buf + count );
        }

	return size;
}


int write (int fno, const void *buf, int size)
{ 
	TransmitData ((const char *)buf,size);
  	return size; 
}
