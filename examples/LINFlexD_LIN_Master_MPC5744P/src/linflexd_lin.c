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

uint8_t RxBuffer[8] = {0};

void initLINFlexD_1 (void) {     /* Master at 10.417K baud with 80MHz LIN_CLK */

  LINFlexD_1.LINCR1.B.INIT = 1;    /* Put LINFlex hardware in init mode */
  LINFlexD_1.LINCR1.R= 0x00000311; /* Configure module as LIN master & header */
  LINFlexD_1.LINIBRR.B.IBR= 480; /* Mantissa baud rate divider component */
        /* Baud rate divider = 80 MHz LIN_CLK input / (16*10417K bps) ~=480 */
  LINFlexD_1.LINFBRR.B.FBR = 0; /* Fraction baud rate divider comonent */
  LINFlexD_1.LINCR1.R= 0x00000310; /* Change module mode from init to normal */

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

void initLINFlexD_0 (void) {     /* Master at 10.417K baud with 80MHz LIN_CLK */

  LINFlexD_0.LINCR1.B.INIT = 1;    /* Put LINFlex hardware in init mode */

#if !SELFTEST
  LINFlexD_0.LINCR1.R= 0x00000311; /* Configure module as LIN master & header */
#else
  LINFlexD_0.LINCR1.R = 0x000003B1;	/* Configure module as LIN master & loopback mode and BF=1 */
#endif
  LINFlexD_0.LINIBRR.B.IBR= 480; /* Mantissa baud rate divider component */
        /* Baud rate divider = 80 MHz LIN_CLK input / (16*10417K bps) ~=480 */
  LINFlexD_0.LINFBRR.B.FBR = 0; /* Fraction baud rate divider comonent */
#if !SELFTEST
  LINFlexD_0.LINCR1.R= 0x00000310; /* Change module mode from init to normal */
#else
  LINFlexD_0.LINCR1.R = 0x000003B0;	/* Change module mode from init to normal */
#endif

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

#if !SELFTEST
void receiveLINframe_0 (void) {      /* Request data from ID 0x15 */
	uint8_t i;

  LINFlexD_0.BIDR.R = 0x00001C15; /* Init header: ID=0x15, 8 B, Rx, enh cksum */
  LINFlexD_0.LINCR2.B.HTRQ = 1;   /* Request header transmission */

  while (!LINFlexD_0.LINSR.B.DRF); /* Wait for data receive complete flag */
                              /* Code waits here if no slave response */
  for (i=0; i<4;i++){         /* If received less than or equal 4 data bytes */
	RxBuffer[i]= (LINFlexD_0.BDRL.R>>(i*8)); /* Fill buffer in reverse order */
  }
  for (i=4; i<8;i++){         /* If received more than 4 data bytes: */
	RxBuffer[i]= (LINFlexD_0.BDRM.R>>((i-4)*8)); /* Fill rest in reverse order */
	if(RxBuffer[i]){}
  }
  LINFlexD_0.LINSR.R = 0x00000204;   /* Clear DRF flag */
}
#else
void receiveLINframe_0(void){
	/* Wait for HRF flag to signify header reception */
	while(LINFlexD_0.LINSR.B.HRF == 0);

	LINFlexD_0.LINSR.R |= 0x00000001; //Clear HRF flag. W1C

	/* Wait for data reception to complete */
	while(LINFlexD_0.LINSR.B.DRF == 0);

	uint8_t i = 0;

	/* Grab the "hello   " string from data buffer */
	for (i=0; i<4;i++){         /* If received less than or equal 4 data bytes */
		RxBuffer[i]= (LINFlexD_0.BDRL.R>>(i*8)); /* Fill buffer in reverse order */
	}
	for (i=4; i<8;i++){         /* If received more than 4 data bytes: */
		RxBuffer[i]= (LINFlexD_0.BDRM.R>>((i-4)*8)); /* Fill rest in reverse order */
		if(RxBuffer[i]){}
	}

	LINFlexD_0.LINSR.R = 0x00000204; 	/* Clear DRF flag. Clear RMB to free message buffer */
}
#endif

