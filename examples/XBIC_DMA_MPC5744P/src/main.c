/* main.c - XBIC example for MPC5744P */
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
* Brief            	XBIC test for DMA master->SRAM slave
*
********************************************************************************
* Detailed Description:
* Tests the XBAR integrity checker. Configures 200 MHz PLL and DMA. The XBIC is
* configured to inject (i.e. simulate) an error to DMA-SRAM communication.  The
* DMA transfer is then activated and UART terminal reports the error detected.
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        19200, 8N1, None
* Fsys:            200 MHz PLL on 40 MHz external oscillator
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  USB to PC
*
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/
#include "MPC5744P.h"
#include "uart.h"
#include <string.h>

/*******************************************************************************
* Constants and macros
*******************************************************************************/
#define 	DRUN_MODE 	0x3

/*******************************************************************************
* External objects
*******************************************************************************/
extern void xcptn_xmpl(void);

/*******************************************************************************
* Local functions
*******************************************************************************/

/*******************************************************************************
Function Name : Sys_Init
Engineer      : b21190
Date          : Oct-22-2014
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : Clock settings
Issues        : NONE
*******************************************************************************/
void Sys_Init(void)
{
    //Clk Out
    MC_CGM.AC6_SC.R = 0x02000000;           //Enable PLL0 - clkout0 //MC_CGM_AC6_SC
    MC_CGM.AC6_DC0.B.DE = 1;                //Enable divider for SYSCLK0
    MC_CGM.AC6_DC0.B.DIV = 0;               //Set divider for SYSCLK0

    //Configure RunPeripheralConfiguration registers in ME_RUN_PC0
    MC_ME.RUN_PC[0].B.DRUN = 1;             //Enable DRUN mode for all peripherals using ME_RUN_PC[0]

    //Enable external oscilator
    MC_ME.DRUN_MC.B.XOSCON = 1;

    //AUX Clock Selector 3 setup
    MC_CGM.AC3_SC.B.SELCTL =0x01;           //connect (8..40MHz) XTALL to the PLL0 input
    MC_ME.DRUN_MC.B.PLL0ON = 1;             //Enable PLL0 for DRUN mode

    // Set PLL0 to 200MHz
    PLLDIG.PLL0CR.B.CLKCFG = 1;             //Bypass mode PLL0 on
    // RFDPHI1 = 10, RFDPHI = 2, PREDIV = 2, MFD = 14
    PLLDIG.PLL0DV.R = 0x50000000 |0x00020000 |0x00002000 |0x0014 ; //predefined PLL0 divider register

    // Set PPL0 as system clock
    MC_ME.DRUN_MC.B.SYSCLK = 0x2;
    //  Enable system clock divider /4
    MC_CGM.SC_DC0.B.DIV = 0x3;

    /* Enable LINFlexD_1 for all modes. */
    MC_ME.RUN_PC[1].R = 0x000000FE; //Enable RUN_PC[1] for all modes
    MC_ME.PCTL91.B.RUN_CFG = 0x1; //LINFlexD_1: select RUN_PC[1]

    //Mode transition to apply the PLL0 setup and set Normal mode with PLL running
    MC_ME.MCTL.R = 0x30005AF0;              //DRUN Mode & Key
    MC_ME.MCTL.R = 0x3000A50F;              //DRUN Mode & Key

    while(!MC_ME.GS.B.S_PLL0);              //ME_GS Wait for PLL stabilization.
    while(MC_ME.GS.B.S_MTRANS);             //Waiting for end of transaction
    while(MC_ME.GS.B.S_CURRENT_MODE != DRUN_MODE);  // ME_GS Check DRUN mode has successfully been entered
}//Sys_Init

/*******************************************************************************
Function Name : XBIC_error_injection
Engineer      : b21190
Date          : Apr-30-2014
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : Inject the error on XBAR for DMA master.
Issues        : NONE
*******************************************************************************/
void XBIC_error_injection (void)
{
  /* Enable all master and slave ports for EDC error detect.
   * Appears that XBIC uses logical master ID on master side,
   * but physical port number on the slave side.
   */
	XBIC.MCR.B.SE2 = 0x1; //Enable slave physical port 2 (SRAM)
	XBIC.MCR.B.ME2 = 0x1; //Enable master logical ID 2 (DMA)

  /* Configure error injection. */
  XBIC.EIR.R = 0x0000220F; /* SLV = 2 (Error injection enabled for slave port 2 (SRAM)
  	  	  	  	  	  	  	* MST = 2 (Error injection is enabled for master 2 (DMA)
  	  	  	  	  	  	  	* SYN = xF. This valued is XOR'd with the calculated syndrome.
  	  	  	  	  	  	  	*/

  /* Error Injection Enable */
  XBIC.EIR.B.EIE = 0x1;
}//XBIC_error_injection

