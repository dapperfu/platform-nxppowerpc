/*
 * spi_dma.c
 *
 *  Created on: Mar 28, 2016
 *      Author: B48683
 */

#include "project.h"
#include "spi_dma.h"

/*****************************************************************************/
/* Check RM for available pinout options.
**
** DSPI_1 MASTER
** CLK	  PA6
** SOUT	  PA7
** SIN	  PA8
** SS/CS0 PA5
**
** DSPI_2  SLAVE
** CLK	  PA0
** SOUT	  PA1
** SIN	  PA2
** SS/CS0 PA3
*/
/*****************************************************************************/

void init_dspi_ports()
{
  /* Master - DSPI_1 */
  SIUL2.MSCR[PA7].B.SSS = 1;                 /* Pad PA7: Source signal is DSPI_1 SOUT  */
  SIUL2.MSCR[PA7].B.OBE = 1;                 /* Pad PA7: OBE=1. */
  SIUL2.MSCR[PA7].B.SRC = 3;                 /* Pad PA7: Full strength slew rate */

  SIUL2.MSCR[PA6].B.SSS = 1;                /* Pad PA6: Source signal is DSPI_1 CLK  */
  SIUL2.MSCR[PA6].B.OBE = 1;                /* Pad PA6: OBE=1. */
  SIUL2.MSCR[PA6].B.SRC = 3;                /* Pad PA6: Full strength slew rate */

  SIUL2.MSCR[PA8].B.IBE = 1;                /* Pad PA8: Enable pad for input DSPI_1 SIN */
  SIUL2.IMCR[44].B.SSS = 1;            		/* Pad PA8: connected to pad PA8 */

  SIUL2.MSCR[PA5].B.SSS = 1;                 /* Pad PA5: Source signal is DSPI_1 CS0  */
  SIUL2.MSCR[PA5].B.OBE = 1;                 /* Pad PA5: OBE=1. */
  SIUL2.MSCR[PA5].B.SRC = 3;                 /* Pad PA5: Full strength slew rate */

  /* Slave - DSPI_2 */
  SIUL2.MSCR[PA0].B.SSS = 2;                /* Pad PA0: Source signal is DSPI_2 CLK */
  SIUL2.MSCR[PA0].B.IBE = 1;                /* Pad PA0: IBE=1. */
  SIUL2.IMCR[48].B.SSS = 1; 	           /* Pad PA0: DSPI_2 CLK. Slave takes clock from master. Therefore input */

  SIUL2.MSCR[PA1].B.SSS = 2;                /* Pad PA1: Source signal is DSPI_2 SOUT */
  SIUL2.MSCR[PA1].B.OBE = 1;                /* Pad PA1: OBE=1. */
  SIUL2.MSCR[PA1].B.SRC = 3;                /* Pad PA1: Full strength slew rate */

  SIUL2.MSCR[PA2].B.IBE = 1;                /* Pad PA2: Enable pad for input DSPI_2 SIN */
  SIUL2.IMCR[47].B.SSS = 2;            /* Pad PA2: connected to pad */

  SIUL2.MSCR[PA3].B.IBE = 1;                /* Pad PA3: IBE=1. DSPI_2 SS */
  SIUL2.IMCR[49].B.SSS = 1;            /* Pad PA3: connected to pad */
}

/*****************************************************************************/

void init_dspi_1(void)
{
  SPI_1.MCR.R = 0x80010001;                /* Configure DSPI_1 as master */
  SPI_1.MODE.CTAR[0].R = 0xB8001111;       /* Configure CTAR0 : Fast as possible : 20MHz : 3 Delays REQUIRED */
  SPI_1.RSER.R = 0x03000000;               /* Enable DMA for TX */
}

void init_dspi_2(void)
{
  SPI_2.MCR.R = 0x00010001;                 /* Configure DSPI_2 as slave */
  SPI_2.MODE.CTAR[0].R = 0x38000000;        /* Configure CTAR0 : 8 Bit  */
  SPI_2.RSER.R = 0x00030000;                /* Enable DMA for RX */
  SPI_2.MCR.B.HALT = 0x0;                   /* Exit HALT mode: go from STOPPED to RUNNING state*/
  SPI_2.SR.R = 0xFCFE0000;                  /* Clear ALL status flags by writing 1 */
}

/*****************************************************************************/

#if 0

