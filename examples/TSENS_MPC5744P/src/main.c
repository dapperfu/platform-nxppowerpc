/* main.c - TSENS example for MPC5744P */
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
* Brief            	TSENS0/TSENS1 temperature calculation
*                   RUN0 mode with max core frequency(200MHz) generated from PLL1
*
********************************************************************************
* Detailed Description:
* Example shows MCU's temperature measurement with the help of TSENS.
* Calibration constants for TSENS0 and TSENS1 are read from Test Flash and
* ADC0/ADC1 is set to measure Vbg and TSENS outputs.
* Calculated internal temperature can be displayed on the Terminal.
*
* EVB connection:
*
*   LIN1 RX/TX (PD9/PD12) is hardwired to USB. Connect USB to computer and load
*   PC terminal such as TeraTerm.
*
* See results on PC terminal (19200, 8N1, None). You should see following text
* (with different values for sure)
*
* TSENS0/TSENS1 temperature measurement
* press any key to continue...
*
* Calibration constants read from Test Flash
*
* TSENS0                           TSENS1
*
* K1 = 429                         K1 = -220
* K2 = -5785                       K2 = -5767
* K3 = -12800                      K3 = -12736
* K4 = 45                          K4 = 45
*
*      K1 * Vbg_code * 2^-1 + K2 * TSENS_code * 2^3
* T = ---------------------------------------------- / 4 - 273.15 [degC]
*     [K3 * Vbg_code * 2^2 + K4 * TSENS_code] * 2^-10
*
* Vbg0_code   = 1502               Vbg1_code   = 1502
* TSENS0_code = 2002               TSENS1_code = 1988
*
* TSENS0 temp = 34.57 degC         TSENS1 temp = 36.78 degC
*
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        19200, 8N1, None
* Fsys:            200 MHz PLL on 40MHz external oscillator
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  USB from DEVKIT to computer for UART communication
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
int16_t K1_0, K2_0, K3_0, K4_0;
int16_t K1_1, K2_1, K3_1, K4_1;
uint16_t Vbg0_code, Vbg1_code, TSENS0_code, TSENS1_code;

float TSENS0_T, TSENS1_T;

enum{
	uint8,
	int8,
	uint16,
	int16,
	uint32,
	int32,
	uint64,
	int64
}type;

/* ASCII character arrays for printing numbers. Allow for 100 digits (ideally size would be dynamic).
 * on TODO: Make array allocation dynamic.
 */
uint8_t K1_0_ASCII[100] = {0};
uint8_t K1_1_ASCII[100] = {0};
uint8_t K2_0_ASCII[100] = {0};
uint8_t K2_1_ASCII[100] = {0};
uint8_t K3_0_ASCII[100] = {0};
uint8_t K3_1_ASCII[100] = {0};
uint8_t K4_0_ASCII[100] = {0};
uint8_t K4_1_ASCII[100] = {0};
uint8_t Vbg0_code_ASCII[100] = {0};
uint8_t Vbg1_code_ASCII[100] = {0};
uint8_t TSENS0_code_ASCII[100] = {0};
uint8_t TSENS1_code_ASCII[100] = {0};
uint8_t TSENS0_T_ASCII[100] = {0};
uint8_t TSENS1_T_ASCII[100] = {0};

/*******************************************************************************
* Local functions
*******************************************************************************/

/*******************************************************************************
Function Name : Sys_Init
Engineer      : PetrS
Date          : Apr-16-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : Enable XOSC, PLL0, PLL1 and enter RUN0 with PLL1 as sys clk (200 MHz)
Issues        : NONE
*******************************************************************************/
void SysClk_Init(void)
{
    MC_CGM.AC3_SC.B.SELCTL = 0x01;		    //connect XOSC to the PLL0 input
    MC_CGM.AC4_SC.B.SELCTL = 0x01;		    //connect XOSC to the PLL1 input

    // Set PLL0 to 160 MHz with 40MHz XOSC reference
    PLLDIG.PLL0DV.R = 0x28021008;	     // PREDIV =  1, MFD = 8, RFDPHI = 2, RFDPHI1 = 5
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

    MC_CGM.AC0_SC.R = 0x02000000;    // Select PLL0 for auxiliary clock 0
    MC_CGM.AC0_DC0.R = 0x80000000;    // MOTC_CLK : Enable aux clk 0 div by 1 … (160 MHz)
    MC_CGM.AC0_DC2.R = 0x80010000;    // ADC_CLK : Enable aux clk 0 div by 2 … (80 MHz)

    MC_CGM.AC6_SC.R = 0x02000000;    // Select PLL0 for auxiliary clock 6
    MC_CGM.AC6_DC0.R = 0x80090000;    // CLKOUT0 : Enable aux clk 6 div by 10 … (16 MHz)

}

