/* main.c - eTimer Frequency Measurement example for MPC5744P */
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
* Brief             eTimer pulse/period measurement
                    RUN0 mode with max core frequency(200MHz) generated from PLL0
********************************************************************************
* Detailed Description:
* Example gives possible implementation of input signal period/freq measurement.
* eTimer channel capture 1 and 2 features are used. CAPT1/CAPT2 capture counter
* value on rising/falling edge of input signal. The FIFO is set to 2 entries
* and ICF2 is monitored. Free-running mode is used here.
*
* eTimer channel 0-1 are cascaded to achieve 1sec/1Hz measuring with 32bit counter.

* EVB connection:
*   J5_3  - A[1]  .. eTimer0 channel1 input signal
*   J1_16 - B[6]  .. CLKOUT signal provided by program. You can connect
*   	CLKOUT as input to eTimer0 channel 1 or you can use any external
*   	signal.
*
* Connect pulse signal (J1_16) to eTimer0 channel 1 (J5_3)
* See results on PC terminal such as TeraTerm (19200, 8N1, None).
* Change freq/duty of input signal.
*
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        19200, 8N1, None
* Fsys:            200 MHz PLL with 40 MHz crystal reference
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  Connect XDEVKIT-MPC5744P to computer through USB.
*
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/

/*******************************************************************************
* Includes
*******************************************************************************/
#include "MPC5744P.h"
#include "uart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*******************************************************************************
* Constants and macros
*******************************************************************************/
#define USE_TERMINAL	1


/*******************************************************************************
* External objects
*******************************************************************************/
extern void xcptn_xmpl(void);

/*******************************************************************************
* Global variables
*******************************************************************************/

uint32_t counts, edge1,edge2,edge3,edge4 ;
uint32_t capture_ch0[4],capture_ch1[4];

float freq, period, duty,  pulseH, pulseL;

/*******************************************************************************
* Local functions
*******************************************************************************/

/*******************************************************************************
Function Name : Sys_Init
Engineer      : PetrS
Date          : Mar-20-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : Enable XOSC, PLL0, PLL1 and enter RUN0 with PLL0 as sys clk (200 MHz)
Issues        : NONE
*******************************************************************************/
void SysClk_Init(void)
{
    MC_CGM.AC3_SC.B.SELCTL = 0x01;		    //connect XOSC to the PLL0 input
    MC_CGM.AC4_SC.B.SELCTL = 0x01;		    //connect XOSC to the PLL1 input
    // Set PLL0 to 200 MHz with 40MHz XOSC reference
    PLLDIG.PLL0DV.R = 0x3002100A;	     // PREDIV =  1, MFD = 10, RFDPHI = 2, RFDPHI1 = 6
    MC_ME.RUN0_MC.R = 0x00130070;		    // RUN0 cfg: IRCON,OSC0ON,PLL0ON,syclk=IRC
    // Mode Transition to enter RUN0 mode:
    MC_ME.MCTL.R = 0x40005AF0;		    // Enter RUN0 Mode & Key
    MC_ME.MCTL.R = 0x4000A50F;		    // Enter RUN0 Mode & Inverted Key
    while (MC_ME.GS.B.S_MTRANS) {};		    // Wait for mode transition to complete
    while(MC_ME.GS.B.S_CURRENT_MODE != 4) {};	    // Verify RUN0 is the current mode
    // Set PLL1 to 160 MHz with 40MHz XOSC reference
    PLLDIG.PLL1DV.R = 0x00020010;	     // MFD = 16, RFDPHI = 2
//    MC_ME.RUN_PC[0].R = 0x000000FE;		    // enable peripherals run in all modes
    MC_ME.RUN_PC[1].R = 0x000000FE;		//Enable peripherals following this profile to run in all modes
    MC_ME.PCTL30.B.RUN_CFG = 1;			//PIT_0 to follow RUN_PC[1]
    MC_ME.PCTL36.B.RUN_CFG = 1;			//DMAMUX_0 to follow RUN_PC[1]
    MC_ME.PCTL247.B.RUN_CFG = 1;		//eTimer_0 to follow RUN_PC[1]
    MC_ME.PCTL91.B.RUN_CFG = 1;			//LINFleXD_1 to follow RUN_PC[1]
    MC_ME.RUN0_MC.R = 0x001300F2;		    // RUN0 cfg: IRCON, OSC0ON, PLL1ON, syclk=PLL0
    MC_CGM.SC_DC0.R = 0x80030000;    // PBRIDGE0/PBRIDGE1_CLK at syst clk div by 4 ... (50 MHz)
    // Mode Transition to enter RUN0 mode:
    MC_ME.MCTL.R = 0x40005AF0;		    // Enter RUN0 Mode & Key
    MC_ME.MCTL.R = 0x4000A50F;		    // Enter RUN0 Mode & Inverted Key
    while (MC_ME.GS.B.S_MTRANS) {};		    // Wait for mode transition to complete
    while(MC_ME.GS.B.S_CURRENT_MODE != 4) {};	    // Verify RUN0 is the current mode

    MC_CGM.AC6_SC.R = 0x04000000;    // Select PLL1 for auxiliary clock 6

    /* eTimer input capture seems like cap at 2 MHz.
     * Faster than that the eTimer starts missing edges.
     */
    MC_CGM.AC6_DC0.R = 0x80500000;		//CLKOUT0 : Enable aux clk 6 div by 80 (2 MHz)

    MC_CGM.AC0_SC.R = 0x02000000;    // Select PLL0 for auxiliary clock 0
    MC_CGM.AC0_DC0.R = 0x80010000;    // MOTC_CLK : Enable aux clk 0 div by 2 … (100 MHz)


}

