/* main.c - FCCU Simulation for MPC5744P */
/* Description:  Measures eTimer pulse/period measurement */
/* Rev 0 Feb 28 2017 D Chung - initial version */
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
* Date              Feb-28-2017
* Classification    General Business Information
* Brief            	This example simulates faults with the FCCU
*
********************************************************************************
* Detailed Description:
* This code example simulates faults with the FCCU. Press SW1 to simulate the fault.
* When FAULT status is detected, the MCU system moves into SAFE mode and flashes the
* red LED.  Press SW2 to clear the fault. Red LED will turn back off.
*
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             MPC5744P
* Terminal:        None
* Fsys:            160 MHz PLL on 40 MHz XOSC
* Debugger:        OpenSDA
* Target:          FLASH
* EVB connection:  None
*
********************************************************************************
Revision History:
Version  Date         Author  			Description of Changes
1.0      Feb-28-2017  David Chung	  	Initial version

*******************************************************************************/

#include "derivative.h" /* include peripheral declarations */

#define COUNTERMAX 50000

extern void xcptn_xmpl(void);

void CLK_Init(void);

void SIUL_Init(void);

void FCCU_Init(void);

void SIUL_ISR(void);

void Alarm_ISR(void);

uint32_t mode_old = 0;


void CLK_Init(){
	MC_CGM.AC3_SC.R = 0x01000000; //Select XOSC as source of PLL

	/* PLL0_PHI = XOSC_freq * MFD/(prediv * rfdphi) */
	PLLDIG.PLL0DV.R = 0x00021008; //Multiply 40 MHz by 8 and divide by 2

	/* Turn on PLL0 and XOSC and switch system clock to PLL0_PHI */
	MC_ME.DRUN_MC.R = 0x00130072;

	/* Switch takes effect after mode transition.
	 * Mode can switch to itself
	 * Transition from DRUN to DRUN.
	 */
	MC_ME.MCTL.R = 0x30005AF0; //DRUN and key
	MC_ME.MCTL.R = 0x3000A50F; //DRUN and inverted key

	/* Wait until mode transition has completed */
	while(MC_ME.GS.R & 0x08000000);

	/* eTimer runs on PBRIDGEx_CLK, max 50MHz
	 * and MOTC_CLK, max 160MHz
	 * 160MHz/4 = 40MHz
	 */
	MC_CGM.SC_DC0.R = 0x80030000; //Enable system clock divide Divide value is 3 because formula is Actual Divider = divide value + 1

	/* FCCU does not require gating. Runs on PBRIDGE1_CLK and IRC.
	 * Configure SAFE mode settings for when FCCU enters FAULT state,
	 * when SW transitions the system to SAFE mode
	 */
	MC_ME.SAFE_MC.R = 0x00130010; //Disable safegating of pins in SAFE mode so normal pad operation

	/* Configure RUN0 to same as DRUN */
	MC_ME.RUN0_MC.R = 0x00130072;

	/* Change mode from DRUN to RUN0
	 * to make change take effect.
	 */
	MC_ME.MCTL.R = 0x40005AF0; //RUN0 and key
	MC_ME.MCTL.R = 0x4000A50F; //RUN0 and inverted key

	/* Wait until mode transition has completed */
	while(MC_ME.GS.R & 0x08000000);

}

void SIUL_Init(){
	/* Configure PF12 (SW1) for input.
	 * Input is SIUL2 (i.e. GPIO) external interrupt 30.
	 * Used in this application to trigger FCCU fault simulation
	 */
	SIUL2.MSCR[92].B.IBE = 1; //Enable input buffer
	SIUL2.IMCR[203].B.SSS = 1; //Input muxing register. Route SIUL2 External Input Interrupt 30 to PF12

	/* Configure PF13 (SW2) for input.
	 * Input is SIUL2 (i.e. GPIO) external interrupt 31.
	 * Used in this application to clear FCCU simulated fault
	 */
	SIUL2.MSCR[93].B.IBE = 1; //Enable input buffer
	SIUL2.IMCR[204].B.SSS = 1; //Route SIUL2 external interrupt to PF13

	/* Configure PC11, red RGB to GPIO. When FCCU enters ALARM state,
	 * PC11 will turn on.
	 */
	SIUL2.MSCR[43].B.OBE = 1; //Enable output buffer
	SIUL2.GPDO[43].R = 1; //Active low. Turn red RGB off
}



