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
	/* LEDS on CalypsoEVB */
	SIUL2.MSCR[98].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[98].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[98].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[98].B.PDO = 1;			/* Turn LED off, note that the LEDs are connected backwards 0 for ON, 1 for OFF */

	SIUL2.MSCR[99].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[99].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[99].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[99].B.PDO = 1;			/* Turn LED off, note that the LEDs are connected backwards 0 for ON, 1 for OFF */

	SIUL2.MSCR[100].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[100].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[100].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[100].B.PDO = 1;			/* Turn LED off, note that the LEDs are connected backwards 0 for ON, 1 for OFF */

	SIUL2.MSCR[101].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[101].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[101].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[101].B.PDO = 1;			/* Turn LED off, note that the LEDs are connected backwards 0 for ON, 1 for OFF */

	/* Buttons on CalypsoEVB */
	SIUL2.MSCR[1].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[1].B.OBE = 0;          /* Output Buffer Enable off */
	SIUL2.MSCR[1].B.IBE = 1;			/* Input Buffer Enable on */

	SIUL2.MSCR[2].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[2].B.OBE = 0;          /* Output Buffer Enable off */
	SIUL2.MSCR[2].B.IBE = 1;			/* Input Buffer Enable on */

	SIUL2.MSCR[89].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[89].B.OBE = 0;          /* Output Buffer Enable off */
	SIUL2.MSCR[89].B.IBE = 1;			/* Input Buffer Enable on */

	SIUL2.MSCR[91].B.SSS = 0;			/* Pin functionality as GPIO */
	SIUL2.MSCR[91].B.OBE = 0;          /* Output Buffer Enable off */
	SIUL2.MSCR[91].B.IBE = 1;			/* Input Buffer Enable on */
/* General purpose output pins for test: */
	SIUL2.MSCR[103].B.SSS = 0;			/* PG7: Pin functionality as GPIO */
	SIUL2.MSCR[103].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[103].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[103].B.PDO = 0;			/* Inialize low */

	SIUL2.MSCR[104].B.SSS = 0;			/* PG8: Pin functionality as GPIO */
	SIUL2.MSCR[104].B.OBE = 1;          /* Output Buffer Enable on */
	SIUL2.MSCR[104].B.IBE = 0;			/* Input Buffer Enable off */
	SIUL2.GPDO[104].B.PDO = 0;			/* Inialize low */

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

 void clock_out_PLL()
 {
   /* Set Up clock selectors to allow clock out 0 to be viewed */
   MC_CGM.AC6_SC.B.SELCTL = 2;           /* Select PHI_0 (PLL0-sysclk0) */
   MC_CGM.AC6_DC0.B.DE    = 1;           /* Enable AC6 divider 0 (SYSCLK0)*/
   MC_CGM.AC6_DC0.B.DIV   = 9;           /* Divide by 10 */

   /* Configure Pin for Clock out 0 on PB6 */
   SIUL2.MSCR[PB6].R = 0x02000001;       /* SRC=2 (Full drive w/o slew) SSS=3 (CLKOUT_0)*/
 }

  void clock_out_FIRC()
 {
   /* Set Up clock selectors to allow clock out 0 to be viewed */
   MC_CGM.AC6_SC.B.SELCTL = 0;            /* Select FIRC */
   MC_CGM.AC6_DC0.B.DE    = 1;            /* Enable AC6 divider 0 */
   MC_CGM.AC6_DC0.B.DIV   = 9;            /* Divide by 10 */

   /* Configure Pin for Clock out 0 on PB6 */
   SIUL2.MSCR[PB6].R = 0x02000001;        /* PB6 = 22 */
 }
