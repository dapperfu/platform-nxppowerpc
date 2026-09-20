/*
 * edma.c
 *
 *  Created on: Feb 23, 2016
 *      Author: B55457
 */

/* edma.c - DMA example TCD initialization */

#include "project.h"
#include "edma.h"

uint8_t TCD0_SourceData[] = {"Hello World\n"};
//#pragma alignvar (16)          /* Align for cache inhibit variable with SMPU */
uint8_t TCD0_Destination[13];
//#pragma alignvar (16)

uint8_t TCD1_SourceData[] = {0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0,0x1,0x0};

void initTCDs(void) {                 /* Transfer string to port pin output */

  DMA_0.TCD[0].SADDR.R = (vuint32_t) &TCD0_SourceData;  /* Load source address*/
  DMA_0.TCD[0].ATTR.B.SSIZE = 0;           /* Read 2**0 = 1 byte per transfer */
  DMA_0.TCD[0].ATTR.B.SMOD = 0;            /* Source modulo feature not used */
  DMA_0.TCD[0].SOFF.R = 1;                 /* After transfer add 1 to src addr*/
  DMA_0.TCD[0].SLAST.R = -12;              /* After major loop, reset src addr*/

  DMA_0.TCD[0].DADDR.R = (vuint32_t) &TCD0_Destination ; /* Load dest. address*/
  DMA_0.TCD[0].ATTR.B.DSIZE = 0;           /* Write 2**0 = 1 byte per transfer*/
  DMA_0.TCD[0].ATTR.B.DMOD = 0;            /* Dest. modulo feature not used */
  DMA_0.TCD[0].DOFF.R = 1;                 /* After transfer add 1 to dst addr*/
  DMA_0.TCD[0].DLASTSGA.R = 0;             /* After major loop no dest addr change*/

  DMA_0.TCD[0].NBYTES.MLNO.R = 1;          /* Transfer 1 byte per minor loop */
  DMA_0.TCD[0].BITER.ELINKNO.B.ELINK = 0;  /* No Enabling channel LINKing */
  DMA_0.TCD[0].BITER.ELINKNO.B.BITER = 13; /* 12 minor loop iterations */
  DMA_0.TCD[0].CITER.ELINKNO.B.ELINK = 0;  /* No Enabling channel LINKing */
  DMA_0.TCD[0].CITER.ELINKNO.B.CITER = 13; /* Init. current iteraction count */

  DMA_0.TCD[0].CSR.B.DREQ = 1;        /* Disable channel when major loop is done*/
  DMA_0.TCD[0].CSR.B.INTHALF = 0;     /* No interrupt when major count half complete */
  DMA_0.TCD[0].CSR.B.INTMAJOR = 0;    /* No interrupt when major count completes */
  DMA_0.TCD[0].CSR.B.MAJORELINK = 0;  /* Dynamic program is not used */
  DMA_0.TCD[0].CSR.B.MAJORLINKCH = 0; /* No link channel # used */
  DMA_0.TCD[0].CSR.B.ESG = 0;         /* Scatter Gather not Enabled */
  DMA_0.TCD[0].CSR.B.BWC = 0;         /* Default bandwidth control- no stalls */
  DMA_0.TCD[0].CSR.B.START = 0;       /* Initialize status flags START, DONE, ACTIVE */
  DMA_0.TCD[0].CSR.B.DONE = 0;
  DMA_0.TCD[0].CSR.B.ACTIVE = 0;


  DMA_0.TCD[1].SADDR.R = (vuint32_t) &TCD1_SourceData;  /* Load source address*/
  DMA_0.TCD[1].ATTR.B.SSIZE = 0;           /* Read 2**0 = 1 byte per transfer */
  DMA_0.TCD[1].ATTR.B.SMOD = 0;            /* Source modulo feature not used */
  DMA_0.TCD[1].SOFF.R = 1;                 /* After transfer add 1 to src addr*/
  DMA_0.TCD[1].SLAST.R = -12;              /* After major loop, reset src addr*/
  DMA_0.TCD[1].DADDR.R = (vuint32_t) &SIUL2.GPDO[PG2].R ; /* Dest. addr. port 98*/
  DMA_0.TCD[1].ATTR.B.DSIZE = 0;           /* Write 2**0 = 1 byte per transfer*/
  DMA_0.TCD[1].ATTR.B.DMOD = 0;            /* Dest. modulo feature not used */
  DMA_0.TCD[1].DOFF.R = 0;                 /* After transfer add 1 to dst addr*/
  DMA_0.TCD[1].DLASTSGA.R = 0;             /* After major loop no dest addr change*/

  DMA_0.TCD[1].NBYTES.MLNO.R = 1;          /* Transfer 1 byte per minor loop */
  DMA_0.TCD[1].BITER.ELINKNO.B.ELINK = 0;  /* No Enabling channel LINKing */
  DMA_0.TCD[1].BITER.ELINKNO.B.BITER = 12; /* 12 minor loop iterations */
  DMA_0.TCD[1].CITER.ELINKNO.B.ELINK = 0;  /* No Enabling channel LINKing */
  DMA_0.TCD[1].CITER.ELINKNO.B.CITER = 12; /* Init. current iteraction count */

  DMA_0.TCD[1].CSR.B.DREQ = 1;        /* Disable channel when major loop is done*/
  DMA_0.TCD[1].CSR.B.INTHALF = 0;     /* No interrupt when major count half complete */
  DMA_0.TCD[1].CSR.B.INTMAJOR = 0;    /* No interrupt when major count completes */
  DMA_0.TCD[1].CSR.B.MAJORELINK = 0;  /* Dynamic program is not used */
  DMA_0.TCD[1].CSR.B.MAJORLINKCH = 0; /* No link channel # used */
  DMA_0.TCD[1].CSR.B.ESG = 0;         /* Scatter Gather not Enabled */
  DMA_0.TCD[1].CSR.B.BWC = 0;         /* Default bandwidth control- no stalls */
  DMA_0.TCD[1].CSR.B.START = 0;       /* Initialize status flags START, DONE, ACTIVE */
  DMA_0.TCD[1].CSR.B.DONE = 0;
  DMA_0.TCD[1].CSR.B.ACTIVE = 0;
}