/*******************************************************************************
Function Name : SUIL2_Init
Engineer      : PetrS
Date          : Mar-20-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : SIUL2 initialization (ports)
Issues        : NONE
*******************************************************************************/
void SUIL2_Init(void)
{
    SIUL2.MSCR[22].R = 0x22800001;   	// PB6 as CLK_OUT (on EVB it is B9)
    SIUL2.MSCR[43].R = 0x32000000;		//PC[11] GPIO - Red LED

    SIUL2.MSCR[1].B.IBE = 1;    /* PA1: Enable pad for input - eTimer0 ch1 */
    SIUL2.IMCR[60].B.SSS = 2;   /* eTimer0 ch1: connected to pad PA1 */

}

/*******************************************************************************
Function Name : PIT_Init
Engineer      : PetrS
Date          : Mar-17-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : PIT timer initialization - 1sec, PIT running from 50Mhz
Issues        : NONE
*******************************************************************************/
void PIT_Init(void)
{
    PIT_0.MCR.B.MDIS = 0;               //Enable PIT_0 timers
    PIT_0.TIMER[0].LDVAL.R = 50000000-1;   // setup timer 0 for 1sec period
    PIT_0.TIMER[0].TCTRL.B.TEN = 1;     // start Timer
}

/*******************************************************************************
Function Name : eTimer_Init
Engineer      : PetrS
Date          : Mar-20-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : eTimer0 channel 0, 1 init for capture features
Issues        : NONE
*******************************************************************************/
static void eTimer_Init(void)
{
    ETIMER_0.ENBL.R = 0x0;			// disable Timer0 channels

    /* Configure the channel */
    ETIMER_0.CH[0].CTRL1.R = 0x3801;		// Counts only rising edge of the MC_CLK (100MHz in RUN0), divide by 1, count up, count repeatedly, rollover
    ETIMER_0.CH[0].COMP1.R = 0xFFFF;
    ETIMER_0.CH[0].CCCTRL.R = 0x0264;		// compare on COMP1 when counting up, COMP2 when counting down
    						// CAPT2 on falling edge, CAPT1 on rising edge, 2 entries
    						// free-running mode
    ETIMER_0.CH[0].CTRL3.R	= 1;

    ETIMER_0.CH[1].CTRL1.R = 0xF001;		// cascaded mode, count up, rollover, count repeatedly
    ETIMER_0.CH[1].COMP1.R = 0xFFFF;
    ETIMER_0.CH[1].CCCTRL.R = 0x0264;		// compare on COMP1 when counting up, COMP2 when counting down
    						// CAPT2 on falling edge, CAPT1 on rising edge, 2 entries
    						// free-running mode
    ETIMER_0.CH[1].CTRL3.R	= 1;

    ETIMER_0.ENBL.R = 0x0003;			// Enable Timer0 channel 1

}

