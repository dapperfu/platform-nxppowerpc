/*
 * pit.c
 *
 *  Created on: Feb 25, 2016
 *      Author: B55457
 */


#include "pit.h"

/*****************************************************************************/
/* peri_clock_gating                                                         */
/* Description: Configures enabling clocks to peri modules or gating them off*/
/*              Default PCTL[RUN_CFG]=0, so by default RUN_PC[0] is selected.*/
/*              RUN_PC[0] is configured here to gate off all clocks.         */
/*****************************************************************************/

void peri_clock_gating (void) {
  MC_ME.RUN_PC[0].R = 0x00000000;  /* gate off clock for all RUN modes */
  MC_ME.RUN_PC[1].R = 0x000000FE;  /* config. peri clock for all RUN modes */

  MC_ME.PCTL30.B.RUN_CFG = 0b001; //PCTL30 is PIT0 Peripheral Control Registers for Panther
}

void PIT0_init(uint32_t LDVAL) {
	PIT_0.TIMER[0].LDVAL.R = LDVAL; /* Load # PIT clocks to count */
	PIT_0.TIMER[0].TCTRL.B.TIE = 1; /* Enable interrupt */
	INTC_0.PSR[226].R = 0x800A;
	PIT_0.TIMER[0].TCTRL.B.TEN = 1; /* enable channel */

}

void PIT1_init(uint32_t LDVAL) {
	PIT_0.TIMER[1].LDVAL.R = LDVAL; /* Load PIT counter */
	PIT_0.TIMER[1].TCTRL.B.TIE = 1; /* Enable interrupt */
	//INTC_0.PSR[227].B.PRC_SELN0 = 0b1000; /* Send interrupt to processor 0 */
	//INTC_0.PSR[227].B.PRIN = 9; /* Set interrupt to priority 9 */
	INTC_0.PSR[227].R = 0x8009;
	PIT_0.TIMER[1].TCTRL.B.TEN = 1; /* Enable channel */
}

void PIT2_init(uint32_t LDVAL) {
	PIT_0.TIMER[2].LDVAL.R = LDVAL; /* Load PIT counter */
	PIT_0.TIMER[2].TCTRL.B.TIE = 1; /* Enable interrupt */
	//INTC_0.PSR[228].B.PRC_SELN0 = 0b1000; /* Send IRQ to processor 0 */
	//INTC_0.PSR[228].B.PRIN = 11; /* Set priority to 11 */
	INTC_0.PSR[228].R = 0x800B;
	PIT_0.TIMER[2].TCTRL.B.TEN = 1; /* Enable channel */
}
