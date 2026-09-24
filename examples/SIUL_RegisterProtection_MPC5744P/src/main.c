/* main.c - Register Protection example for MPC5744P */
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
* Brief             This example demonstrates write protection of SIUL2 registers
*
********************************************************************************
* Detailed Description:
* In this example, SIUL2 registers are protected.  When protected, all writes
* to protected registers are ignored.  Program then unlocks the SIUL2 registers
* and attempts to write the same values to the previously protected registers.
* Having been unlocked, the write will change the register values.  This code
* example has no terminal connection.  To check whether operation works, the
* flags LockedRegisterUnchanged, UnlockedRegisterChanged,
* and RegisterLockMaintained change from 0 to 1 to represent that the protection
* worked.  Connect USB to PC terminal.  UART will print explanation messages to
* UART terminal.
*
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        19200, 8N1, None
* Fsys:            160 MHz PLL on 40MHz external oscillator
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  USB to PC
*
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Sept-06-2016  David Chung	  	Initial version

*******************************************************************************/

#include "derivative.h" /* include peripheral declarations */
#include "mode_entry.h"
#include "project.h"
#include "uart.h"
#include <string.h>

#define KEY_VALUE1 0x5AF0ul
#define KEY_VALUE2 0xA50Ful

extern void xcptn_xmpl(void);

__attribute__ ((section(".text")))

/****************** Additional Headers for Register Protection ***************/
struct SIUL2_GFCR_tag {
  union {                              /* Global Configuration Register */
    vuint32_t R;
    struct {
      vuint32_t HLB:1;                 /* Hard Lock Bit */
      vuint32_t  :7;
      vuint32_t UAA:1;                 /* User Access Allowed. */
      vuint32_t  :23;
    } B;
  } GCR;
};

struct SIUL2_SSLB_tag {
  union {                              /* Set Soft Lock Bit Registers */
    vuint8_t R;
    struct {
      vuint8_t WE0:1;                  /* Write Enable Bits */
      vuint8_t WE1:1;
      vuint8_t WE2:1;
      vuint8_t WE3:1;
      vuint8_t SLB0:1;                 /* Soft Lock Bits */
      vuint8_t SLB1:1;
      vuint8_t SLB2:1;
      vuint8_t SLB3:1;
    } B;
  } SSLB[1535];
};

#define SIUL2_REGLOCK     (*(volatile struct SIUL2_tag *)        0xFFFC2000UL)
#define SIUL2_LOCKBITS    (*(volatile struct SIUL2_SSLB_tag *)   0xFFFC3800UL)
#define SIUL2_GLOBALLOCK  (*(volatile struct SIUL2_GFCR_tag *)   0xFFFC3FFCUL)

/************************************ Main ***********************************/

