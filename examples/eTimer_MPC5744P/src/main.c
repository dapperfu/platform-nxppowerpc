/*
 * main implementation: use this 'C' sample to create your own application
 *
 */
/*******************************************************************************
* NXP Semiconductor Inc.
* (c) Copyright 2015 NXP Semiconductor, Inc.
* ALL RIGHTS RESERVED.
********************************************************************************
Services performed by NXP in this matter are performed AS IS and without
any warranty. CUSTOMER retains the final decision relative to the total design
and functionality of the end product. NXP neither guarantees nor will be
held liable by CUSTOMER for the success of this project.
NXP DISCLAIMS ALL WARRANTIES, EXPRESSED, IMPLIED OR STATUTORY INCLUDING,
BUT NOT LIMITED TO, IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
A PARTICULAR PURPOSE ON ANY HARDWARE, SOFTWARE ORE ADVISE SUPPLIED
TO THE PROJECT BY NXP, AND OR NAY PRODUCT RESULTING FROM NXP
SERVICES. IN NO EVENT SHALL NXP BE LIABLE FOR INCIDENTAL OR CONSEQUENTIAL
DAMAGES ARISING OUT OF THIS AGREEMENT.
CUSTOMER agrees to hold NXP harmless against any and all claims demands
or actions by anyone on account of any damage, or injury, whether commercial,
contractual, or tortuous, rising directly or indirectly as a result
of the advise or assistance supplied CUSTOMER in connection with product,
services or goods supplied under this Agreement.
********************************************************************************
* File:             MPC5744P-ETimerCountMode-S32DS.c
* Owner:            Martin Kovar
* Version:          1.0
* Date:             Jan-12-2016
* Classification:   General Business Information
* Brief:            ETimer count mode demonstration
********************************************************************************
* Detailed Description:
* Application performs basic initialization, setup PLL1 to maximum allowed freq. PLL1 is system frequency,
* PLL0 in initialized to 50MHz
* initializes peripherals clock (MOTC_CLK is set to 5MHz)
* initializes eTimer to count mode
* initializes interrupts, blinking one LED by eTimer interrupt,
*
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             PPC5744PFMMM8 1N65H
* Fsys:            200 MHz
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  none
*
* ------------------------------------------------------------------------------
*
********************************************************************************
Revision History:
Version  Date            Author  			Description of Changes
1.0      Dec-29-2015     Martin Kovar  		Initial Version
*******************************************************************************/
#include "derivative.h" /* include peripheral declarations */


/*******************************************************************************
* Constants and macros
*******************************************************************************/
#define LED1_pin 43


/*******************************************************************************
* Function prototypes
*******************************************************************************/
static void SysClk_Init(void);
static void InitPeriClkGen(void);
static void GPIO_Init(void);
static void HW_Init(void);
static void ETimer_Init(void);
void ETimer_ISR(void);

extern void xcptn_xmpl(void);

/*******************************************************************************
* Local functions
*******************************************************************************/


/*******************************************************************************
Function Name : HW_init
Engineer      : Martin Kovar
Date          : Dec-29-2015
Parameters    :
Modifies      :
Returns       :
Notes         : initialization of the hw for the purposes of this example
Issues        :
*******************************************************************************/
static void HW_Init(void)
{
	xcptn_xmpl ();              /* Configure and Enable Interrupts */
	SysClk_Init();
	InitPeriClkGen();
    GPIO_Init();
}


/*******************************************************************************
Function Name : Sys_Init
Engineer      : Martin Kovar
Date          : Dec-29-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : Enable XOSC, PLL0, PLL1 and enter RUN0 with PLL1 as sys clk (200 MHz)
Issues        : NONE
*******************************************************************************/
// Enable XOSC, PLL0, PLL1 and enter RUN0 with PLL1 as sys clk (200 MHz)
void SysClk_Init(void)
{

	      MC_CGM.AC3_SC.B.SELCTL = 0x01;		    //connect XOSC to the PLL0 input
	      MC_CGM.AC4_SC.B.SELCTL = 0x01;		    //connect XOSC to the PLL1 input

	      // Set PLL0 to 50 MHz with 40MHz XOSC reference
	      PLLDIG.PLL0DV.R = 0x3008100A;	     // PREDIV =  1, MFD = 10, RFDPHI = 8, RFDPHI1 = 6

	      MC_ME.RUN0_MC.R = 0x00130070;		    // RUN0 cfg: IRCON,OSC0ON,PLL0ON,syclk=IRC

	      // Mode Transition to enter RUN0 mode:
	      MC_ME.MCTL.R = 0x40005AF0;		    // Enter RUN0 Mode & Key
	      MC_ME.MCTL.R = 0x4000A50F;		    // Enter RUN0 Mode & Inverted Key
	      while (MC_ME.GS.B.S_MTRANS) {};		    // Wait for mode transition to complete
	      while(MC_ME.GS.B.S_CURRENT_MODE != 4) {};	    // Verify RUN0 is the current mode

	      // Set PLL1 to 200 MHz with 40MHz XOSC reference
	      PLLDIG.PLL1DV.R = 0x00020014;	     // MFD = 20, RFDPHI = 2

	      MC_ME.RUN_PC[0].R = 0x000000FE;		    // enable peripherals run in all modes
	      MC_ME.RUN0_MC.R = 0x001300F4;		    // RUN0 cfg: IRCON, OSC0ON, PLL1ON, syclk=PLL1

	      MC_CGM.SC_DC0.R = 0x80030000;    // PBRIDGE0/PBRIDGE1_CLK at syst clk div by 4 ... (50 MHz)

	      // Mode Transition to enter RUN0 mode:
	      MC_ME.MCTL.R = 0x40005AF0;		    // Enter RUN0 Mode & Key
	      MC_ME.MCTL.R = 0x4000A50F;		    // Enter RUN0 Mode & Inverted Key
	      while (MC_ME.GS.B.S_MTRANS) {};		    // Wait for mode transition to complete
	      while(MC_ME.GS.B.S_CURRENT_MODE != 4) {};	    // Verify RUN0 is the current mode

}