void FCCU_Init(){
	/* Clear all status bits so configuration does
	 * not cause an interrupt to instantly fire.
	 */
    FCCU.NCFK.R =  0xAB3498FE;
    FCCU.NCF_S[0].R = 0xFFFFFFFF;
    while (FCCU.CTRL.B.OPS != 0x3);

    FCCU.NCFK.R =  0xAB3498FE;
    FCCU.NCF_S[1].R = 0xFFFFFFFF;
    while (FCCU.CTRL.B.OPS != 0x3);

    FCCU.NCFK.R =  0xAB3498FE;
    FCCU.NCF_S[2].R = 0xFFFFFFFF;
    while (FCCU.CTRL.B.OPS != 0x3);

	/* Configure FCCU by entering CONFIG mode.
	 * NORMAL -> CONFIG. Do so by writing 0x913756AF
	 * to FCCU_CTRLK and then writing FCCU_CTRL[OPR] = 1
	 */
    FCCU.TRANS_LOCK.R = 0xBC;		//unlock configuration
	FCCU.CTRLK.R = 0x913756AF;
	FCCU.CTRL.R = 0x1;

	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful

	/* By default, FCCU CONFIG mode timer gives user
	 * 4.096ms to configure FCCU. FCCU CONFIG timer is configurable
	 * in FCCU_CFG_TO[TO]. Options are integer multiples of 64us, up to 8.492ms
	 * Use (non-critical fault) NCF[7] which
	 * maps to STCU2 fault condition, "fake"-able.
	 */
	FCCU.NCF_CFG[0].R = 0x00000080; //Enable NCF[7] as a SW recoverable fault
	FCCU.NCFS_CFG[0].R = 0x00000000; //No reset reaction, short or long
	//FCCU.NCFS_CFG[2].R = 0x00000010; //Short functional reset for NCF[34]

	/* CONFIG state only available for a short time. Switch back to
	 * NORMAL state.  Then switch back to CONFIG for next step
	 * for CONFIG.
	 */
	FCCU.CTRLK.R = 0x825A132B; //Key for OP2
	FCCU.CTRL.R = 0x2;
	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful

	/* Switch back to CONFIG */
    FCCU.TRANS_LOCK.R = 0xBC;		//unlock configuration
	FCCU.CTRLK.R = 0x913756AF;
	FCCU.CTRL.R = 0x1;

	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful

	/* Enable NCF[7] */
	FCCU.NCF_E[0].R = 0x00000080;

	/* CONFIG state only available for a short time. Switch back to
	 * NORMAL state.  Then switch back to CONFIG for next step
	 * for CONFIG.
	 */
	FCCU.CTRLK.R = 0x825A132B; //Key for OP2
	FCCU.CTRL.R = 0x2;
	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful

	/* Switch back to CONFIG */
    FCCU.TRANS_LOCK.R = 0xBC;		//unlock configuration
	FCCU.CTRLK.R = 0x913756AF;
	FCCU.CTRL.R = 0x1;

	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful

	/* Enable NCF[7] timeout. When NCF[7] occurs
	 * FCCU will transition to ALARM mode and run
	 * the NCF timer.  When NCF timer runs out,
	 * FCCU will transition from ALARM to FAULT mode
	 * and system will go to SAFE mode
	 */
	FCCU.NCF_TOE[0].R = 0x00000080;
	//FCCU.NCF_TOE[0].R = 0;

	/* CONFIG state only available for a short time. Switch back to
	 * NORMAL state.  Then switch back to CONFIG for next step
	 * for CONFIG.
	 */
	FCCU.CTRLK.R = 0x825A132B; //Key for OP2
	FCCU.CTRL.R = 0x2;
	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful

	/* Switch back to CONFIG */
    FCCU.TRANS_LOCK.R = 0xBC;		//unlock configuration
	FCCU.CTRLK.R = 0x913756AF;
	FCCU.CTRL.R = 0x1;

	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful


	/* FCCU NCF timer runs on 16 MHz IRC
	 * Set 0x9896800 (1.6 x 10^8d) ticks, which
	 * makes for 10 seconds. When NCF[7] occurs,
	 * will remain in ALARM mode for maximum time allowed
	 * (4.096ms) before transition to FAULT mode
	 */
	FCCU.NCF_TO.R = 0x0000FFFF;

	/* CONFIG state only available for a short time. Switch back to
	 * NORMAL state.  Then switch back to CONFIG for next step
	 * for CONFIG.
	 */
	FCCU.CTRLK.R = 0x825A132B; //Key for OP2
	FCCU.CTRL.R = 0x2;
	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful

	/* Switch back to CONFIG */
    FCCU.TRANS_LOCK.R = 0xBC;		//unlock configuration
	FCCU.CTRLK.R = 0x913756AF;
	FCCU.CTRL.R = 0x1;

	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful

	/* Enable FCCU Alarm interrupt for NCF[7] */
	FCCU.IRQ_ALARM_EN[0].R = 0x00000080;

	/* Switch to NORMAL state */
	FCCU.CTRLK.R = 0x825A132B; //Key for OP2
	FCCU.CTRL.R = 0x2;
	while(FCCU.CTRL.B.OPS != 0x3); //Wait until operation is successful

	/* Configure ALARM interrupt in INTC */
	INTC_0.PSR[488].R = 0x800B;

	/* Configure PF12 interrupt to enter FCCU fault simulation
	 * PF12 maps to SW1 and a selectable function is SIUL2_REQ30
	 */
	SIUL2.DIRER0.B.EIRE30 = 1; //Enable external input interrupt/DMA 30
	SIUL2.DIRSR0.B.DIRSR30 = 0; //Select interrupt option as event

	/* SW1 is active high. Enable SIUL external input interrupt
	 * rising edge event
	 */
	SIUL2.IREER0.B.IREE30 = 1;

	/* Configure PF13 interrupt to clear FCCU simulated fault
	 * PF13 maps to SW2 and a selectable function is SIUL2_REQ31
	 */
	SIUL2.DIRER0.B.EIRE31 = 1; //Enable external input interrupt/DMA 30
	SIUL2.DIRSR0.B.DIRSR31 = 0; //Interrupt option

	SIUL2.IREER0.B.IREE31 = 1; //Trigger on rising edge of SW2

	/* SIUL2 has 32 interrupt sources grouped into 4 sets of 8 sources each.
	 * Interrupts 30 and 31 would fall under group 3
	 */
	INTC_0.PSR[246].R = 0x800D; //Send to Core0 with priority 13

}

