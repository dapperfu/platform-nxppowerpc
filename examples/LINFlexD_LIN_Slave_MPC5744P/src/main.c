/* main.c - LIN Slave example for MPC5744P */
/* Description:  Measures eTimer pulse/period measurement */
/* Rev 1 Sept 06 2016 D Chung - initial version */
/* Copyright NXP Semiconductor, Inc 2016 All rights reserved. */

/*******************************************************************************
* NXP Semiconductor Inc.
* (c) Copyright 2016 NXP Semiconductor, Inc.
* ALL RIGHTS RESERVED.
********************************************************************************
Services performed by NXP in this matter are performed AS IS and without
any warranty. CUSTOMER retains the final decision relative to the total design
and functionality of the end product. NXP neither guarantees nor will be
held liable by CUSTOMER for the success of this project.
NXP DISCLAIMS ALL WARRANTIES, EXPRESSED, IMPLIED OR STATUTORY INCLUDING,
BUT NOT LIMITED TO, IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
A PARTICULAR PURPOSE ON ANY HARDWARE, SOFTWARE ORE ADVISE SUPPLIED TO THE PROJECT
BY NXP, AND OR NAY PRODUCT RESULTING FROM NXP SERVICES. IN NO EVENT
SHALL NXP BE LIABLE FOR INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING OUT OF
THIS AGREEMENT.

CUSTOMER agrees to hold NXP harmless against any and all claims demands or
actions by anyone on account of any damage, or injury, whether commercial,
contractual, or tortuous, rising directly or indirectly as a result of the advise
or assistance supplied CUSTOMER in connection with product, services or goods
supplied under this Agreement.
********************************************************************************
* File              main.c
* Owner             David Chung
* Version           1.0
* Date              Sept-6-2016
* Classification    General Business Information
* Brief            	This example uses the LIN function of the LINFlexD to receive
* 					and send a frame in slave mode.
*
********************************************************************************
* Detailed Description:
* Configures MPC5744P PLL to 160 MHz and LINFlexD to LIN slave mode.
* Receives "Hello" in slave mode. Then sends "World" back to master. To be
* used alongside LINFlexD_LIN_Master_MPC5744P so two boards are needed.
* One should be loaded with this program and the other should be loaded with
* the master program.  Flash the program then reset so both boards can run
* simultaneously.
*
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        19200, 8N1, None
* Fsys:            160 MHz PLL on 40MHz external oscillator
* Debugger:        PeMicro USB-ML-PPCNEXUS
* Target:          FLASH
* EVB connection:  USB to PC
*
* 					Master and slave boards must be powered by 12V external supply
* 					Connect 12V,5A supply to power jack P1.
* 					Select MCU to be powered from external supply. Connect jumper
* 					J13_1 to J13_2.
*
* 					12V must be connected to header pin. Put jumper onto J39.
*
* 					LIN0_Tx (PB3) and LIN0_Rx (PB2) connect to LIN generation
* 					MCU U8. Output of U8 leads to LINPHY P6.
* 					Connect the master P6 to slave P6: GND to GND (P6.1 to P6.1),
* 					LIN line to LIN line (P6.4 to P6.4), and provide 12V supply to
* 					LIN V supply P6.3 (J3_15 to P6.3). You can use a split wire so a
* 					single 12V source supplies 12V to both master Vsup and slave Vsup.
*
* 					Refer to setup.jpg for reference
*
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/

#include "derivative.h" /* include peripheral declarations */
#include "project.h"
#include "mode_entry.h"
#include "linflexd_lin.h"
#include "uart.h"
#include <string.h>

#define KEY_VALUE1 0x5AF0ul
#define KEY_VALUE2 0xA50Ful

extern void xcptn_xmpl(void);
void peri_clock_gating (void);

extern uint8_t RxBuffer[];
uint8_t Message1[] = {"\n\rLIN slave has transferred 'World' to master and received a word from master.\n\r"};
uint8_t Message2[] = {"\n\rThe slave receive buffer is now "};
uint8_t newline[] = {".\n\r"};

__attribute__ ((section(".text")))
int main(void)
{
	uint8_t i = 0;
	uint8_t intro[] = {"\n\rWelcome to the XDEVKIT-MPC5744P LIN Slave code example!\n\r"};
	uint8_t Message0[] = {"\n\rNo transfer has been made. The LIN slave receive buffer is 0x"};

	xcptn_xmpl ();              /* Configure and Enable Interrupts */

	peri_clock_gating();    /* Configure gating/enabling peri. clocks for modes*/
	                          /* Configuraiton occurs after mode transition */
	system160mhz();         /* sysclk=160MHz, dividers configured, mode trans*/

	initLINFlexD_0();			/* Initialize LINFlexD_0 as slave */

	LINFlexD_1_Init();		/* Initialize LINFlexD_1 to print UART messages */

	/* Print intro message */
	TransmitData((const char*)intro,strlen((const char*)intro));

	/* Print the original state of buffer.
	 * Convert to ASCII
	 */
	for(i=0;i<8;i++){
		if(RxBuffer[i] > 9){
			RxBuffer[i] = RxBuffer[i] + '0' + 7;
		}else{
			RxBuffer[i] = RxBuffer[i] + '0';
		}
	}

	TransmitData((const char*)Message0,strlen((const char*)Message0));
	TransmitData((const char*)RxBuffer,8);
	TransmitData((const char*)newline,strlen((const char*)newline));

	while (1) {
		receiveLINframe_0();	/* Receive one frame from master */
	}

	return 0;
}

/*****************************************************************************/
/* peri_clock_gating                                                         */
/* Description: Configures enabling clocks to peri modules or gating them off*/
/*              Default PCTL[RUN_CFG]=0, so by default RUN_PC[0] is selected.*/
/*              RUN_PC[0] is configured here to gate off all clocks.         */
/*****************************************************************************/

void peri_clock_gating (void) {
	MC_ME.RUN_PC[0].R = 0x00000000;  /* gate off clock for all RUN modes */
	MC_ME.RUN_PC[1].R = 0x000000FE;  /* config. peri clock for all RUN modes */
	MC_ME.PCTL204.B.RUN_CFG = 0x1; //LINFlexD_0: Select run config 1
	MC_ME.PCTL91.B.RUN_CFG = 0x1; //LINFlexD_1: Select peripheral config RUN_PC[1]. No LINFlex_D_2 on Panther
}