/*******************************************************************************
Function Name : AIPS_0_Init
Engineer      : b21190
Date          : Feb-19-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : Setup access right for Masters and Peripherals on AIPS
Issues        : NONE
*******************************************************************************/
void AIPS_0_Init(void)
{
    // All Master Can go through AIPS_0 and all peripheral have no protection

    // Master Privilege Register A
    AIPS_0.MPRA.R      = 0x77777777;
    // Peripheral Access Control Register
    AIPS_0.PACR[0].R    = 0x0;
    AIPS_0.PACR[1].R    = 0x0;
    AIPS_0.PACR[2].R    = 0x0;
    AIPS_0.PACR[3].R    = 0x0;
    //AIPS_0.PACR[4].R    = 0x0; - Reserved, not implemented
    AIPS_0.PACR[5].R    = 0x0;
    AIPS_0.PACR[6].R    = 0x0;
    AIPS_0.PACR[7].R    = 0x0;

    // Off-Platform Peripheral Access Control Register
    AIPS_0.OPACR[0].R	= 0x0;
    AIPS_0.OPACR[1].R	= 0x0;
    AIPS_0.OPACR[2].R	= 0x0;
    AIPS_0.OPACR[3].R	= 0x0;
    AIPS_0.OPACR[4].R	= 0x0;
    AIPS_0.OPACR[5].R	= 0x0;
    AIPS_0.OPACR[6].R	= 0x0;
    AIPS_0.OPACR[7].R	= 0x0;
    AIPS_0.OPACR[8].R	= 0x0;
    AIPS_0.OPACR[9].R	= 0x0;
    AIPS_0.OPACR[10].R	= 0x0;
    AIPS_0.OPACR[11].R	= 0x0;
    AIPS_0.OPACR[12].R	= 0x0;
    AIPS_0.OPACR[13].R	= 0x0;
    AIPS_0.OPACR[14].R	= 0x0;
    AIPS_0.OPACR[15].R	= 0x0;
    AIPS_0.OPACR[16].R	= 0x0;
    AIPS_0.OPACR[17].R	= 0x0;
    AIPS_0.OPACR[18].R	= 0x0;
    AIPS_0.OPACR[19].R	= 0x0;
    AIPS_0.OPACR[20].R	= 0x0;
    AIPS_0.OPACR[21].R	= 0x0;
    AIPS_0.OPACR[22].R	= 0x0;
    AIPS_0.OPACR[23].R	= 0x0;
    AIPS_0.OPACR[24].R	= 0x0;
    AIPS_0.OPACR[25].R	= 0x0;
    AIPS_0.OPACR[26].R	= 0x0;
    AIPS_0.OPACR[27].R	= 0x0;
    AIPS_0.OPACR[28].R	= 0x0;
    AIPS_0.OPACR[29].R	= 0x0;
    AIPS_0.OPACR[30].R	= 0x0;
    AIPS_0.OPACR[31].R	= 0x0;
}//AIPS_0_Init

/*******************************************************************************
Function Name : DMA_0_Init
Engineer      : b21190
Date          : Mar-11-2015
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : DMA_0 initialization funtion initialize TCD for simple transfer
                from constant "source" stored in internal flash to start of SRAM
		to demonstrate correct configuration. Used is one major loop.
		DMA is started by SW.
Issues        : NONE
*******************************************************************************/
void DMA_0_Init(void)
{
    // Clear the ENBL and TRIG bits of the DMA channel
    DMAMUX_0.CHCFG[0].B.ENBL = 0;
    DMAMUX_0.CHCFG[0].B.TRIG = 0;

    // DMA_0 config
    DMA_0.ERQ.B.ERQ0 = 0;    //The DMA request signal for channel 0 is disabled.
    //TCD config for channel[0]

    // TCD[0] Word 0 config
    DMA_0.TCD[0].SADDR.R = 0x40000000;  	//Source Address

    // TCD[0] Word 1 config SMOD(0) | SSIZE(32-bit) | DMOD(0) | DSIZE(32-bit)
    DMA_0.TCD[0].ATTR.R = 0x0|0x200|0x0|0x2;	//Source transfer size 32-bit, no Address Modulo used

    // TCD[0] Word 2 config NBYTES - Minor Byte Transfer Count
    // Number of bytes to be transferred in each service request of the channel
    DMA_0.TCD[0].NBYTES.MLNO.R = 0x4;

    // TCD[0] Word 3 config SLAST - TCD Last Source Address Adjustment
    DMA_0.TCD[0].SLAST.R = 0;

    // TCD[0] Word 4 config DADDR - TCD Destination Address
    DMA_0.TCD[0].DADDR.R = 0x40001000;	 //Destination Address

    // TCD[0] Word 5 config CITER - TCD Current Minor Loop Link, Major Loop Count
    // ELINK | CITER
    DMA_0.TCD[0].CITER.ELINKNO.R = 0x0 | 0x1;  //Destination Address

    // TCD[0] Word 6 config DLAST_SGA - TCD Last Destination Address Adjustment/Scatter Gather Address
    DMA_0.TCD[0].DLASTSGA.R = 0x4;	 // Destination last address adjustment

    // TCD[0] Word 7 config BITER - TCD Beginning Minor Loop Link, Major Loop Count
    // ELINK | BITER
    DMA_0.TCD[0].BITER.ELINKNO.R = 0x0 | 0x1;	 // Destination last address adjustment

    DMA_0.SERQ.R = 0x0; //Enable Channel 0
}//DMA_0_Init