int main(void)
{

  uint32_t i = 0;
  uint8_t dummychar = 0;
  uint8_t intro[] = {"\n\rWelcome to the XDEVKIT-MPC5744P SIUL register protection code example!\n\r"};
  uint8_t intro2[] = {"\n\rSIUL2 register lock is achieved by writing to registers in specific memory locations. "
		  "Parallel SIUL2 register types have been declared in main.c and assigned to these specific memory locations "
		  "to simplify writing to these memory locations.  The lock-bits registers have their own unique bit assignments. "
		  "These are also declared in main.c.\n\r"};
  uint8_t continue_message[] = {"\n\rPress any key to continue.\n\r"};

  uint32_t LockedRegisterUnchanged  = 0;  /* test result */
  uint32_t UnlockedRegisterChanged  = 0;  /* test result */
  uint32_t RegisterLockMaintained   = 0;  /* test result */

  xcptn_xmpl ();              /* Configure and Enable Interrupts */

  system160mhz();        /* sysclk=160MHz, dividers configured, mode trans*/

  LINFlexD_1_Init();	/* Initialize LINFlexD_1 to print UART messages */

  TransmitData((const char*)intro,strlen((const char*)intro));
  TransmitData((const char*)intro2,strlen((const char*)intro2));
  TransmitData((const char*)continue_message,strlen((const char*)continue_message));
  ReceiveData((char*)&dummychar);

  /******************************************************/
  /*  SOFT LOCK                                         */
  /******************************************************/
  uint8_t softlock_message[] = {"\n\rThe register SIUL2_IFER0 is softlocked with the value 0x11111111 by"
		  " writing to the register SIUL2_REGLOCK_IFER0. This register mirrors SIUL2_IFER0.\n\r"};
  TransmitData((const char*)softlock_message,strlen((const char*)softlock_message));
  SIUL2_REGLOCK.IFER0.R = 0x11111111;  /* Write to address mirror & set lock */
    /* Next instruction will cause a Machine Check on cut 2 or later sillcon */
  uint8_t softlock_message2[] = {"\n\rAttempt is made to write 0x22222222 to SIUL2_IFER0.\n\r"};
  TransmitData((const char*)softlock_message2,strlen((const char*)softlock_message2));
  SIUL2.IFER0.R = 0x22222222;          /* Attempt to modify register */
    /* Next instruction will cause a Machine Check on cut 2 or later sillcon */
  uint8_t softlock_message3[] = {"\n\rAttempt is also made to write 0x33333333 to SIUL2_REGLOCK_IFER0.\n\r"};
  TransmitData((const char*)softlock_message3,strlen((const char*)softlock_message3));
  SIUL2_REGLOCK.IFER0.R = 0x33333333;  /* Attempt to modify, lock register */
  if (SIUL2.IFER0.R==0x11111111) {     /* Verify locked value was maintained */
    LockedRegisterUnchanged = 1;       /* Register was not modified */
    if(LockedRegisterUnchanged){
    	uint8_t softlock_returnmessage[] = {"\n\rAfter softlock, an attempted write of 0x22222222 to SIUL2_IFER0 and of 0x33333333 "
    			"to SIUL2_REGLOCK_IFER0 do not affect the registers. SIUL2_IFER0 stays 0x11111111.\n\r"};
    	TransmitData((const char*)softlock_returnmessage,strlen((const char*)softlock_returnmessage));
    }
  }

  TransmitData((const char*)continue_message,strlen((const char*)continue_message));
  ReceiveData((char*)&dummychar);

  /******************************************************/
  /*  CLEAR SOFT LOCK                                   */
  /******************************************************/

  uint8_t softlock_message4[] = {"\n\rClear the softlock by writing to SIUL2_LOCKBITS_SSLB14[WE0]. Setting this writes "
		  "to the corresponding SIUL2_LOCKBITS_SSLB14[SLB0] bit. SIUL2_IFER0 has SIUL2 module offset 0x38 (56d). Each "
		  "SIUL2_LOCKBITS_SSLBn register handles 4 SIUL2 registers. So SSLB index is 56/14 = 14, hence SIUL2_LOCKBITS_SSLB14.\n\r"};
  TransmitData((const char*)softlock_message4,strlen((const char*)softlock_message4));
  SIUL2_LOCKBITS.SSLB[14].R = 0xF0; /* Un-Write protect register SIUL2_IFER0 */
                                   /* IFER0 has SIUL2 module offset 0x38 = 56*/
                                   /* Register's SSLB index = 56 bytes/4 = 14*/
  uint8_t softlock_message5[] = {"\n\rWrite the value 0x44444444 directly to SIUL2_IFER0.\n\r"};
  TransmitData((const char*)softlock_message5,strlen((const char*)softlock_message5));
  SIUL2.IFER0.R = 0x44444444;      /* Attempt to modify register */
  if (SIUL2.IFER0.R==0x44444444) { /* Verify unlock succeeded */
    UnlockedRegisterChanged = 1;   /* Register was modified */
    if(UnlockedRegisterChanged){
    	uint8_t softlock_returnmessage2[] = {"\n\rNow that softlock has been cleared, SIUL2_IFER0 is again responsive "
    			"to write requests. SIUL2_IFER0 now contains 0x44444444.\n\r"};
    	TransmitData((const char*)softlock_returnmessage2,strlen((const char*)softlock_returnmessage2));
    }
  }

  TransmitData((const char*)continue_message,strlen((const char*)continue_message));
  ReceiveData((char*)&dummychar);

  /******************************************************/
  /*   WRITE PROTECT A MODULE'S LOCK BITS               */
  /******************************************************/

  uint8_t hardlock_message[] = {"\n\rNow attempt to hardlock SIUL2_IFER0. First softlock SIUL2_IFER0 with "
		  "the value 0x55555555.\n\r"};
  TransmitData((const char*)hardlock_message,strlen((const char*)hardlock_message));
  SIUL2_REGLOCK.IFER0.R = 0x55555555; /* IFER0: soft lock with new value */

  uint8_t hardlock_message2[] = {"\n\rHardlock all register protection registers by setting SIUL2_GLOBALLOCK_GCR[HLB]. "
		  "This bit can only be cleared by a system reset.\n\r"};
  TransmitData((const char*)hardlock_message2,strlen((const char*)hardlock_message2));
  SIUL2_GLOBALLOCK.GCR.B.HLB = 1;  /* Protect all protectable regs. lock bits*/
    /* Next instruction will cause a Machine Check on cut 2 or later sillcon */

  uint8_t hardlock_message3[] = {"\n\rAttempt to clear the softlock on SIUL2_IFER0, then write 0x66666666 directly to "
		  "SIUL2_IFER0 and through SIUL2_REGLOCK_IFER0.\n\r"};
  TransmitData((const char*)hardlock_message3,strlen((const char*)hardlock_message3));
  SIUL2_LOCKBITS.SSLB[14].R = 0xF0;   /* IFERO: Attempt to remove protection -> IVOR1 */
    /* Next instruction will cause a Machine Check on cut 2 or later sillcon */
  SIUL2.IFER0.R = 0x66666666;       /* IFERO: Attempt new value -> IVOR1 */
    /* Next instruction will cause a Machine Check on cut 2 or later sillcon */
  SIUL2_REGLOCK.IFER0.R = 0x66666666; /* IFERO: Attempt new value with lock -> IVOR1 */
  if (SIUL2.IFER0.R==0x55555555) {    /* IFER0: Verify prior value maintained*/
    RegisterLockMaintained = 1;       /* IFER0: retained prior value */
    if(RegisterLockMaintained){
    	uint8_t hardlock_returnmessage[] = {"\n\rSIUL2_IFER0 remains 0x55555555 despite clearing softlock. Register protection hardlock "
    			"is successful.\n\r"};
    	TransmitData((const char*)hardlock_returnmessage,strlen((const char*)hardlock_returnmessage));
    }
  }

  uint8_t end_message[] = {"\n\rEnd of MPC5744P register protection test.\n\r"};
  TransmitData((const char*)end_message,strlen((const char*)end_message));

  while(1){
	  i++;
  }

  return 0;
}
