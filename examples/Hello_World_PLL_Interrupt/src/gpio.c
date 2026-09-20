/*
 * gpio.c
 *
 *  Created on: Feb 23, 2016
 *      Author: B55457
 */


#include "gpio.h"

/********************************************************************************************
*
* @brief    initGPIO - Init LEDs and Buttons
* @param    none
* @return   none
*
*********************************************************************************************/
void initGPIO(void)
{
	/* LEDS on DEVKIT-MPC5744P */
	SIUL2.MSCR[PC11].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[PC11].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[PC11].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[PC11].B.PDO = 1;			/* Turn LED off, note that the LEDs are connected backwards 0 for ON, 1 for OFF */

	SIUL2.MSCR[PC12].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[PC12].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[PC12].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[PC12].B.PDO = 1;			/* Turn LED off, note that the LEDs are connected backwards 0 for ON, 1 for OFF */

	SIUL2.MSCR[PC13].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[PC13].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[PC13].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[PC13].B.PDO = 1;			/* Turn LED off, note that the LEDs are connected backwards 0 for ON, 1 for OFF */

	SIUL2.MSCR[PA0].B.SSS = 0;			/* Pin functionality as GPIO for external LED */
	SIUL2.MSCR[PA0].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[PA0].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[PA0].B.PDO = 1;			/* Turn LED off, note that the LEDs are connected backwards 0 for ON, 1 for OFF */

	/* Buttons on DEVKIT-MPC5744P */
	SIUL2.MSCR[PF12].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[PF12].B.OBE = 0;          /* Output Buffer Enable off */
	SIUL2.MSCR[PF12].B.IBE = 1;			/* Input Buffer Enable on */

	SIUL2.MSCR[PF13].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[PF13].B.OBE = 0;          /* Output Buffer Enable off */
	SIUL2.MSCR[PF13].B.IBE = 1;			/* Input Buffer Enable on */

/* General purpose output pins for test: */
	SIUL2.MSCR[PC10].B.SSS = 0;			/* PG7: Pin functionality as GPIO */
	SIUL2.MSCR[PC10].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[PC10].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[PC10].B.PDO = 0;			/* Inialize low */

	SIUL2.MSCR[PC14].B.SSS = 0;			/* PG8: Pin functionality as GPIO */
	SIUL2.MSCR[PC14].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[PC14].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[PC14].B.PDO = 0;			/* Inialize low */

}

void GPIO_toggle(uint16_t GPIO, uint32_t TOGGLES, uint32_t DELAY)
{
  uint32_t i, j;

  SIUL2.MSCR[GPIO].B.OBE   = 1;
  for(i=0;i<TOGGLES*2;i++)
  {
   for(j=0;j<DELAY;j++);
   SIUL2.GPDO[GPIO].R ^= 1;
  }
}

void DebounceDelay(void)
{
    vuint32_t DelayCounter;
    for (DelayCounter=0; DelayCounter<DEBOUNCEDELAYTIME; DelayCounter++){};
}

void DebouncedWaitTilLow(uint16_t GPIO)
{
    /* enable GPIO as input */
    SIUL2.MSCR[GPIO].B.IBE  = 1;
    /* debounce */
    while(!SIUL2.GPDI[GPIO].R)  {};   /* wait here until level is high, if not already */
    do
    {
      while(SIUL2.GPDI[GPIO].R) {};   /* wait for falling edge */
      DebounceDelay();
    }while(SIUL2.GPDI[GPIO].R);       /* repeat loop, if after debounce delay high */
}

void DebouncedWaitTilHigh(uint16_t GPIO)
{
    /* enable GPIO as input */
    SIUL2.MSCR[GPIO].B.IBE  = 1;
    /* debounce */
    while(SIUL2.GPDI[GPIO].R)  {};   /* wait here until level is low, if not already */
    do
    {
      while(!SIUL2.GPDI[GPIO].R) {};   /* wait for rising edge */
      DebounceDelay();
    }while(!SIUL2.GPDI[GPIO].R);       /* repeat loop, if after debounce delay low */

}

 void clock_out_FMPLL()
 {
   /* Set Up clock selectors to allow clock out 0 to be viewed */
   MC_CGM.AC6_SC.B.SELCTL = 2;           /* Select PLL0 (PLL0-sysclk0) */
   MC_CGM.AC6_DC0.B.DE    = 1;           /* Enable AC6 divider 0 (SYSCLK0)*/
   MC_CGM.AC6_DC0.B.DIV   = 9;           /* Divide by 10 */

   /* Configure Pin for Clock out 0 on PG7 */
   //SIUL2.MSCR[PG7].R = 0x02000003;       /* SRC=2 (Full drive w/o slew) SSS=3 (CLKOUT_0)*/
   SIUL2.MSCR[PB6].R = 0x02000001; /* PB6 = 22 */
 }

  void clock_out_FIRC()
 {
   /* Set Up clock selectors to allow clock out 0 to be viewed */
   MC_CGM.AC6_SC.B.SELCTL = 1;            /* Select FIRC */
   MC_CGM.AC6_DC0.B.DE    = 1;            /* Enable AC6 divider 0 */
   MC_CGM.AC6_DC0.B.DIV   = 9;            /* Divide by 10 */

   /* Configure Pin for Clock out 0 on PG7 */
   //SIUL2.MSCR[PG7].R = 0x02000003;        /* PG7 = 103 */
   SIUL2.MSCR[PB6].R = 0x02000001; /* PB6 = 22 */
 }