/*******************************************************************************
Function Name : SUIL2_Init
Engineer      : PetrS
Date          : Apr-16-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : SIUL2 initialization (ports)
Issues        : NONE
*******************************************************************************/
void SUIL2_Init(void)
{
    SIUL2.MSCR[22].R = 0x22800001;   	// PB6 as CLK_OUT
    SIUL2.MSCR[43].R = 0x32000000; 	//PC[11] GPIO - Red LED

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
Function Name : ADC_Init
Engineer      : PetrS
Date          : Apr-16-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : ADCs initialization
Issues        : NONE
*******************************************************************************/
static void ADC_Init(void)
{

    ADC_0.MCR.R	= 0x80000000;	// enable result overwriting in scan mode
    ADC_0.CTR1.R = 0x00000016;	// setup conversion time, max value
    ADC_0.NCMR0.R = 0x00008400;	// enable channels 10 (Vbdg), 15 (TSENS0) to be sampled

    ADC_1.MCR.R	= 0x80000000;	// enable result overwriting in scan mode
    ADC_1.CTR1.R = 0x00000016;	// setup conversion time, max value
    ADC_1.NCMR0.R = 0x00008400;	// enable channels 10 (Vbdg), 15 (TSENS1) to be sampled
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
static void ftoa(float number, uint8_t * buf, uint8_t precision){
	//uint8_t * temp; //Temporary ASCII array
	uint8_t temp[100];
	uint32_t integer = (uint32_t)number; //Get the part of the number left of the decimal point
	float decimal_part = number - integer; //To bring up the decimal
	uint32_t decimal = 0; //Decimal part once it is converted to int type
	int32_t index = 0; //For index of temp
	int32_t i = 0; //For index of buf
	int j = 0;

	/* First check the sign bit to see if it is positive or negative. If negative,
	 * add a '-' to the beginning of buf.
	 */
	if(number<0){
		buf[i] = '-';
		i++; //increment the index
	}

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
Function Name : itoa
Engineer      : DavidC
Date          : Apr-5-2016
Parameters    : uint32_t number, uint8_t * buffer, enum type
Modifies      : *buffer
Returns       : NONE
Notes         : Converts integer numbers to ASCII and places it in a buffer
Issues        : NONE
*******************************************************************************/

void itoa(uint32_t number, uint8_t * buffer, uint8_t type){
	uint8_t temp[100] = {0}; //Temporary array. Stores number backwards
	int8_t i=0; //index for buffer
	int8_t j=0; //index for temp

	/* If the number is 0, store a '0' and return */
	if(number == 0){
		buffer[0] = '0';
		return;
	}
	/* Deal with the sign first if it is a signed integer. */
	if((type == int8) || (type == int16) || (type == int32) || (type == int64)){
		/* Check the MSB for the sign. If negative number and smaller than 32-bits, will '1'-extend to MSB. */
		if((number & 1<<31) != 0){
			/* If it is a negative number, add a '-' sign to the beginning of buffer and invert number
			 * so it will be like normal unsigned conversion.
			 */
			buffer[i] = '-';
			i++; //Increment i
			number *= -1; //Invert the number
		}
	}

	/* Convert integer to ASCII, ones place first. */
	while(number != 0){
		temp[j] = (number % 10) + '0'; //Convert the least significant digit to ASCII
		j++; //Increment the index
		/* Divide number by 10 to get the next digit. */
		number = (uint32_t)number/10; //Typecast to uint32_t so it does not become a float
	}

	j--; //Reset index j

	/* The number is not in ASCII, stored backwards in temp. Store in buffer
	 * in correct order.
	 */
	while(j>=0){
		buffer[i] = temp[j];
		i++; //increment i
		j--; //decrement j
	}

}

/*******************************************************************************
* Global functions
*******************************************************************************/
int main (void)
{
    float temp1,temp2;
    uint8_t dummychar = 0;
    uint16_t *testFlashPtr = (uint16_t*)0x04000C0;
	
    xcptn_xmpl();

    SysClk_Init();
    SUIL2_Init();
    PIT_Init();
    ADC_Init();
    //LINFlexD_0_Init();
    LINFlexD_1_Init();

#if USE_TERMINAL
    uint8_t intro0[] = {"\n\r"};
    uint8_t intro1[] = {"\n\rTSENS0/TSENS1 temperature measurement.\n\r"};
    uint8_t intro2[] = {"\n\rPress any key to continue...\r\n"};
    uint8_t intro3[] = {"\n\r"};
    TransmitData((const char*)intro0, strlen((const char*)intro0));
    TransmitData((const char*)intro1, strlen((const char*)intro1));
    TransmitData((const char*)intro2, strlen((const char*)intro2));
    ReceiveData((char*)&dummychar);
    TransmitData((const char*)intro3, strlen((const char*)intro3));
 #endif

    // Read TSENS calibration constants from Test Flash
    K1_0 = *testFlashPtr++;
    K2_0 = *testFlashPtr++;
    K3_0 = *testFlashPtr++;
    K4_0 = *testFlashPtr++;
    K1_1 = *testFlashPtr++;
    K2_1 = *testFlashPtr++;
    K3_1 = *testFlashPtr++;
    K4_1 = *testFlashPtr;

    /* Convert TSENS calibration constants to ASCII for UART display. */
    itoa((uint32_t)K1_0, K1_0_ASCII, int16);
    itoa((uint32_t)K1_1, K1_1_ASCII, int16);
    itoa((uint32_t)K2_0, K2_0_ASCII, int16);
    itoa((uint32_t)K2_1, K2_1_ASCII, int16);
    itoa((uint32_t)K3_0, K3_0_ASCII, int16);
    itoa((uint32_t)K3_1, K3_1_ASCII, int16);
    itoa((uint32_t)K4_0, K4_0_ASCII, int16);
    itoa((uint32_t)K4_1, K4_1_ASCII, int16);


#if USE_TERMINAL
    uint8_t parameter0[] = {"Calibration constants read from Test Flash\n\r"};
    uint8_t parameter1[] = {"\n\r"};
    uint8_t parameter2[] = {"TSENS0 \t\t\t\t TSENS1\n\r"};
    uint8_t parameter3[] = {"\n\r"};

    uint8_t parameter4_0[] = {"K1 = "};
    /* K1_0 */
    uint8_t parameter4_1[] = {"\t\t\t K1 = "};
    /* K1_1 */
    uint8_t parameter4_2[] = {" \n\r"};

    uint8_t parameter5_0[] = {"K2 = "};
    /* K2_0 */
    uint8_t parameter5_1[] = {"\t\t\t K2 = "};
    /* K2_1 */
    uint8_t parameter5_2[] = {" \n\r"};

    uint8_t parameter6_0[] = {"K3 = "};
    /* K3_0 */
    uint8_t parameter6_1[] = {"\t\t\t K3 = "};
    /* K3_1 */
    uint8_t parameter6_2[] = {" \n\r"};

    uint8_t parameter7_0[] = {"K4 = "};
    /* K4_0 */
    uint8_t parameter7_1[] = {"\t\t\t K4 = "};
    /* K4_1 */
    uint8_t parameter7_2[] = {" \n\r"};

    uint8_t parameter8[] = {"\n\r"};
    uint8_t parameter9[] = {"	K1 * Vbg_code * 2^-1 + K2 * TSENS_code * 2^3\n\r"};
    uint8_t parameter10[] = {"T = ---------------------------------------------- / 4 - 273.15 [degC]\n\r"};
    uint8_t parameter11[] = {"    [K3 * Vbg_code * 2^2 + K4 * TSENS_code] * 2^-10\n\r"};
    uint8_t parameter12[] = {"\n\r"};

    TransmitData((const char*)parameter0, strlen((const char*)parameter0));
    TransmitData((const char*)parameter1, strlen((const char*)parameter1));
    TransmitData((const char*)parameter2, strlen((const char*)parameter2));
    TransmitData((const char*)parameter3, strlen((const char*)parameter3));

    TransmitData((const char*)parameter4_0, strlen((const char*)parameter4_0));
    /* %d, K1_0 */
    TransmitData((const char*)K1_0_ASCII, strlen((const char*)K1_0_ASCII));
    TransmitData((const char*)parameter4_1, strlen((const char*)parameter4_1));
    /* %d, K1_1 */
    TransmitData((const char*)K1_1_ASCII, strlen((const char*)K1_1_ASCII));
    TransmitData((const char*)parameter4_2, strlen((const char*)parameter4_2));

    TransmitData((const char*)parameter5_0, strlen((const char*)parameter5_0));
    /* %d, K2_0 */
    TransmitData((const char*)K2_0_ASCII, strlen((const char*)K2_0_ASCII));
    TransmitData((const char*)parameter5_1, strlen((const char*)parameter5_1));
    /* %d, K2_1 */
    TransmitData((const char*)K2_1_ASCII, strlen((const char*)K2_1_ASCII));
    TransmitData((const char*)parameter5_2, strlen((const char*)parameter5_2));

    TransmitData((const char*)parameter6_0, strlen((const char*)parameter6_0));
    /* %d, K3_0 */
    TransmitData((const char*)K3_0_ASCII, strlen((const char*)K3_0_ASCII));
    TransmitData((const char*)parameter6_1, strlen((const char*)parameter6_1));
    /* %d, K3_1 */
    TransmitData((const char*)K3_1_ASCII, strlen((const char*)K3_1_ASCII));
    TransmitData((const char*)parameter6_2, strlen((const char*)parameter6_2));

    TransmitData((const char*)parameter7_0, strlen((const char*)parameter7_0));
    /* %d, K4_0 */
    TransmitData((const char*)K4_0_ASCII, strlen((const char*)K4_0_ASCII));
    TransmitData((const char*)parameter7_1, strlen((const char*)parameter7_1));
    /* %d, K4_1 */
    TransmitData((const char*)K4_1_ASCII, strlen((const char*)K4_1_ASCII));
    TransmitData((const char*)parameter7_2, strlen((const char*)parameter7_2));

    TransmitData((const char*)parameter8, strlen((const char*)parameter8));
    TransmitData((const char*)parameter9, strlen((const char*)parameter9));
    TransmitData((const char*)parameter10, strlen((const char*)parameter10));
    TransmitData((const char*)parameter11, strlen((const char*)parameter11));
    TransmitData((const char*)parameter12, strlen((const char*)parameter12));

 #endif

    while(1)
    {

	// TSEN0 temperature calculation
	ADC_0.MCR.B.NSTART=1;           /* Trigger normal conversions for ADC0 */
        while (ADC_0.CDR[15].B.VALID != 1) {}; /* Wait for last scan to complete */
	Vbg0_code = ADC_0.CDR[10].B.CDATA;
	TSENS0_code = ADC_0.CDR[15].B.CDATA;

	/* Convert Vbg0_code and TSENS0_code to ASCII */
	itoa((uint32_t)Vbg0_code, Vbg0_code_ASCII, uint16);
	itoa((uint32_t)TSENS0_code, TSENS0_code_ASCII, uint16);

	temp1 = K1_0 * Vbg0_code / 2 + K2_0 * TSENS0_code * 8;
	temp2 = (K3_0 * Vbg0_code * 4 + K4_0 * TSENS0_code) * 0.0009765625;

	TSENS0_T = temp1 / temp2 / 4 - 273.15;

	/* Convert TSENS0_T to ASCII with precision 2. */
	ftoa(TSENS0_T, TSENS0_T_ASCII, 2);

	// TSEN1 temperature calculation
	ADC_1.MCR.B.NSTART=1;           /* Trigger normal conversions for ADC1 */
        while (ADC_1.CDR[15].B.VALID != 1) {}; /* Wait for last scan to complete */
	Vbg1_code = ADC_1.CDR[10].B.CDATA;
	TSENS1_code = ADC_1.CDR[15].B.CDATA;

	/* Convert Vbg1_code and TSENS1_code to ASCII */
	itoa((uint32_t)Vbg1_code, Vbg1_code_ASCII, uint16);
	itoa((uint32_t)TSENS1_code, TSENS1_code_ASCII, uint16);

	temp1 = K1_1 * Vbg1_code / 2 + K2_1 * TSENS0_code * 8;
	temp2 = (K3_1 * Vbg1_code * 4 + K4_1 * TSENS0_code) * 0.0009765625;

	TSENS1_T = temp1 / temp2 / 4 - 273.15;

	/* Convert TSENS1_T to ASCII with precision 2. */
	ftoa(TSENS1_T, TSENS1_T_ASCII, 2);

#if USE_TERMINAL

	if(PIT_0.TIMER[0].TFLG.B.TIF)	// put results into terminal each second
	{

		uint8_t message0_0[] = {"Vbg0_code   = "};
		/* Vbg0_code */
		uint8_t message0_1[] = {" \t\t Vbg1_code   = "};
		/* Vbg1_code */
		uint8_t message0_2[] = {"\n\r"};

		uint8_t message1_0[] = {"TSENS0_code = "};
		/* TSENS0_code */
		uint8_t message1_1[] = {" \t\t TSENS1_code = "};
		/* TSENS1_code */
		uint8_t message1_2[] = {"\n\r"};

		uint8_t message2[] = {"\n\r"};

		uint8_t message3_0[] = {"TSENS0 temp = "};
		/* %.2f TSENS0_T */
		uint8_t message3_1[] = {" degC \t TSENS1 temp = "};
		/* %.2f TSENS1_T */
		uint8_t message3_2[] = {" degC\r"};

//		uint8_t message4[] = {"\x1b[3A"};
		uint8_t message4[] = {"\n"};

		TransmitData((const char*)message0_0, strlen((const char*)message0_0));
		/* %d, Vbg0_code */
		TransmitData((const char*)Vbg0_code_ASCII, strlen((const char*)Vbg0_code_ASCII));
		TransmitData((const char*)message0_1, strlen((const char*)message0_1));
		/* %d, Vbg1_code */
		TransmitData((const char*)Vbg1_code_ASCII, strlen((const char*)Vbg1_code_ASCII));
		TransmitData((const char*)message0_2, strlen((const char*)message0_2));

		TransmitData((const char*)message1_0, strlen((const char*)message1_0));
		/* %d, TSENS0_code */
		TransmitData((const char*)TSENS0_code_ASCII, strlen((const char*)TSENS0_code_ASCII));
		TransmitData((const char*)message1_1, strlen((const char*)message1_1));
		/* %d, TSENS1_code */
		TransmitData((const char*)TSENS1_code_ASCII, strlen((const char*)TSENS1_code_ASCII));
		TransmitData((const char*)message1_2, strlen((const char*)message1_2));

		TransmitData((const char*)message2, strlen((const char*)message2));

		TransmitData((const char*)message3_0, strlen((const char*)message3_0));
		/* %.2f, TSENS0_T */
		TransmitData((const char*)TSENS0_T_ASCII, strlen((const char*)TSENS0_T_ASCII));
		TransmitData((const char*)message3_1, strlen((const char*)message3_1));
		/* %.2f, TSENS1_T */
		TransmitData((const char*)TSENS1_T_ASCII, strlen((const char*)TSENS1_T_ASCII));
		TransmitData((const char*)message3_2, strlen((const char*)message3_2));

		TransmitData((const char*)message4, strlen((const char*)message4));

	    PIT_0.TIMER[0].TFLG.B.TIF = 1;     // Clear PIT_0 interrupt flag
	}
#endif


	SIUL2.GPDO[43].R ^= 0x1;

	/* "Press any key to continue..." */
	TransmitData((const char*)intro2, strlen((const char*)intro2));
	ReceiveData((char*)&dummychar);

    }
    return 0;
}//main