void SIUL_ISR(){
	/* See which button was pressed and service that */
	if(SIUL2.DISR0.B.EIF30){
		/* Clear the interrupt flag */
		SIUL2.DISR0.B.EIF30 = 1; //W1C

		/* Implement fake fault */
		FCCU.NCFF.B.FNCFC = 7; //"Fake" trigger NCF[7]


	}else if(SIUL2.DISR0.B.EIF31){
		uint32_t tempval = 0;

		/* Clear the fault NCF[7] */
		FCCU.NCFK.R =  0xAB3498FE;
		FCCU.NCF_S[0].R = 0xFFFFFFFF;
		while (FCCU.CTRL.B.OPS != 0x3);
		FCCU.CTRL.B.OPR = 0xA; //Read the NCF_S register to verify the fault has been cleared
		while (FCCU.CTRL.B.OPS != 0x3);


		FCCU.NCFK.R =  0xAB3498FE;
		FCCU.NCF_S[1].R = 0xFFFFFFFF;
		while (FCCU.CTRL.B.OPS != 0x3);
		FCCU.CTRL.B.OPR = 0xA; //Read the NCF_S register to verify the fault has been cleared
		while (FCCU.CTRL.B.OPS != 0x3);

		FCCU.NCFK.R =  0xAB3498FE;
		FCCU.NCF_S[2].R = 0xFFFFFFFF;
		while (FCCU.CTRL.B.OPS != 0x3);
		FCCU.CTRL.B.OPR = 0xA; //Read the NCF_S register to verify the fault has been cleared
		while (FCCU.CTRL.B.OPS != 0x3);

		/* Read the NCF_S register to verify the fault has been cleared */
		FCCU.CTRL.B.OPR = 10; //Set operation to OP10 (read FCCU_NCF_S)
		while(FCCU.CTRL.B.OPS != 3); //Wait until operation is successful
		tempval = (FCCU.NCF_S[0].R & 0x00000080); //Read value of NCF_S register and mask NCF[34] bit

		/* If NCF[7] remains 1, then call SIUL_ISR recursively to try
		 * to clear the flag again
		 */
		if(tempval){
			SIUL_ISR();
		}

		/* Once flags cleared, read the FCCU state */
		FCCU.CTRL.B.OPR = 3;
		while(FCCU.CTRL.B.OPS != 3);
		tempval = FCCU.STAT.R;

		/* If NORMAL state indicated, switch back to
		 * old mode, IF current mode is safemode
		 */
		if(((tempval & 0x7) == 0) && (MC_ME.GS.B.S_CURRENT_MODE == 2)){
			/* RUN0 -> SAFE allowed; SAFE -> RUN0 not allowed.
			 * Must do SAFE -> DRUN -> RUN0
			 */
			MC_ME.MCTL.R = 0x30005AF0; //Transition to DRUN and key
			MC_ME.MCTL.R = 0x3000A50F; //DRUN and inverted key
			while(MC_ME.GS.B.S_MTRANS); //Wait while mode is in transition

			MC_ME.MCTL.R = (mode_old << 28) | 0x5AF0; //old mode and key
			MC_ME.MCTL.R = (mode_old << 28) | 0xA50F; //old mode and inverted key
			while(MC_ME.GS.B.S_MTRANS); //Wait while mode is in transition
		}

		/* Turn off red LED */
		SIUL2.GPDO[43].R = 1;

		/* If program gets here, then
		 * fault clearing was successful.
		 * Clear the interrupt flag
		 */
		SIUL2.DISR0.B.EIF31 = 1; //W1C
	}
}