/*******************************************************************************
Function Name : ftoa
Engineer      : DavidC
Date          : Mar-31-2016
Parameters    : float number, uint8_t * buffer, uint8_t precision
Modifies      : *buffer
Returns       : NONE
Notes         : Converts floating point numbers to ASCII and places it in a buffer
Issues        : NONE
*******************************************************************************/
void ftoa(float number, uint8_t * buf, uint8_t precision){
	uint8_t temp[100];
	uint32_t integer = (uint32_t)number; //Get the part of the number left of the decimal point
	float decimal_part = number - integer; //To bring up the decimal
	uint32_t decimal = 0; //Decimal part once it is converted to int type
	int32_t index = 0; //For index of temp
	int32_t i = 0; //For index of buf
	int j = 0;
	/* Convert the integer part of the number first. Start with one's place, convert
	 * least significant digit, divide by 10 for next digit and repeat until integer becomes 0
	 * (when truncated to uint32_t. Do at least once because if number is like 0.32, you want that ASCII 0.
	 */
	do{
		*(temp + index) = ((integer % 10) + '0'); //Store last significant digit in temp buffer
		integer = (uint32_t)(integer/10); //Move to the next digit
		index++; //Increment the index of temp
	}while(integer != 0);

	/* Reset index to point at latest character. */
	index--;

	/* Store the integer part in the buf in correct order. Because we started in the one's place,
	 * If the number is 432, it is stored in temp as 234. Store in buf backwards.
	 */
	while(index >= 0){
		buf[i] = temp[index]; //Store in buf in reverse order relative to temp
		i++; //Increment buf index
		index--; //Decrement temp index
	}

	/* Now that we have stored the integer part of the number, we will convert the decimal part of it. */
	index = 0; //Reset temp index because we will need to use it again

	/* Store a decimal point in buf. */
	buf[i] = '.';
	i++; //Increment buf index

	/* Bring the number of digits specified in precision variable left of the decimal piont. */
	for(j = 0; j < precision; j++){
		decimal_part *= 10; //Keep multiplying by 10
	}

	/* Convert to uint32_t type. */
	decimal = (uint32_t) decimal_part;

	/* Convert the decimal part to ASCII. */
	for(index = 0; index<precision; index++){
		temp[index] = (decimal %10) + '0';
		decimal = (uint32_t)(decimal/10);
	}

	/* Reset index to point at latest character. */
	index--;

	/* Store temp contents in buf */
	while(index >= 0){
		buf[i] = temp[index];
		i++;
		index--;
	}

	/* Store a null terminator in the last place of buf. */
	buf[i] = '\0';

}

