/* main.c - UART example for MPC5744P */
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
* Brief            	This example uses the UART function of the LINFlexD to transmit
* 					messages to PC UART terminal.
*
********************************************************************************
* Detailed Description:
* LINFlexD_UART example. Uses LINFlexD_1 to communicate to computer terminal like TeraTerm. Connect USB to PC.
* Sends a string to the computer terminal.
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        19200, 8N1, None
* Fsys:            160 MHz PLL on 40MHz external oscillator
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  None
*
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/

#include "derivative.h" /* include peripheral declarations */
#include "project.h"
#include "linflexd_uart.h"
#include "mode_entry.h"
#include "gpio.h"

#define KEY_VALUE1 0x5AF0ul
#define KEY_VALUE2 0xA50Ful

extern void xcptn_xmpl(void);
void peri_clock_gating(void);

__attribute__ ((section(".text")))
int main(void)
{
	
	xcptn_xmpl ();              /* Configure and Enable Interrupts */

	peri_clock_gating();       /* Config gating/enabling peri. clocks for modes*/
                             /* Configuraiton occurs after mode transition */
	system160mhz();            /* sysclk=160MHz, dividers configured, mode trans*/

	initGPIO();

	initLINFlexD_1(80,19200);

	testLINFlexD_1();            /* Send test message to PC terminal.*/

	for(;;); //Loop forever

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

  MC_ME.PCTL91.B.RUN_CFG = 0x1; //LINFlexD_1: Select peripheral config RUN_PC[1]. No LINFlex_D_2 on Panther
}