/*****************************************************************************/
/* There are only 2 options available on the EVB
** the "x" signifies the channel used
**
** DSPI_3 MASTER
** CLK	  PG4x	PH12
** SOUT	  PG2x	PH11
** SIN	  PG5x	PI11
** SS/CS0 PG3x	PH13
**
** SPI_1  SLAVE
** CLK	  PJ4x
** SOUT	  PH15  PK13	  PL0x	  PP6
** SIN	  PJ1	  PK15x
** SS/CS0 PI6	  PJ2	    PK14x
*/
/*****************************************************************************/
void init_spi_ports()
{
  /* Master - DSPI_3 */
  SIUL2.MSCR[98].B.SSS = 2;                 /* Pad PG2: Source signal is DSPI_3 SOUT  */
  SIUL2.MSCR[98].B.OBE = 1;                 /* Pad PG2: OBE=1. */
  SIUL2.MSCR[98].B.SRC = 3;                 /* Pad PG2: Full strength slew rate */

  SIUL2.MSCR[100].B.SSS = 2;                /* Pad PG4: Source signal is DSPI_3 CLK  */
  SIUL2.MSCR[100].B.OBE = 1;                /* Pad PG4: OBE=1. */
  SIUL2.MSCR[100].B.SRC = 3;                /* Pad PG4: Full strength slew rate */

  SIUL2.MSCR[101].B.IBE = 1;                /* Pad PG5: Enable pad for input DSPI_3 SIN */
  SIUL2.IMCR[809-512].B.SSS = 1;            /* Pad PG5: connected to pad PG5 */

  SIUL2.MSCR[99].B.SSS = 2;                 /* Pad PG3: Source signal is DSPI_3 CS0  */
  SIUL2.MSCR[99].B.OBE = 1;                 /* Pad PG3: OBE=1. */
  SIUL2.MSCR[99].B.SRC = 3;                 /* Pad PG3: Full strength slew rate */

  /* Slave - SPI_1 */
  SIUL2.MSCR[148].B.SSS = 1;                /* Pad PJ4: Source signal is SPI_1 CLK */
  SIUL2.MSCR[148].B.IBE = 1;                /* Pad PJ4: IBE=1. */
  SIUL2.IMCR[816-512].B.SSS = 1;            /* Pad PJ4: SPI_1 CLK */

  SIUL2.MSCR[176].B.SSS = 1;                /* Pad PL0: Source signal is SPI_1 SOUT */
  SIUL2.MSCR[176].B.OBE = 1;                /* Pad PL0: OBE=1. */
  SIUL2.MSCR[176].B.SRC = 3;                /* Pad PL0: Full strength slew rate */

  SIUL2.MSCR[175].B.IBE = 1;                /* Pad PK15: Enable pad for input SPI_1 SIN */
  SIUL2.IMCR[815-512].B.SSS = 2;            /* Pad PK15: connected to pad */

  SIUL2.MSCR[174].B.IBE = 1;                /* Pad PK14: IBE=1. SPI_1 SS */
  SIUL2.IMCR[817-512].B.SSS = 3;            /* Pad PK14: connected to pad */
}

void init_spi_1(void)
{
  SPI_1.MCR.R = 0x00010001;                 /* Configure DSPI_1 as slave */
  SPI_1.MODE.CTAR[0].R = 0x38000000;        /* Configure CTAR0 : 8 Bit  */
  SPI_1.RSER.R = 0x00030000;                /* Enable DMA for RX */
  SPI_1.MCR.B.HALT = 0x0;                   /* Exit HALT mode: go from STOPPED to RUNNING state*/
  SPI_1.SR.R = 0xFCFE0000;                  /* Clear ALL status flags by writing 1 */
}

/*****************************************************************************/

void init_dspi_3(void)
{
  SPI_3.MCR.R = 0x80010001;                /* Configure DSPI as master */
  SPI_3.MODE.CTAR[0].R = 0xB8001111;       /* Configure CTAR0 : Fast as possible : 20MHz : 3 Delays REQUIRED */
  SPI_3.RSER.R = 0x03000000;               /* Enable DMA for TX */
}

void init_dspi_1(void)
{
  SPI_1.MCR.R = 0x00010001;                 /* Configure DSPI_1 as slave */
  SPI_1.MODE.CTAR[0].R = 0x38000000;        /* Configure CTAR0 : 8 Bit  */
  SPI_1.RSER.R = 0x00030000;                /* Enable DMA for RX */
  SPI_1.MCR.B.HALT = 0x0;                   /* Exit HALT mode: go from STOPPED to RUNNING state*/
  SPI_1.SR.R = 0xFCFE0000;                  /* Clear ALL status flags by writing 1 */
}

/*****************************************************************************/
#endif

