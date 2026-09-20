/* main.c - Hello World + PLL + Interrupt example for MPC5744P */
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
* Brief             PLL Configured to 160 MHz. Three PIT channels
* 					and one SW interrupt drive RGB LED.
*
********************************************************************************
* Detailed Description:
* Configures MPC5744P PLL to 160 MHz, three PIT channels, and one SW interrupt.
* Each PIT is loaded with different period. SW interrupt is set after every four
* iterations of PIT0 interrupt.  PITs and SW interrupt each connected to an LED.
* LEDs flash at frequency of their corresponding interrupts.
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        None
* Fsys:            160 MHz PLL on 40MHz external oscillator
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  To see LED4 toggle, an external LED of your choice to
* 					PA0 (J5_1) and GND (J5_12), in series with current-limiting
* 					resistor (typically 330 Ohms). The other 3 LEDs are housed in
* 					onboard RGB LED.
*
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/

#include "derivative.h" /* include peripheral declarations */
#include "pit.h"
#include "gpio.h"
#include "mode_entry.h"

#define KEY_VALUE1 0x5AF0ul
#define KEY_VALUE2 0xA50Ful

#define LED1 SIUL2.GPDO[PC11].B.PDO
#define LED2 SIUL2.GPDO[PC12].B.PDO
#define LED3 SIUL2.GPDO[PC13].B.PDO
#define LED4 SIUL2.GPDO[PA0].B.PDO

extern void xcptn_xmpl(void);

void SW_INT_1_init(void);

__attribute__ ((section(".text")))
int main(void)
{
	int counter = 0;
	
	xcptn_xmpl ();              /* Configure and Enable Interrupts */
    //Since We are using PIT- one of the peripherials. We need to enable peri clocks.

	peri_clock_gating();     /* Config gating/enabling peri. clocks for modes*/
                             /* Configuraiton occurs after mode transition! */

    system160mhz();
    /* Sets clock dividers= max freq,
       calls PLL_160MHz function which:
       MC_ME.ME: enables all modes for Mode Entry module
       Connects XOSC to PLL
       PLLDIG: LOLIE=1, PLLCAL3=0x09C3_C000, no sigma delta, 160MHz
       MC_ME.DRUN_MC: configures sysclk = PLL
       Mode transition: re-enters DRUN which activates PLL=sysclk & peri clks
       */

    initGPIO();         /* Init LED, buttons & vars for Freescale EVB */

    PIT_0.MCR.B.MDIS = 0; /* Enable PIT module. NOTE: PIT module must be       */
                        /* enabled BEFORE writing to it's registers.         */
                        /* Other cores will write to PIT registers so the    */
                        /* PIT is enabled here before starting other cores.  */
    PIT_0.MCR.B.FRZ = 1;  /* Freeze PIT timers in debug mode */

    PIT0_init(40000000); /* Initalize PIT channel 0 for desired SYSCLK counts*/
             /* timeout= 40M  PITclks x 4 sysclks/1 PITclk x 1 sec/160Msysck */
             /*        = 40M x 4 / 160M = 160/160 = 1 sec.  */

	PIT1_init(20000000);
	             /* timeout= 20M  PITclks x 4 sysclks/1 PITclk x 1 sec/160Msysck */
    			/*        = 20M x 4 / 160M = 80/160 = 0.5 sec.  */

	PIT2_init(10000000);
	             /* timeout= 10M  PITclks x 4 sysclks/1 PITclk x 1 sec/160Msysck */
	             /*        = 10M x 4 / 160M = 40/160 = 0.25 sec.  */

    SW_INT_1_init();    /* Initialize SW INT1 (to be serviced by core 1) */

    PIT_0.MCR.B.FRZ = 0; //Unfreeze timers


	for(;;) {	   
		counter++;
	}
	
	return 0;
}

void PIT0_isr(void) {
	static uint8_t counter=0;/* Increment ISR counter */

	counter++;
	LED1 = ~LED1;             /* Toggle LED1 port */
    if(counter == 4)
	{
    	counter = 0;
    	INTC_0.SSCIR[1].B.SET = 1; //Write to set the software interrupt flag, which requests a SW int 1 in INTC
    }
    PIT_0.TIMER[0].TFLG.R |= 1;  /* Clear interrupt flag. w1c */
}

void PIT1_isr(void) {
    LED2 = ~LED2;             /* Toggle LED3 port */
    PIT_0.TIMER[1].TFLG.R |= 1;  /* Clear interrupt flag. w1c */
}

void PIT2_isr(void) {
    LED3 = ~LED3;             /* Toggle LED3 port */
    PIT_0.TIMER[2].TFLG.R |= 1;  /* Clear interrupt flag */
}

void SW_INT_1_init(void) {
	INTC_0.PSR[1].R = 0x800F;
}

void SW_INT_1_isr(void) {
    LED4 = ~LED4;             /* Toggle external LED4  */
    INTC_0.SSCIR[1].B.CLR = 0x01;       /* Clear interrupt flag. w1c */
}
