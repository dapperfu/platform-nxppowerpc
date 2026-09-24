/* main.c - LP STOP example for MPC5744P */
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
* Brief            	This example shows STOP mode entry and exit.
*
********************************************************************************
* Detailed Description:
* NOTE: This demo currently does not work when running from P&E or GHS debuggers. It will get stuck trying to enter STOP0 mode!
* If you are using one of these debuggers, flash your program, then disconnect and reset the board for it to run freely.
* Program runs fine on Lauterbach, however.
*
* This program tests the ability of the MPC574xP to enter into and wake up from STOP0 mode.
* Before entering STOP mode, the PIT will be enabled to interrupt after 1 second.  The PIT interrupt
* will wake up the system from STOP mode and disable the PIT channel that did the waking up.  The interrupt
* will also toggle the blue LED six times.  Note that there is an errata for MPC5744P where an internal
* module sometimes cannot wake up system from STOP mode.
* The first time, the red LED (PC11) will turn on.  When you wake up the MCU, that LED will turn back off.
* The second time it enters STOP0 mode, the green LED (PC12) will turn on and turn back off when you wake up the part.
*
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        None
* Fsys:            160 MHz PLL on 40MHz external oscillator
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  When STOP mode is entered, connect NMI to GND (J5_12).
* 					NMI is not mapped to a header on XDEVKIT-MPC5744P, but
* 					functionality still works if you connect a wire to GND and
* 					then tap the other end at the NMI pin on the MPC5744P chip.
* 					The NMI pin is pin 1 on the chip.  Look for the small circle
* 					at the corner of the MPC5744P chip.  Pin 1 is the pin next to
* 					the "1" label next to that small circle.
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/

#include "derivative.h" /* include peripheral declarations */
#include "project.h"
#include "mode_entry.h"

#define KEY_VALUE1 0x5AF0ul
#define KEY_VALUE2 0xA50Ful

extern void xcptn_xmpl(void);
void peri_clock_gating (void); /* Configure gating/enabling peri. clocks */
void PIT0_Init(void);
void PIT0_ISR(void);

