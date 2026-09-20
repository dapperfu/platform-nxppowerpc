/* main.c - SPI-DMA example for MPC5744P */
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
* Brief            	This example makes SPI transfer and then eDMA transfers
* 					SPI buffer to memory
*
********************************************************************************
* Detailed Description:
* Configures PLL to 160 MHz. DMAMux sets eDMA channels to DSPI as source.
* Configure DSPI_1 to master and DSPI_2 to slave.  Transfers a buffer of 256
* bytes through SPI.  DMA transfers the buffer to DSPI_1. DSPI_1
* sends to DSPI_2. DMA transfers the buffer from DSPI_2 to destination buffer.
* Put a watch on TransmitBuffer and ReceiveBuffer. ReceiveBuffer will start with
* all '\0's.  After running, ReceiveBuffer's contents will be 0-255, same as
* TransmitBuffer after truncation of TransmitBuffer from int-size to byte-size.
*
* NOTE: There is a problem with the S32DS variable and memory watch windows.
* ReceiveBuffer will NOT update if you check ReceiveBuffer's contents before
* running program.  If you check ReceiveBuffer before SPI transfers, it will
* be all '\0's and stay all '\0s' even after transfers.  Check ReceiveBuffer
* only after all transfers are done. Connect to PC terminal. UART will print
* the buffer contents before and after SPI transfer.
*
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        19200, 8N1, None
* Fsys:            160 MHz PLL on 40MHz external oscillator
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  USB to PC
* 					DSPI_1 SIN (PA8 - J1_12) to DSPI_2 SOUT (PA1 - J5_3), DSPI_2 SIN (PA2 - J5_5) to DSPI_1 SOUT (PA7 - J1_10)
* 					DSPI_1 CLK (PA6 - J1_8) to DSPI_2 CLK (PA0 - J5_1), DSPI_1 SS (PA5 - J1_6) to DSPI_2 SS (PA3 - J1_2)
*
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/

#include "derivative.h" /* include peripheral declarations */
#include "project.h"
#include "spi_dma.h"
#include "edma.h"
#include "smpu.h"
#include "uart.h"
#include <string.h>

#define KEY_VALUE1 0x5AF0ul
#define KEY_VALUE2 0xA50Ful

extern void xcptn_xmpl(void);
void peri_clock_gating(void);
void bridges_config(void);
void crossbar_config_DMA_highest(void);
void system160mhz(void);

extern const unsigned int TransmitBuffer[];
extern unsigned char ReceiveBuffer[];

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

void PrintInitialBuffers(){
	uint32_t i = 0;
	uint8_t PrintBuffer[8] = {0};
	uint8_t comma[] = {", "};
	uint8_t newline[] = {"\n\r"};
	uint8_t Transmit_Message[] = {"\n\rTransmitBuffer's contents are:\n\r"};
	uint8_t Receive_Message[] = {"\n\rReceiveBuffer's contents are:\n\r"};
	uint8_t Empty_Message[] = {"\n\rReceiveBuffer is empty.\n\r"};
	uint8_t Prefix[] = {"0x"};
	uint8_t Continue_Message[] = {"\n\rPress any key to continue.\n\r"};
	uint8_t dummychar = 0;

	TransmitData((const char*)Transmit_Message,strlen((const char*)Transmit_Message));

	/* In this loop, print all bytes except last one */
	for(i=0;i<(NUMBER_OF_BYTES-1);i++){
		HexToASCII_Word(TransmitBuffer[i],PrintBuffer);
		TransmitData((const char*)Prefix,strlen((const char*)Prefix));
		TransmitData((const char*)PrintBuffer,8);
		TransmitData((const char*)comma,strlen((const char*)comma));
	}

	/* Print the last one with a newline instead of a comma */
	HexToASCII_Word(TransmitBuffer[i],PrintBuffer);
	TransmitData((const char*)Prefix,strlen((const char*)Prefix));
	TransmitData((const char*)PrintBuffer,8);
	TransmitData((const char*)newline,strlen((const char*)newline));
	TransmitData((const char*)Continue_Message,strlen((const char*)Continue_Message));
	ReceiveData((char*)&dummychar);

	if(strlen((const char*)ReceiveBuffer)){
		TransmitData((const char*)Empty_Message,strlen((const char*)Empty_Message));
	}else{
		TransmitData((const char*)Receive_Message,strlen((const char*)Receive_Message));

		/* In this loop, print all bytes except last one */
		for(i=0;i<(NUMBER_OF_BYTES-1);i++){
			HexToASCII_Word(ReceiveBuffer[i],PrintBuffer);
			TransmitData((const char*)Prefix,strlen((const char*)Prefix));
			TransmitData((const char*)PrintBuffer,8);
			TransmitData((const char*)comma,strlen((const char*)comma));
		}

		/* Print the last one with a newline instead of a comma */
		HexToASCII_Word(ReceiveBuffer[i],PrintBuffer);
		TransmitData((const char*)Prefix,strlen((const char*)Prefix));
		TransmitData((const char*)PrintBuffer,8);
		TransmitData((const char*)newline,strlen((const char*)newline));
		TransmitData((const char*)Continue_Message,strlen((const char*)Continue_Message));
		ReceiveData((char*)&dummychar);
	}
}