void Alarm_ISR(){
	static uint32_t counter = 0; //Internal SW counter
	uint32_t tempval = 0;
	/* Read the STAT register to see
	 * current state of FCCU
	 */
	FCCU.CTRL.B.OPR = 3;
	while(FCCU.CTRL.B.OPS != 3);
	tempval = FCCU.STAT.R;

	/* If FAULT state indicated,
	 * switch system to SAFE mode by SW.
	 * MPC5744P is fault tolerant, meaning system
	 * will not automatically switch to SAFE mode
	 * when FCCU enters FAULT state.
	 * Only do this if not already in SAFE mode
	 */
	if(((tempval & 0x7) == 3) && (MC_ME.GS.B.S_CURRENT_MODE != 2)){
		/* Save the mode so we know where to reenter to
		 * when FCCU returns to NORMAL state
		 */
		mode_old = (MC_ME.GS.R & 0xF0000000) >> 28;
		MC_ME.MCTL.R = 0x20005AF0; //SAFE and key
		MC_ME.MCTL.R = 0x2000A50F; //SAFE and inverted key
		while(MC_ME.GS.B.S_MTRANS); //Wait while mode is in transition
	}

	/* When in SAFE mode, flash red LED based on SW counter */
	if(MC_ME.GS.B.S_CURRENT_MODE == 2){
		counter++; //Increment counter
		if(counter > COUNTERMAX){
			SIUL2.GPDO[43].R ^= 1; //Toggle Red LED
			counter = 0; //Reset counter
		}
	}else{
		/* If not in SAFE mode but in this ISR,
		 * then FCCU was in ALARM state. Just turn on
		 * red LED
		 */
		SIUL2.GPDO[43].R = 0; //Active low
	}
}

int main(void)
{
	xcptn_xmpl ();              /* Configure and Enable Interrupts */

	/* Initialize clock to 160MHz PLL */
	CLK_Init();

	/* Configure IO pins */
	SIUL_Init();

	/* Configure the FCCU */
	FCCU_Init();

	/* Loop forever */
	for(;;){
		/* Make sure red LED is off if
		 * the PC is here.
		 */
		SIUL2.GPDO[43].R = 1;
	}

	return 0;
}
