/* main.c - FlexCAN example for MPC5744P */
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
* Brief             MPC5744P sends/receives CAN message "Hello".
********************************************************************************
* Detailed Description:
* MPC5744P sends CAN message "Hello".  The macros DEVKIT and LOOPBACK in
* can.h determine mode of operation of this demo.  DEVKIT = 1 if using XDEVKIT-MPC5744P,
* LOOPBACK = 1 if using loopback function.  DEVKIT MUST use LOOPBACK mode. This code
* example ships configured for DEVKIT and loopback.
*
* CAN message "Hello" is sent from either CAN_2 if using DEVKIT or CAN_0 if using
* MB.  Receiver in loopback mode is same CAN as the transmitting CAN (CAN_2 or CAN_0).  If
* not in loopback mode, CAN_1 is the receiver.  In all cases, the buffer RxDATA gets the
* message "Hello World".  Check the UART terminal to see operation.  The UART will print
* the RxDATA before and after CAN transmission.
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P or MPC5744P 257BGA/144LQFPDS + MPC57XXMB
* MCU:             MPC5744P
* Terminal:        19200, 8N1, None
* Fsys:            160 MHz PLL with 40 MHz crystal reference
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  DEVKIT - USB to PC, loopback only
* 					DS+MB - Connect J19 to PC (will need SCI to USB adapter) if loopback
* 							If non-loopback:
* 								Connect J19 to PC (will need SCI to USB adapter)
* 								CAN0_H (P4.1 MB) to CAN1_H (P5.1 MB), CAN0_L (P4.2 MB) to CAN1_L (P5.2 MB)
* 								Jumper connections: J32_1 to J32_2, J32_3 to J32_4
* 													J23_1 to J23_2, J23_3 to J23_4, J23_5 to J23_6
* 													J37_2 to J37_3
* 													J38_2 to J38_3
* 													J35_1 to J35_2, J35_3 to J35_4
* 													J21_1 to J21_2, J21_3 to J21_4, J21_5 to J21_6
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/

#include "derivative.h" /* include peripheral declarations */
#include "can.h"
#include "uart.h"
#include "mode_entry.h"
#include <string.h>

extern void xcptn_xmpl(void);

void peri_clock_gating(void);

extern uint8_t RxDATA[];

/* If using DEVKIT, only allow loopback mode. If LOOPBACK not set,
 * return compile error.
 */
#if DEVKIT && !LOOPBACK
#error "Loopback mode must be configured if using XDEVKIT-MPC5744P. Either change the macro in can.h or change hardware setup."
#endif

__attribute__ ((section(".text")))
int main(void)
{
	uint8_t transmit_done = 0;
	uint8_t intro[] = {"\n\rWelcome to the MPC5744P FlexCAN code example!\n\r"};
	uint8_t BeforeMessage[] = {"\n\rBefore CAN transmission, "};
	uint8_t AfterMessage[] = {"\n\rAfter CAN transmission, "};
	uint8_t RxMessage[] = {"RxData reads "};
	uint8_t EmptyMessage[] = {"RxDATA is empty.\n\r"};
	uint8_t period[] = {"."};
	uint8_t newline[] = {"\n\r"};
	uint8_t EndMessage[] = {"\n\rMPC5744P CAN transmission test done.\n\r"};
	
	xcptn_xmpl ();              /* Configure and Enable Interrupts */

	uint32_t CAN_msg_count = 0;

	peri_clock_gating();   /* configure gating/enabling peri. clocks for modes*/
	                         /* configuraiton occurs after mode transition */

	system160mhz();        /* sysclk=160MHz, dividers configured, mode trans*/

#if !LOOPBACK
	initCAN_1();             /* Initialize FLEXCAN 1 & one of its buffers for receive if no loopback */
#endif

#if !(DEVKIT)
	initCAN_0();             /* Initialize FlexCAN 0 & one of its buffers for transmit and receive if loopback enabled */
	LINFlexD_0_Init();		/* Initialize LINFlexD_0 to transmit messages to terminal */
#else
	initCAN_2();			/* Initialize FlexCAN 2 & one of its buffers for transmit and receive if loopback enabled  */
	LINFlexD_1_Init();		/* Initialize LINFlexD_1 to transmit messages to terminal */
#endif

	TransmitData((const char*)intro,(uint32_t)strlen((const char*)intro));

	TransmitData((const char*)BeforeMessage,(uint32_t)strlen((const char*)BeforeMessage));
	if(strlen((const char*)RxDATA) == 0){
		TransmitData((const char*)EmptyMessage,(uint32_t)strlen((const char*)EmptyMessage));
	}else{
		TransmitData((const char*)RxMessage,(uint32_t)strlen((const char*)RxMessage));
		TransmitData((const char*)RxDATA,(uint32_t)8);
		TransmitData((const char*)period,(uint32_t)strlen((const char*)period));
		TransmitData((const char*)newline,(uint32_t)strlen((const char*)newline));
	}


	while (1) {

	TransmitMsg();           /* Transmit one message from a FlexCAN 0 buffer */
	ReceiveMsg();            /* Wait for the message to be received at FlexCAN 1 */
	CAN_msg_count++;         /* Increment CAN message counter */

	/* Only print this message once */
	if(!transmit_done){
		TransmitData((const char*)AfterMessage,(uint32_t)strlen((const char*)AfterMessage));
		if(strlen((const char*)RxDATA) == 0){
			TransmitData((const char*)EmptyMessage,(uint32_t)strlen((const char*)EmptyMessage));
		}else{
			TransmitData((const char*)RxMessage,(uint32_t)strlen((const char*)RxMessage));
			TransmitData((const char*)RxDATA,(uint32_t)8);
			TransmitData((const char*)period,(uint32_t)strlen((const char*)period));
			TransmitData((const char*)newline,(uint32_t)strlen((const char*)newline));
		}

		TransmitData((const char*)EndMessage,(uint32_t)strlen((const char*)EndMessage));

		/* Set the flag so it doesn't print again */
		transmit_done = 1;
	}

	SIUL2.MSCR[PC11].B.OBE = 0; //Turn off red LED output

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

#if !(DEVKIT)
  MC_ME.PCTL79.B.RUN_CFG = 0x1; //FlexCAN 0: select peripheral config RUN_PC[1]
  MC_ME.PCTL204.B.RUN_CFG = 0x1; //LINFlexD_0: select peripheral config RUN_PC[1]
#else
  MC_ME.PCTL77.B.RUN_CFG = 0x1; //FlexCAN 2: select peripheral config RUN_PC[1]
  MC_ME.PCTL91.B.RUN_CFG = 0x1; //LINFlexD_1: select peripheral config RUN_PC[1]
#endif
#if !LOOPBACK
  MC_ME.PCTL78.B.RUN_CFG = 0x1; //Repeat for FlexCAN 1 if no loopback feature
#endif
}