void PrintTransferredBuffer(){
	uint32_t i = 0;
	uint8_t PrintBuffer[8] = {0};
	uint8_t comma[] = {", "};
	uint8_t newline[] = {"\n\r"};
	uint8_t intro[] = {"\n\rSPI has transferred.\n\r"};
	uint8_t Receive_Message[] = {"\n\rReceiveBuffer's contents are:\n\r"};
	uint8_t Empty_Message[] = {"\n\rReceiveBuffer is empty.\n\r"};
	uint8_t Prefix[] = {"0x"};

	TransmitData((const char*)intro,strlen((const char*)intro));

	if(strlen((const char*)ReceiveBuffer)){
		TransmitData((const char*)Empty_Message,strlen((const char*)Empty_Message));
	}else{
		TransmitData((const char*)Receive_Message,strlen((const char*)Receive_Message));

		/* In this loop, print all bytes except last one */
		for(i=0;i<(NUMBER_OF_BYTES-1);i++){
			HexToASCII_Word(ReceiveBuffer[i],PrintBuffer);
			TransmitData((const char*)Prefix,strlen((const char*)Prefix));
			TransmitData((const char*)PrintBuffer,8);
			TransmitData((const char*)comma,strlen((const char*)comma));
		}

		/* Print the last one with a newline instead of a comma */
		HexToASCII_Word(ReceiveBuffer[i],PrintBuffer);
		TransmitData((const char*)Prefix,strlen((const char*)Prefix));
		TransmitData((const char*)PrintBuffer,8);
		TransmitData((const char*)newline,strlen((const char*)newline));
	}
}

__attribute__ ((section(".text")))
int main(void)
{
	unsigned int i = 0;
	uint8_t intro[] = {"\n\rWelcome to the XDEVKIT-MPC5744P SPI-DMA code example!\n\r"};
	uint8_t End_Message[] = {"\n\rEnd of MPC5744P SPI_DMA test.\n\r"};

	xcptn_xmpl ();              /* Configure and Enable Interrupts */

	smpu_config();				/* Configure memory protection and cache settings for memory regions */

	peri_clock_gating();               /* Config gating/enabling peri. clocks for modes*/
	                                   /* Configuraiton occurs after mode transition */
	system160mhz();                    /* sysclk=160MHz, dividers configured, mode trans */

	bridges_config();                  /* Config PBridge(s) access rights and priorities */
	crossbar_config_DMA_highest();     /* PBridges, flash port 2: DMA highest priority */

	LINFlexD_1_Init();					/* Initialize LINFlexD_1 to transfer UART messages to PC terminal */
	TransmitData((const char*)intro,strlen((const char*)intro));

	/* Print the initial buffers */
	PrintInitialBuffers();

	init_dma_mux();                    /* DMA MUX for DSPI_1 Slave */
	init_edma_tcd_16();                /* DMA Channel for DSPI_1 Master */
	init_edma_tcd_15();                /* DMA Channel for DSPI_2 Slave */
	init_edma_channel_arbitration();   /* DMA Channel priorities */

	init_dspi_1();                     /* Initialize DSPI_1 as master SPI and init CTAR0 */
	init_dspi_2();                      /* Initialize DSPI_2 as Slave SPI and init CTAR0 */
	init_dspi_ports();                  /* DSPI1 Master, DSPI2 Slave */

	SPI_1.MCR.B.HALT = 0x0;           /* Exit HALT mode: go from STOPPED */
	                                   /*  to RUNNING state to start transfers */
	DMA_0.SERQ.R = 15;                  /* Enable EDMA channel 15 DSPI_2  RX */
	DMA_0.SERQ.R = 16;                  /* Enable EDMA channel 16 DSPI_1 TX*/

	while( SPI_1.SR.B.EOQF != 1 ){}   /* Wait until the End Of DSPI Queue: */
	                                   /* All data is transmitted & received by DMA */

	PrintTransferredBuffer();

	TransmitData((const char*)End_Message,strlen((const char*)End_Message));

	while( 1 )
	{
	  i++;
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
  MC_ME.PCTL209.B.RUN_CFG = 0x1;  /* DSPI_2: select peri. cfg. RUN_PC[1] */
  MC_ME.PCTL98.B.RUN_CFG = 0x1;  /* DSPI_1: select peri. cfg. RUN_PC[1] */
  MC_ME.PCTL36.B.RUN_CFG = 0x1; /* DMAMUX_0: select peri. cfg. RUN_PC[1] */
  MC_ME.PCTL146.B.RUN_CFG = 0x1;  /* DMAMUX_1: select peri. cfg. RUN_PC[1] */
  MC_ME.PCTL91.B.RUN_CFG = 0x1;		/* LINFlexD_1: select peri. cfg. RUN_PC[1] */
}

/*****************************************************************************/
/* bridges_config                                                            */
/* Description: Configures bridges to provide desired RW and user/supervisor */
/*              access and priorites by crossbar masters to crossbar slaves. */
/*****************************************************************************/

void bridges_config (void) {
  AIPS_0.MPRA.R |= 0x77777770;       /* All masters have RW & user level access */
  AIPS_1.MPRA.R |= 0x77777770;       /* All masters have RW & user level access */
}

/*****************************************************************************/
/* crossbar_config_DMA_highest                                               */
/* Description: Configures desired crossbar priroities for crossbar slaves   */
/*              to give to crossbar masters.                                 */
/*****************************************************************************/

void crossbar_config_DMA_highest (void) {
  XBAR_0.PORT[4].PRS.R = 0x03000021; /* PBridge 0:    gives highest priority to DMA */
  XBAR_0.PORT[5].PRS.R = 0x03000021; /* PBridge 1:    gives highest priority to DMA */
  XBAR_0.PORT[0].PRS.R = 0x03000021; /* PFLASH CTRL:  gives highest priority to DMA */
}

