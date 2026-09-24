/*
 * edma.h
 *
 *  Created on: Feb 23, 2016
 *      Author: B55457
 */

#ifndef EDMA_H_
#define EDMA_H_

#include "project.h"

#define NUMBER_OF_BYTES 256

void initTCD0(void);
void init_dma_mux(void);
void init_edma_tcd_15(void);
void init_edma_tcd_16(void);
void init_edma_tcd_17(void);
void init_edma_channel_arbitration(void);
//#pragma alignvar (16)
extern uint8_t TCD0_Destination[];

extern const unsigned int  TransmitBuffer[];
extern       unsigned char ReceiveBuffer[];


#endif /* EDMA_H_ */