/*******************************************************************************
* Global functions
*******************************************************************************/
int main (void)
{

	xcptn_xmpl();
    SysClk_Init();
    SUIL2_Init();
    PIT_Init();
    eTimer_Init();
    LINFlexD_1_Init();


#if USE_TERMINAL
    uint8_t intro0[] = {"\f"};
    uint8_t intro1[] = {"eTimer - wide frequency measurement\n\r"};
    uint8_t intro2[] = {"Connect periodic pulse signal to the eTimer0 channel1 input\n\r"};
    uint8_t intro3[] = {"J5_3 on the XDEVKIT-MPC5744P EVB\n\r"};
    uint8_t intro4[] = {"press any key to continue...\r\n"};
    uint8_t intro5[] = {"\f"};
    uint8_t dummychar = 0;
    TransmitData((const char*)intro0, strlen((const char*)intro0));
    TransmitData((const char*)intro1, strlen((const char*)intro1));
    TransmitData((const char*)intro2, strlen((const char*)intro2));
    TransmitData((const char*)intro3, strlen((const char*)intro3));
    TransmitData((const char*)intro4, strlen((const char*)intro4));
    ReceiveData((char*)&dummychar);
    TransmitData((const char*)intro5, strlen((const char*)intro5));
 #endif

    ETIMER_0.CH[0].CCCTRL.B.ARM = 1;		// starts the input capture process
    ETIMER_0.CH[1].CCCTRL.B.ARM = 1;

    while(1){
    	while(!(0x0080 & ETIMER_0.CH[1].STS.R)){}  // wait for channel 1's capture2 flag
    	while(!(0x0080 & ETIMER_0.CH[0].STS.R)){}

    	/* Capture the edges. CAPTn registers are 2-deep FIFOs */
    	capture_ch0[0] = ETIMER_0.CH[0].CAPT1.R; //First rising edge
    	capture_ch0[1] = ETIMER_0.CH[0].CAPT2.R; //First falling edge
    	capture_ch0[2] = ETIMER_0.CH[0].CAPT1.R; //Second rising edge
    	capture_ch0[3] = ETIMER_0.CH[0].CAPT2.R; //Second falling edge

    	capture_ch1[0] = ETIMER_0.CH[1].CAPT1.R; //First rising edge
    	capture_ch1[1] = ETIMER_0.CH[1].CAPT2.R; //First falling edge
    	capture_ch1[2] = ETIMER_0.CH[1].CAPT1.R; //Second rising edge
    	capture_ch1[3] = ETIMER_0.CH[1].CAPT2.R; //Second falling edge

		edge1 = capture_ch1[0]*65536 + capture_ch0[0];	// save 1st rising edge
		edge2 = capture_ch1[1]*65536 + capture_ch0[1];	// save 1st falling edge
		edge3 = capture_ch1[2]*65536 + capture_ch0[2];	// save 2nd rising edge
		edge4 = capture_ch1[3]*65536 + capture_ch0[3];	// save 2nd falling edge

		// calculate period, pulseH, pulseL, freq and duty
		if(edge3>edge1){
			counts = edge3 - edge1;
		}else{
			counts = (0xFFFFFFFF - edge1 +1) + edge3;
		}

	
		freq = (float)100000000.0/counts;
		period = (counts / (float)100000.0)*1000000;

		if(edge2>edge1){
			counts = edge2 - edge1;
		}else{
			counts = (0xFFFFFFFF - edge1 +1) + edge2;
		}

		pulseH = (counts / (float)100000.0)*1000000;
		pulseL = period-pulseH;

		duty = pulseH/period*100;

#if USE_TERMINAL

		if(PIT_0.TIMER[0].TFLG.B.TIF){			// put results into terminal each second

			uint8_t message0_0[] = {"Period = "};
			//uint8_t * period_ascii;
			uint8_t period_ascii[100];
			uint8_t message0_1[] = {" ns\n\r"};
			uint8_t message1_0[] = {"Freq = "};
			//uint8_t * freq_ascii;
			uint8_t freq_ascii[100];
			uint8_t message1_1[] = {"Hz\n\n\r"};
			uint8_t message2_0[] = {"PulseH = "};
			//uint8_t * pulseH_ascii;
			uint8_t pulseH_ascii[100];
			uint8_t message2_1[] = {" ns\n\r"};
			uint8_t message3_0[] = {"PulseL = "};
			//uint8_t * pulseL_ascii;
			uint8_t pulseL_ascii[100];
			uint8_t message3_1[] = {" ns\n\r"};
			uint8_t message4_0[] = {"Duty = "};
			//uint8_t * duty_ascii;
			uint8_t duty_ascii[100];
			uint8_t message4_1[] = {"%\n\r"};
			uint8_t message5[] = {"\n\r"};
			uint8_t message6[] = {"\f"};

			/* Transmit the messages. S32DS lacks printf/sprintf functionality. */
			/* "Period = %f ms\n\r", period */
			ftoa(period, period_ascii, 2);
			TransmitData((const char*)message0_0, strlen((const char*)message0_0));
			TransmitData((const char*)period_ascii, strlen((const char*)period_ascii));
			TransmitData((const char*)message0_1, strlen((const char*)message0_1));

			/* "Freq = %.4f Hz\n\n\r", freq */
			ftoa(freq, freq_ascii, 4);
			TransmitData((const char*)message1_0, strlen((const char*)message1_0));
			TransmitData((const char*)freq_ascii, strlen((const char*)freq_ascii));
			TransmitData((const char*)message1_1, strlen((const char*)message1_1));

			/* "PulseH = %f ms\n\r", pulseH */
			ftoa(pulseH, pulseH_ascii, 2);
			TransmitData((const char*)message2_0, strlen((const char*)message2_0));
			TransmitData((const char*)pulseH_ascii, strlen((const char*)pulseH_ascii));
			TransmitData((const char*)message2_1, strlen((const char*)message2_1));

			/* "PulseL = %f ms\n\r", pulseL */
			ftoa(pulseL, pulseL_ascii, 2);
			TransmitData((const char*)message3_0, strlen((const char*)message3_0));
			TransmitData((const char*)pulseL_ascii, strlen((const char*)pulseL_ascii));
			TransmitData((const char*)message3_1, strlen((const char*)message3_1));

			/* Duty = %.1f %%\n\r", duty */
			ftoa(duty, duty_ascii, 1);
			TransmitData((const char*)message4_0, strlen((const char*)message4_0));
			TransmitData((const char*)duty_ascii, strlen((const char*)duty_ascii));
			TransmitData((const char*)message4_1, strlen((const char*)message4_1));

			/* "\n\r" */
			TransmitData((const char*)message5, strlen((const char*)message5));

			/* "\f" */
			TransmitData((const char*)message6, strlen((const char*)message6));

			PIT_0.TIMER[0].TFLG.B.TIF = 1;     // Clear PIT_0 interrupt flag
		}
#endif

	    ETIMER_0.CH[1].STS.R = 0x00C0;		// clear eTimer0 channel 1's capture1/2 flags
	    ETIMER_0.CH[0].STS.R = 0x00C0;		// clear eTimer0 channel 0's capture1/2 flags

		SIUL2.GPDO[43].R ^= 0x1; //Toggle PC11

		TransmitData((const char*)intro4, strlen((const char*)intro4));
		ReceiveData((char*)&dummychar);
		TransmitData((const char*)intro5, strlen((const char*)intro5));
    }

    return 0;
}//main
