/*
 * linflexd_lin.c
 *
 *  Created on: Mar 2, 2016
 *      Author: B55457
 */


/*****************************************************************************/
/* FILE NAME: linflexd_lin.c                   COPYRIGHT (c) Freescale 2015  */
/*                                                      All Rights Reserved  */
/* DESCRIPTION: Transmit & receive LIN messages using LINflexD modules.      */
/*  - LINFlexD_1 module divides its 80 MHz LIN_CLK input to get 10.417K baud.*/
/*  - Transmit function sends 'hello   ' as 8 bytes with ID 0x35.            */
/*  - Receive function requests data from slave with ID 0x35. An external    */
/*    node or LIN tool is needed complete reception.  Without the node or    */
/*    tool, code will wait forever for the receive flag.                     */
/*                                                                           */
/*****************************************************************************/
/* REV      AUTHOR        DATE        DESCRIPTION OF CHANGE                  */
/* ---   -----------    ----------    ---------------------                  */
/* 1.0	 R. Delgado     Jan 13 2015   Initial Version                        */
/*****************************************************************************/
#include "linflexd_lin.h"
#include "uart.h"
#include <string.h>

extern uint8_t Message1[];
extern uint8_t Message2[];
extern uint8_t newline[];
uint8_t RxBuffer[8] = {0};

void initLINFlexD_1 (void) {     /* Master at 10.417K baud with 80MHz LIN_CLK */

  LINFlexD_1.LINCR1.B.INIT = 1;    /* Put LINFlex hardware in init mode */
  LINFlexD_1.LINCR1.R= 0x00000301; /* Configure module as LIN slave */
  LINFlexD_1.LINIBRR.B.IBR= 480; /* Mantissa baud rate divider component */
        /* Baud rate divider = 80 MHz LIN_CLK input / (16*10417K bps) ~=480 */
  LINFlexD_1.LINFBRR.B.FBR = 0; /* Fraction baud rate divider comonent */
  LINFlexD_1.LINCR1.R= 0x00000300; /* Change module mode from init to normal */

  /* Configure LINFlexD_1 TxD Pin. */
  SIUL2.MSCR[PF14].B.SSS = 0b0001; //Pad PF14: Set to LINFlex_1 TxD. Must choose this option because F14 leads to LIN PHY of motherboard
  SIUL2.MSCR[PF14].B.OBE = 1; //Enable output buffer
  SIUL2.MSCR[PF14].B.SRC = 3; //Full drive-strength without slew rate control

  /* Configure LINFlexD_1 RxD Pin. */
  SIUL2.MSCR[PF15].B.IBE = 1; //Pad PF15: Enable input buffer
  SIUL2.IMCR[166].B.SSS = 0b0011; //Connect LINFlexD_2 signal to PF15
}

void transmitLINframe_1 (void) {   /* Transmit one frame 'hello    ' to ID 0x35*/
  LINFlexD_1.BDRM.R = 0x2020206F; /* Load most significant bytes '   o' */
  LINFlexD_1.BDRL.R = 0x6C6C6548; /* Load least significant bytes 'lleh' */
  LINFlexD_1.BIDR.R = 0x00001E35; /* Init header: ID=0x35, 8 B, Tx, enh cksum*/
  LINFlexD_1.LINCR2.B.HTRQ = 1;   /* Request header transmission */

  while (!LINFlexD_1.LINSR.B.DTF); /* Wait for data transfer complete flag */
  LINFlexD_1.LINSR.R = 0x00000002;   /* Clear DTF flag */
}

void receiveLINframe_1 (void) {      /* Request data from ID 0x15 */
	uint8_t RxBuffer[8] = {0};
	uint8_t i;

	while(LINFlexD_1.LINSR.B.HRF == 0);

  LINFlexD_1.BIDR.R = 0x00001C15; /* Init header: ID=0x15, 8 B, Rx, enh cksum */
  LINFlexD_1.LINCR2.B.HTRQ = 1;   /* Request header transmission */
  while (!LINFlexD_1.LINSR.B.DRF); /* Wait for data receive complete flag */
                              /* Code waits here if no slave response */
  for (i=0; i<4;i++){         /* If received less than or equal 4 data bytes */
	RxBuffer[i]= (LINFlexD_1.BDRL.R>>(i*8)); /* Fill buffer in reverse order */
  }
  for (i=4; i<8;i++){         /* If received more than 4 data bytes: */
	RxBuffer[i]= (LINFlexD_1.BDRM.R>>((i-4)*8)); /* Fill rest in reverse order */
	if(RxBuffer[i]){}
  }
  LINFlexD_1.LINSR.R = 0x00000004;   /* Clear DRF flag */
}