/*******************************************************************************
Function Name : HexToASCII_Word
Engineer      : b48683
Date          : Jan-30-2017
Parameters    : NONE
Modifies      : NONE
Returns       : NONE
Notes         : Turns hex word to ASCII and stores in buffer
Issues        : NONE
*******************************************************************************/
void HexToASCII_Word(const unsigned int input,uint8_t* buffer){
	uint8_t i = 0;
	unsigned int number = input;
	unsigned int temp = 0;

	for(i=0;i<8;i++){
		temp = number & 0x0000000F; //Mask least significant nibble of word
		/* Convert to ASCII. Store in buffer backwards because LSB gets stored first */
		if(temp > 9){
			*(buffer+8-(i+1)) = temp + '0' + 7; //'A' does not come immediately after '9' in ASCII table. Additional offset
		}else{
			*(buffer+8-(i+1)) = temp + '0'; //Otherwise store '0' to '9'
		}

		number = number >> 4; //Get next nibble
	}
}

/*******************************************************************************
* Global functions
*******************************************************************************/
int main (void){
	char dummychar = 0;
	uint8_t	intro[] = {"\n\rWelcome to the XDEVKIT-MPC5744P XBIC code example! "
			"XBIC is the crossbar integrity checker, which checks the consistency "
			"between signals on the master side and what comes out on the slave side,"
			"and vice versa.\n\r"};
	uint8_t intro2[] = {"\n\rDMA is configured to transfer PRAM to PRAM."
			" XBIC is configured to inject an error for DMA-PRAM communication. DMA is master 5 (M5) and "
			"PRAM is slave 2 (S2).\n\r"};
	uint8_t Continue_Message[] = {"\n\rPress any key to continue...\n\r"};
	uint8_t Message0[] = {"\n\rDMA transfer of started.\n\r"};
	uint8_t Message1[] = {"\n\rAn error was detected. Syndrome 0x"};
	uint8_t Message2[] = {"\n\rThe master associated with the error is M"};
	uint8_t Message3[] = {"\n\rThe slave associated with the error is S"};
	uint8_t Message4[] = {"\n\rThe error was encountered at address 0x"};
	uint8_t newline[] = {".\n\r"};

	uint8_t PrintBuffer[8] = {0};
	uint8_t master = 0;
	uint8_t slave = 0;
	uint8_t syndrome = 0;
	uint32_t address = 0;

	xcptn_xmpl();
	Sys_Init();
	AIPS_0_Init();
    DMA_0_Init();
    LINFlexD_1_Init(); //Initialize LINFlexD_1 for printing UART messages to computer terminal
    TransmitData((const char*)intro,strlen((const char*)intro));
    TransmitData((const char*)Continue_Message,strlen((const char*)Continue_Message));
    ReceiveData((char*)&dummychar);
    TransmitData((const char*)intro2,strlen((const char*)intro2));

    XBIC_error_injection();

    while(1){
    	TransmitData((const char*)Message0,strlen((const char*)Message0));

    	/* Start DMA */
    	DMA_0.SSRT.R = 0; 	// Start DMA transfer ROM to RAM

    	syndrome = XBIC.ESR.R & 0x000000FF;
    	master = ((XBIC.ESR.R & 0x00000F00) >> 8) + '0'; //Turn into ASCII
    	slave = ((XBIC.ESR.R & 0x00007000) >> 12) + '0'; //Turn into ASCII
    	address = XBIC.EAR.R;

    	/* Print messages. Talk about syndrome. */
    	HexToASCII_Word(syndrome,PrintBuffer);
    	TransmitData((const char*)Message1,strlen((const char*)Message1));
    	TransmitData((const char*)PrintBuffer,8);
    	TransmitData((const char*)newline,strlen((const char*)newline));

    	/* Print message about master */
    	TransmitData((const char*)Message2,strlen((const char*)Message2));
    	TransmitData((const char*)&master,1);
    	TransmitData((const char*)newline,strlen((const char*)newline));

    	/* Print message about slave */
    	TransmitData((const char*)Message3,strlen((const char*)Message3));
    	TransmitData((const char*)&slave,1);
    	TransmitData((const char*)newline,strlen((const char*)newline));

    	/* Print message about error address */
    	HexToASCII_Word(address,PrintBuffer);
    	TransmitData((const char*)Message4,strlen((const char*)Message4));
    	TransmitData((const char*)PrintBuffer,8);
    	TransmitData((const char*)newline,strlen((const char*)newline));

    	/* Await user response before repeat */
    	TransmitData((const char*)Continue_Message,strlen((const char*)Continue_Message));
    	ReceiveData((char*)&dummychar);
    }
    return 0;
}//main