/*******************************************************************************
Function Name : PeriClkGen_init
Engineer      : Martin Kovar
Date          : Jan-5-2016
Parameters    :
Modifies      :
Returns       :
Notes         : - Enable all auxiliary clocks, IMPORTANT - MOTC_CLK is set to 5MHz
Issues        :
*******************************************************************************/
void InitPeriClkGen(void)
{
	      // MC_CGM.SC_DC0.R = 0x80030000;    // PBRIDGE0/PBRIDGE1_CLK at syst clk div by 4 ... (50 MHz)
	      MC_CGM.AC0_SC.R = 0x02000000;    // Select PLL0 for auxiliary clock 0
	      MC_CGM.AC0_DC0.R = 0x80090000;    // MOTC_CLK : Enable aux clk 0 div by 10 … (5 MHz)
	      MC_CGM.AC0_DC1.R = 0x80070000;    // SGEN_CLK : Enable aux clk 0 div by 8 … (6.25 MHz)
	      MC_CGM.AC0_DC2.R = 0x80010000;    // ADC_CLK : Enable aux clk 0 div by 2 … (25 MHz)
	      MC_CGM.AC6_SC.R = 0x04000000;    // Select PLL1 for auxiliary clock 6
	      MC_CGM.AC6_DC0.R = 0x80010000;    // CLKOUT0 : Enable aux clk 6 div by 2 … (100 MHz)
	      MC_CGM.AC10_SC.R = 0x04000000;    // Select PLL1 for auxiliary clock 10
	      MC_CGM.AC10_DC0.R = 0x80030000;    // ENET_CLK : Enable aux clk 10 div by 4 … (50 MHz)
	      MC_CGM.AC11_SC.R = 0x04000000;    // Select PLL1 for auxiliary clock 11
	      MC_CGM.AC11_DC0.R = 0x80030000;    // ENET_TIME_CLK : Enable aux clk 11 div by 4 … (50 MHz)
	      MC_CGM.AC5_SC.R = 0x02000000;    // Select PLL0 for auxiliary clock 5
	      MC_CGM.AC5_DC0.R = 0x80000000;    // LFAST_CLK : Enable aux clk 5 div by 1 … (50 MHz)
	      MC_CGM.AC2_DC0.R = 0x80010000;    // CAN_PLL_CLK : Enable aux clk 2 (PLL0) div by 2 … (25 MHz)
	      MC_CGM.AC1_DC0.R = 0x80010000;    // FRAY_PLL_CLK : Enable aux clk 1 (PLL0) div by 2 … (25 MHz)
	      MC_CGM.AC1_DC1.R = 0x80010000;    // SENT_CLK : Enable aux clk 1 (PLL0) div by 2 … (25 MHz)
}


/*******************************************************************************
Function Name : SUIL2_Init
Engineer      : Martin Kovar
Date          : Dec-29-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : SIUL2 initialization (ports),
Issues        : NONE
*******************************************************************************/
static void GPIO_Init(void)
{
   SIUL2.MSCR[LED1_pin].R = 0x32000000; 	//PC[11] GPIO - Red LED
   SIUL2.GPDO[LED1_pin].R = 1;
}


/*******************************************************************************
Function Name : ETimer_Init
Engineer      : Martin Kovar
Date          : Jan-12-2016
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : ETimer0 channel 0, count mode for delay, Primary source is MOTC_CLK divided by 128
Issues        : NONE
*******************************************************************************/
static void ETimer_Init(void)
{
	    ETIMER_0.ENBL.R = 0x0;			// disable Timer0 channels
	    ETIMER_0.CH[0].CTRL1.R = 0x3F40;	 // Counts only rising edge of the MC_CLK (5MHz in RUN0), divide by 128, count up, count repeatedly, count until compare, then reinitialized
	    ETIMER_0.CH[0].COMP1.R = 0x9896;     // Compare value for 1 second delay (5MHz/128 = 39.063KHz)
	    ETIMER_0.CH[0].INTDMA.R = 0x0002;    // enable interrupt for COMP1
	    ETIMER_0.CH[0].CCCTRL.R = 0x0240;	 // compare on COMP1 when counting up, CAPT1 on rising edge,
	    ETIMER_0.ENBL.R = 0x0003;			// Enable Timer0 channel
}


/*******************************************************************************
Function Name : ETimer_ISR
Engineer      : Martin Kovar
Date          : Jan-12-2016
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : ETimer interrupt routine
Issues        : NONE
*******************************************************************************/
void ETimer_ISR(void)
{
	ETIMER_0.CH[0].STS.R = 0x0002;                     //clear interrupt flag
	SIUL2.GPDO[LED1_pin].R = ~SIUL2.GPDO[LED1_pin].R;  //toggle with pin (LED blinking)
}



__attribute__ ((section(".text")))
int main(void)
{
	int counter = 0;

	HW_Init();
	INTC_0.PSR[611].R = 0x8001;    //set interrupt priority

	ETimer_Init();

	    /* Loop forever */
	while (1)
	{
	    counter++;
	}

	return 0;
}