void init_edma_channel_arbitration (void) { /* Use default fixed arbitration */

  DMA_0.CR.R = 0x0000E400; /* Fixed priority arbitration for groups, channels */

  DMA_0.DCHPRI[0].R  = 0x00; /* Grp 0 chan 00, no suspension, no premption */
  DMA_0.DCHPRI[1].R  = 0x01; /* Grp 0 chan 01, no suspension, no premption */
  DMA_0.DCHPRI[2].R  = 0x02; /* Grp 0 chan 02, no suspension, no premption */
  DMA_0.DCHPRI[3].R  = 0x03; /* Grp 0 chan 03, no suspension, no premption */
  DMA_0.DCHPRI[4].R  = 0x04; /* Grp 0 chan 04, no suspension, no premption */
  DMA_0.DCHPRI[5].R  = 0x05; /* Grp 0 chan 05, no suspension, no premption */
  DMA_0.DCHPRI[6].R  = 0x06; /* Grp 0 chan 06, no suspension, no premption */
  DMA_0.DCHPRI[7].R  = 0x07; /* Grp 0 chan 07, no suspension, no premption */
  DMA_0.DCHPRI[8].R  = 0x08; /* Grp 0 chan 08, no suspension, no premption */
  DMA_0.DCHPRI[9].R  = 0x09; /* Grp 0 chan 09, no suspension, no premption */
  DMA_0.DCHPRI[10].R = 0x0A; /* Grp 0 chan 10, no suspension, no premption */
  DMA_0.DCHPRI[11].R = 0x0B; /* Grp 0 chan 11, no suspension, no premption */
  DMA_0.DCHPRI[12].R = 0x0C; /* Grp 0 chan 12, no suspension, no premption */
  DMA_0.DCHPRI[13].R = 0x0D; /* Grp 0 chan 13, no suspension, no premption */
  DMA_0.DCHPRI[14].R = 0x0E; /* Grp 0 chan 14, no suspension, no premption */
  DMA_0.DCHPRI[15].R = 0x0F; /* Grp 0 chan 15, no suspension, no premption */

  DMA_0.DCHPRI[16].R = 0x10; /* Grp 1 chan 00, no suspension, no premption */
  DMA_0.DCHPRI[17].R = 0x11; /* Grp 1 chan 01, no suspension, no premption */
  DMA_0.DCHPRI[18].R = 0x12; /* Grp 1 chan 02, no suspension, no premption */
  DMA_0.DCHPRI[19].R = 0x13; /* Grp 1 chan 03, no suspension, no premption */
  DMA_0.DCHPRI[20].R = 0x14; /* Grp 1 chan 04, no suspension, no premption */
  DMA_0.DCHPRI[21].R = 0x15; /* Grp 1 chan 05, no suspension, no premption */
  DMA_0.DCHPRI[22].R = 0x16; /* Grp 1 chan 06, no suspension, no premption */
  DMA_0.DCHPRI[23].R = 0x17; /* Grp 1 chan 07, no suspension, no premption */
  DMA_0.DCHPRI[24].R = 0x18; /* Grp 1 chan 08, no suspension, no premption */
  DMA_0.DCHPRI[25].R = 0x19; /* Grp 1 chan 09, no suspension, no premption */
  DMA_0.DCHPRI[26].R = 0x1A; /* Grp 1 chan 10, no suspension, no premption */
  DMA_0.DCHPRI[27].R = 0x1B; /* Grp 1 chan 11, no suspension, no premption */
  DMA_0.DCHPRI[28].R = 0x1C; /* Grp 1 chan 12, no suspension, no premption */
  DMA_0.DCHPRI[29].R = 0x1D; /* Grp 1 chan 13, no suspension, no premption */
  DMA_0.DCHPRI[30].R = 0x1E; /* Grp 1 chan 14, no suspension, no premption */
  DMA_0.DCHPRI[31].R = 0x1F; /* Grp 1 chan 15, no suspension, no premption */
}