void initLINFlexD_0 (void) {     /* Slave at 10.417K baud with 80MHz LIN_CLK */

	LINFlexD_0.LINCR1.B.INIT = 1;    /* Put LINFlex hardware in init mode */
	LINFlexD_0.LINCR1.R= 0x00000301; /* Configure module as LIN slave */
	LINFlexD_0.LINIBRR.B.IBR= 480; /* Mantissa baud rate divider component */
	/* Baud rate divider = 80 MHz LIN_CLK input / (16*10417K bps) ~=480 */
	LINFlexD_0.LINFBRR.B.FBR = 0; /* Fraction baud rate divider comonent */

	/* Change module mode from init to normal. As per LINSR[HRF] bit field description,
	 * LINCR1[BF] must be 1 when no filters are active (which is the case in this example)
	 * in order for LIN slave to respond to incoming frames.
	 */
	LINFlexD_0.LINCR1.R = 0x00000380;

	/* Configure LINFlexD_0 TxD Pin. */
	SIUL2.MSCR[PB2].B.SSS = 0b0001; //Pad PF14: Set to LINFlex_1 TxD. Must choose this option because F14 leads to LIN PHY of motherboard
	SIUL2.MSCR[PB2].B.OBE = 1; //Enable output buffer
	SIUL2.MSCR[PB2].B.SRC = 3; //Full drive-strength without slew rate control

	/* Configure LINFlexD_0 RxD Pin. */
	SIUL2.MSCR[PB3].B.IBE = 1; //Pad PF15: Enable input buffer
	SIUL2.IMCR[165].B.SSS = 0b0001; //Connect LINFlexD_2 signal to PF15
}

void transmitLINframe_0 (void) {   /* Transmit one frame 'hello    ' to ID 0x35*/
  LINFlexD_0.BDRM.R = 0x2020206F; /* Load most significant bytes '   o' */
  LINFlexD_0.BDRL.R = 0x6C6C6548; /* Load least significant bytes 'lleh' */
  LINFlexD_0.BIDR.R = 0x00001E35; /* Init header: ID=0x35, 8 B, Tx, enh cksum*/
  LINFlexD_0.LINCR2.B.HTRQ = 1;   /* Request header transmission */

  while (!LINFlexD_0.LINSR.B.DTF); /* Wait for data transfer complete flag */
  LINFlexD_0.LINSR.R = 0x00000002;   /* Clear DTF flag */
}

void receiveLINframe_0 (void) {      /* Request data from ID 0x15 */
	uint8_t i;

	/* Wait for header to be received */
	while(!(LINFlexD_0.LINSR.B.HRF));

	/* Read the identifier. Look for ID=0x35 */
	if(LINFlexD_0.BIDR.B.ID == 0x35){
		/* Clear the HRF flag */
		LINFlexD_0.LINSR.R |= 0x00000001; //W1C

		/* If ID=0x35, receive bytes from master.
		 * Configure the data length.
		 */
		LINFlexD_0.BIDR.B.DFL = 7; //Receive 8 bytes. DFL = number of data bytes - 1 = 8-1 = 7

		/* Configure checksum for enhanced checksum */
		LINFlexD_0.BIDR.B.CCS = 0;

		/* Configure to receive */
		LINFlexD_0.BIDR.B.DIR = 0;

		/* Wait for DRF bit to be set */
		while(!(LINFlexD_0.LINSR.B.DRF));

		/* Read the data bytes */
		for (i=0; i<4;i++){         /* If received less than or equal 4 data bytes */
			RxBuffer[i]= (LINFlexD_0.BDRL.R>>(i*8)); /* Fill buffer in reverse order */
		}
		for (i=4; i<8;i++){         /* If received more than 4 data bytes: */
			RxBuffer[i]= (LINFlexD_0.BDRM.R>>((i-4)*8)); /* Fill rest in reverse order */
			if(RxBuffer[i]){}
		}

		/* Clear DRF and release message buffer */
		LINFlexD_0.LINSR.R = 0x00000204;

	 	TransmitData((const char*)Message2,strlen((const char*)Message2));
	 	TransmitData((const char*)RxBuffer,8);
	 	TransmitData((const char*)newline,strlen((const char*)newline));

	/* Read the identifier. Look for ID=0x15 */
	}else if(LINFlexD_0.BIDR.B.ID == 0x15){
		/* If ID=0x15, send btyes to master.
		 * Fill the buffer with "World   ".
		 */
		LINFlexD_0.BDRL.B.DATA0 = 'W';
		LINFlexD_0.BDRL.B.DATA1 = 'o';
		LINFlexD_0.BDRL.B.DATA2 = 'r';
		LINFlexD_0.BDRL.B.DATA3 = 'l';
		LINFlexD_0.BDRM.B.DATA4 = 'd';
		LINFlexD_0.BDRM.B.DATA5 = ' ';
		LINFlexD_0.BDRM.B.DATA6 = ' ';
		LINFlexD_0.BDRM.B.DATA7 = ' ';

		/* Wait for LINSR[RXbusy] to clear. Cannot set
		 * DIR and DTRQ if RXbusy is set.
		 */
		while(LINFlexD_0.LINSR.B.RXbusy);

		/* Program CCS and DIR */
		LINFlexD_0.BIDR.B.CCS = 0; //Enhanced checksum
		LINFlexD_0.BIDR.B.DIR = 1; //Send bytes to master

		/* Specify number of bytes to send */
		LINFlexD_0.BIDR.B.DFL = 7; //Send 8 bytes. DFL = number of data bytes - 1 = 8-1 = 7

		/* Trigger data transmission */
		LINFlexD_0.LINCR2.R |= 0x00000400; //Set LINCR2[DTRQ] to transmit data

		/* Clear HRF finally. DTRQ must be set while HRF=1
		 * in order to prevent random setting of DTRQ.
		 */
		LINFlexD_0.LINSR.R |= 0x00000001;

		/* Wait for transmission to complete */
		while(!(LINFlexD_0.LINSR.B.DTF));

		/* Clear DTF */
		LINFlexD_0.LINSR.R |= 0x00000002;

		/* Print the message */
	 	TransmitData((const char*)Message1,strlen((const char*)Message1));
	}

}