__attribute__ ((section(".text")))
int main(void)
{
	int i = 0;
	uint32_t STOP_counter = 0;
	
	xcptn_xmpl ();              /* Configure and Enable Interrupts */

	peri_clock_gating();   /* configure gating/enabling peri. clocks for modes*/
	                         /* configuration occurs after mode transition */
	system160mhz();
	           /* Sets clock dividers= max freq, calls PLL_160MHz function which:
                  MC_ME.ME: enables all modes for Mode Entry module
	           	  Connects XOSC to PLL
		          PLLDIG: LOLIE=1, PLLCAL3=0x09C3_C000, no sigma delta, 160MHz
		          MC_ME.DRUN_MC: configures sysclk = PLL
		          Mode transition: re-enters DRUN mode
	            */

	/* Enable STOP0 and RUN3 modes. */
	MC_ME.ME.R = 0x00000480;

	 /* MPC574xP cannot transition from DRUN to STOP0. Must go to a RUNx mode first.
	  * Additional mode configurations for STOP, RUN3 modes & enter RUN3 mode: */
	 MC_ME.RUN3_MC.R = 0x001F0010; /* mvron=1 FLAON=RUN XOSCON=1 FIRCON=1 SYSCLK=FIRC */
	 MC_ME.STOP0_MC.R   = 0x00130010; /* MVRON=1 FLAON=RUN XOSCON=1 FIRCON=1 sysclk=FIRC */

	 MC_ME.MCTL.R = 0x70005AF0;      /* Enter RUN3 Mode & Key */
	 MC_ME.MCTL.R = 0x7000A50F;      /* Enter RUN3 Mode & Inverted Key */
	 while (MC_ME.GS.B.S_MTRANS) {}  /* Wait for RUN3 mode transition to complete */
	                              /* Note: could wait here using timer and/or I_TC IRQ */
	 while(MC_ME.GS.B.S_CURRENT_MODE != 7) {} /* Verify RUN3 (0x7) is the current mode */

	 /* Enable RGB LED */
	 SIUL2.MSCR[PC11].B.OBE = 1;  /* Pad PC11: OBE=1. Red LED */
	 SIUL2.MSCR[PC12].B.OBE = 1;  /* Pad PC12: OBE=1. Green LED. */
	 SIUL2.MSCR[PC13].B.OBE = 1;  /* Pad PC13: OBE=1. Blue LED */

	 /* Turn LEDs off */
	 SIUL2.GPDO[PC11].R = 1;
	 SIUL2.GPDO[PC12].R = 1;
	 SIUL2.GPDO[PC13].R = 1;

	 /* Configure WKPU (wakeup unit). */
	 WKPU.NCR.B.NDSS0 = 0x2;         // Machine check interrupt enabled. - seems necessary to wake up without generating interrupt.
	 WKPU.NCR.B.NFEE0 = 1;           // Enable falling edge event
	 WKPU.NCR.B.NWRE0 = 1;           // Enable wake up pin

	 while(1) {

		 /* Turn on red LED. */
		 SIUL2.GPDO[PC11].B.PDO = 0;

		 PIT0_Init();				/* Initialize the PIT to wakeup system upon PIT interrupt */
		 enter_STOP_mode();         /* Enter STOP mode */

	   /* To wake up from STOP mode, drive NMI signal low. The NMI pin is located at J1.2 of MPC574xP144LQFPDS and MPC574xP257BGADS.
	    * Attach a wire to that pin and connect the other end to a GND pin of your choice. Driving the NMI signal low wakes the MCU.
	    */
	                                /* ON STOP MODE EXIT, CODE CONTINUES HERE: */
	   while(MC_ME.GS.B.S_CURRENT_MODE != 7) {} /* Verify RUN3 (0x7) is current mode */

	   SIUL2.GPDO[PC11].B.PDO = 1; //Turn red LED back off

	   /* Clear the NMI flag. It is write-1-clear. If the flag remains set, then
	    * the MCU will wake up immediately after it goes into STOP0 mode because it will
	    * think that the NMI interrupt needs to be fired.
	    */
	   WKPU.NSR.B.NIF0 = 1;

	   /* Wait for a short time. */
	   for(i=0; i<500000; i++);

	   /* Repeat with Green LED. */
	   SIUL2.GPDO[PC12].B.PDO = 0;

	   PIT0_Init();				/* Initialize the PIT to wakeup system upon PIT interrupt */
	   enter_STOP_mode ();         /* Enter STOP mode */
	                               /* ON STOP MODE EXIT, CODE CONTINUES HERE: */
	   while(MC_ME.GS.B.S_CURRENT_MODE != 7) {} /* Verify RUN3 (0x7) is current mode */

	   /* Turn LED back off. */
	   SIUL2.GPDO[PC12].B.PDO = 1;

	   /* Clear the WKPU flag. */
	   WKPU.NSR.B.NIF0 = 1;

	   /* Wait for a short time. */
	   for(i=0; i<500000; i++);

	   STOP_counter++;                        /* Counter for STOP mode pairs of cycles */

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
  MC_ME.RUN_PC[7].R = 0x00000088;  /* Run Peri. Cfg 7 settings: run in DRUN, RUN3 modes */
  MC_ME.LP_PC[7].R = 0x00000400;   /* LP Peri. Cfg. 7 settings: run in STOP  */

  MC_ME.PCTL30.R = 0x3F; /* PIT_0: select peri. cfg. RUN_PC[7], LP_PC[7] */
}

/*****************************************************************************/
/* PIT0_Init			                                                     */
/* Description: Initializes PIT0 to wait 1 second					         */
/*****************************************************************************/
void PIT0_Init(){
	PIT_0.MCR.B.MDIS = 1; //Disable PIT0
	PIT_0.TIMER[0].LDVAL.R = 10000000; //PIT runs on PBRIDGEx_CLK (10 MHz). 10 million ticks for 1 sec
	PIT_0.TIMER[0].TCTRL.R = 0x00000003; //Enable timer and interrupt

	/* Enable PIT_0 interrupt in INTC */
	INTC_0.PSR[226].R = 0x800F; //Highest priority

	/* Reenable PIT_0 */
	PIT_0.MCR.B.MDIS = 0;
}

/*****************************************************************************/
/* PIT0_ISR			                                                         */
/* Description: PIT0 Interrupt handler. Toggles LED					         */
/*****************************************************************************/
void PIT0_ISR(){
	int i, j;
	/* Clear the Flag. */
	PIT_0.TIMER[0].TFLG.R = 0x00000001; //W1C bit

	/* Toggle 6 times. */
	for(i = 0; i < 6; i++){
		SIUL2.GPDO[PC13].R ^= 1; //Toggle blue LED
		for(j = 0; j<500000; j++){} //Wait
	}

	/* Disable PIT channel 0 */
	PIT_0.TIMER[0].TCTRL.B.TEN = 0;
}
