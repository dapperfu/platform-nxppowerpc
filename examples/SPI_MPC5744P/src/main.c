/* main.c - SPI example for MPC5744P */
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
* Brief            	DSPI_1 master sends message to DSPI_2 slave
*
********************************************************************************
* Detailed Description:
* Configures PLL to 160 MHz, DSPI_1 to master, DSPI_2 to slave.  Slave sends 0x1234 to master.
* Master sends 0x5678 to Slave. Slave receive buffer (RecDataSlave) gets 0x5678 and Master receive
* buffer (RecDataMaster) gets 0x1234. Put a watch on RecDataMaster and RecDataSlave. They will start
* at 0. Run the program and then pause.  RecDataSlave will be 0x5678 and RecDataMaster will be 0x1234.
* The UART terminal will print the contents of the master and slave buffers before and after SPI transfer.
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
#include "spi.h"
#include "mode_entry.h"
#include "uart.h"
#include "smpu.h"
#include <string.h>

#define KEY_VALUE1 0x5AF0ul
#define KEY_VALUE2 0xA50Ful

extern void xcptn_xmpl(void);

void peri_clock_gating(void);
void HexToASCII_Word(unsigned int input,uint8_t* buffer);

extern unsigned int RecDataMaster;
extern unsigned int RecDataSlave;

__attribute__ ((section(".text")))
int main(void)
{
	unsigned int i = 0;

	uint8_t intro[] = {"\n\rWelcome to the XDEVKIT-MPC5744P SPI transfer code example!\n\r"};
	uint8_t Before_Message[] = {"\n\rSPI has not yet transferred. SPI slave (SPI_2) sends 0x00001234 and "
			"SPI master (SPI_1) sends 0x00005678\n\r"};
	uint8_t After_Message[] = {"\n\rSPI transfer has happened.\n\r"};
	uint8_t Master_Message[] = {"\n\rRecDataMaster is 0x"};
	uint8_t Slave_Message[] = {"\n\rRecDataSlave is 0x"};
	uint8_t newline[] = {".\n\r"};
	uint8_t PrintBuffer[8] = {0};
	uint8_t Continue_Message[] = {"\n\rPress any key to continue.\n\r"};
	uint8_t dummychar = 0;

	xcptn_xmpl ();              /* Configure and Enable Interrupts */

	smpu_config();				/* Configure memory and cache settings for memory regions */

    peri_clock_gating();                      /* Config gating/enabling peri. clocks for modes*/
	                                            /* Configuraiton occurs after mode transition */
	system160mhz();                           /* sysclk=160MHz, dividers configured, mode trans*/

	LINFlexD_1_Init();						/* Initialize LINFlexD_1 to print UART messages */

	init_dspi_ports();                         /* DSPI1 Master, DSPI_2 Slave */
	init_dspi_1();                            /* Initialize DSPI_1 as master SPI and init CTAR0 */
	init_dspi_2();                             /* Initialize DSPI_2 as Slave SPI and init CTAR0 */

	/* Print initial UART messages */
	TransmitData((const char*)intro,strlen((const char*)intro));
	TransmitData((const char*)Before_Message,strlen((const char*)Before_Message));
	TransmitData((const char*)Master_Message,strlen((const char*)Master_Message));
	HexToASCII_Word(RecDataMaster,PrintBuffer); //Convert RecDataMaster to ASCII
	TransmitData((const char*)PrintBuffer,8);
	TransmitData((const char*)newline,strlen((const char*)newline));
	TransmitData((const char*)Slave_Message,strlen((const char*)Slave_Message));
	HexToASCII_Word(RecDataSlave,PrintBuffer); //Convert RecDataSlave to ASCII
	TransmitData((const char*)PrintBuffer,8);
	TransmitData((const char*)newline,strlen((const char*)newline));
	TransmitData((const char*)Continue_Message,strlen((const char*)Continue_Message));
	ReceiveData((char*)&dummychar);

	TransmitData((const char*)After_Message,strlen((const char*)After_Message));

	while( 1 )
	{
	  SPI_2.PUSHR.PUSHR.R = 0x00001234;       /* Initialize slave DSPI_2's response to master */
	  SPI_1.PUSHR.PUSHR.R  = 0x08015678;     /* Transmit data from master to slave SPI with EOQ */
	  read_data_DSPI_2();                      /* Read data on slave DSPI */
	  read_data_DSPI_1();                     /* Read data on master DSPI */

	  TransmitData((const char*)Master_Message,strlen((const char*)Master_Message));
	  HexToASCII_Word(RecDataMaster,PrintBuffer); //Convert RecDataMaster to ASCII
	  TransmitData((const char*)PrintBuffer,8);
	  TransmitData((const char*)newline,strlen((const char*)newline));
	  TransmitData((const char*)Slave_Message,strlen((const char*)Slave_Message));
	  HexToASCII_Word(RecDataSlave,PrintBuffer); //Convert RecDataSlave to ASCII
	  TransmitData((const char*)PrintBuffer,8);
	  TransmitData((const char*)newline,strlen((const char*)newline));
	  TransmitData((const char*)Continue_Message,strlen((const char*)Continue_Message));
	  ReceiveData((char*)&dummychar);

	  i++;
	}
	return 0;
}

/****************************************************************************/
/* HexToASCII_Word                                                        	*/
/* Description: Converts an unsigned int to an array of 8 ASCII characters	*/
/****************************************************************************/
void HexToASCII_Word(unsigned int input,uint8_t* buffer){
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

/*****************************************************************************/
/* peri_clock_gating                                                         */
/* Description: Configures enabling clocks to peri modules or gating them off*/
/*              Default PCTL[RUN_CFG]=0, so by default RUN_PC[0] is selected.*/
/*              RUN_PC[0] is configured here to gate off all clocks.         */
/*****************************************************************************/

void peri_clock_gating (void)
{
  MC_ME.RUN_PC[0].R = 0x00000000;  /* gate off clock for all RUN modes */
  MC_ME.RUN_PC[1].R = 0x000000FE;  /* config. peri clock for all RUN modes */
  MC_ME.PCTL98.B.RUN_CFG = 0x1;  /* DSPI_1: select peri. cfg. RUN_PC[1] */
  MC_ME.PCTL209.B.RUN_CFG = 0x1;  /* DSPI_2: select peri. cfg. RUN_PC[1] */
  MC_ME.PCTL91.B.RUN_CFG = 0x1;		/* LINFlexD_1: select peri. cfg. RUN_PC[1] */
}
