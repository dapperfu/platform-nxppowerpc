/* main.c - eDMA Transfer example for MPC5744P */
/* Description:  Makes eDMA transfers of data from Source to Destination buffers */
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
* Brief             Tests MPC5744P eDMA. Transfers data from one buffer in flash
* 					to another buffer in flash.
********************************************************************************
* Detailed Description:
* Application initializes eDMA and automatically starts transfers of "Hello Word"
* from source buffer to destination buffer. Add a watch on TCD0_SourceData and
* TCD0_Destination by entering the variable names in the Expressions window of
* Debug view.  Click dropdown menu on TCD0_SourceData and TCD0_Destination to
* reveal contents of the arrays.  TCD0_SourceData will start with "Hello World"
* and TCD0_Destination will start with "\0".  Run until Red LED turns on
* Pause execution and check the buffers again.  TCD0_Destination will change to
* "Hello World".  The UART will print the contents of TCD0_Destination before
* and after eDMA transmission.
*
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        19200, 8N1, None
* Fsys:            160 MHz
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  Connect XDEVKIT-MPC5744P to computer through USB.
*
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/
#include "derivative.h" /* include peripheral declarations */
#include "project.h"
#include "mode_entry.h"
#include "edma.h"
#include "smpu.h"
#include "uart.h"
#include <string.h>

#define KEY_VALUE1 0x5AF0ul
#define KEY_VALUE2 0xA50Ful

extern void xcptn_xmpl(void);
extern void initTCDs(void);

extern uint8_t TCD0_SourceData[];
extern uint8_t TCD0_Destination[];

/*****************************************************************************/
/* bridges_config                                                            */
/* Description: Configures bridges to provide desired RWX and user/supervisor*/
/*              access and priorites by crossbar masters to crossbar slaves. */
/*****************************************************************************/

void bridges_config (void) {
       AIPS_0.MPRA.R |= 0x00700000;   /* Enable DMA RWU for PBRIDGE A. Logical Master ID found in Chip Configuration chapter. */
}

__attribute__ ((section(".text")))
/************************************ Main ***********************************/
int main(){
  volatile uint32_t i = 0;              /* Dummy idle counter */
  uint8_t intro[] = {"\n\rWelcome to the XDEVKIT-MPC5744P eDMA code example!\n\r"};
  uint8_t BeforeMessage[] = {"\n\rBefore eDMA transmission, "};
  uint8_t AfterMessage[] = {"\n\rAfter eDMA transmission, "};
  uint8_t SourceMessage[] = {"TCD0_SourceData reads "};
  uint8_t EmptyMessage[] = {"TCD0_Destination is empty."};
  uint8_t DestinationMessage[] = {"TCD0_Destination reads "};
  uint8_t EndMessage[] = {"\n\rEnd of MPC5744P eDMA test.\n\r"};
  uint8_t period[] = {"."};
  uint8_t newline[] = {"\n\r"};

  xcptn_xmpl ();              /* Configure and Enable Interrupts */

  bridges_config();       /* Enable R/W to peripherals by DMA & all masters*/
  smpu_config();          /* Cache inhibit a RAM region for shared data */
  system160mhz();         /* sysclk=160MHz, dividers configured, mode trans*/

  SIUL2.MSCR[PC11].B.OBE = 1;	/* Pad PC11: Enable output on PC11. */
  SIUL2.GPDO[PC11].R = 1;		/* Pad PC11: Output=1. Red LED. */

  init_edma_channel_arbitration(); /* Initialze arbitration among channels */
  initTCDs();             /* Initialize DMA Transfer Control Descriptors */

  LINFlexD_1_Init();		/* Initialize LINFlexD_1 for UART transmission */

  /* Print UART messages */
  TransmitData((const char*)intro,(uint32_t)strlen((const char*)intro));
  TransmitData((const char*)BeforeMessage,(uint32_t)strlen((const char*)BeforeMessage));
  TransmitData((const char*)SourceMessage,(uint32_t)strlen((const char*)SourceMessage));
  TransmitData((const char*)TCD0_SourceData,(uint32_t)strlen((const char*)TCD0_SourceData));
  TransmitData((const char*)newline,(uint32_t)strlen((const char*)newline));

  if(strlen((const char*)TCD0_Destination) == 0){
	  TransmitData((const char*)EmptyMessage,(uint32_t)strlen((const char*)EmptyMessage));
  }else{
	  TransmitData((const char*)DestinationMessage,(uint32_t)strlen((const char*)DestinationMessage));
	  TransmitData((const char*)TCD0_Destination,(uint32_t)13);
	  TransmitData((const char*)period,(uint32_t)strlen((const char*)period));
	  TransmitData((const char*)newline,(uint32_t)strlen((const char*)newline));
  }


  DMA_0.SERQ.R = 0;        /* Enable EDMA channel 0 */
                          /* Initiate DMA service using software activation: */

  DMA_0.SSRT.R = 0;        /* Set chan 0 START bit to initiate 1st minor loop */
  while (DMA_0.TCD[0].CITER.ELINKNO.B.CITER != 1) {
                          /* while CITER != 1 (not on last minor loop), */
                          /* wait for START=0 and ACTIVE=0 */
    while ((DMA_0.TCD[0].CSR.B.START == 1) | (DMA_0.TCD[0].CSR.B.ACTIVE == 1)) {}
    DMA_0.SSRT.R = 0;     /* Set chan 0 START bit again for next minor loop */
  }

  DMA_0.SSRT.R = 1;        /* Set chan 1 START bit to initiate 1st minor loop */
  while (DMA_0.TCD[1].CITER.ELINKNO.B.CITER != 1) {
                          /* while CITER != 1 (not on last minor loop), */
                          /* wait for START=0 and ACTIVE=0 */
    while ((DMA_0.TCD[1].CSR.B.START == 1) | (DMA_0.TCD[1].CSR.B.ACTIVE == 1)) {}
    DMA_0.SSRT.R = 1;     /* Set chan 0 START bit again for next minor loop */
  }

  /* Transmit UART messages */
  TransmitData((const char*)AfterMessage,(uint32_t)strlen((const char*)AfterMessage));
  TransmitData((const char*)SourceMessage,(uint32_t)strlen((const char*)SourceMessage));
  TransmitData((const char*)TCD0_SourceData,(uint32_t)strlen((const char*)TCD0_SourceData));
  TransmitData((const char*)newline,(uint32_t)strlen((const char*)newline));

  if(strlen((const char*)TCD0_Destination) == 0){
	  TransmitData((const char*)EmptyMessage,(uint32_t)strlen((const char*)EmptyMessage));
  }else{
	  TransmitData((const char*)DestinationMessage,(uint32_t)strlen((const char*)DestinationMessage));
	  TransmitData((const char*)TCD0_Destination,(uint32_t)13);
	  TransmitData((const char*)period,(uint32_t)strlen((const char*)period));
	  TransmitData((const char*)newline,(uint32_t)strlen((const char*)newline));
  }

  TransmitData((const char*)EndMessage,(uint32_t)strlen((const char*)EndMessage));


  while (1) {i++;}
  return 0;

}
