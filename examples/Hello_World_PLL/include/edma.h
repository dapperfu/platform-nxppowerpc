/*
 * edma.h
 *
 *  Created on: Feb 23, 2016
 *      Author: B55457
 */

#ifndef EDMA_H_
#define EDMA_H_

#include "project.h"

void initTCD0(void);
void init_edma_channel_arbitration(void);
//#pragma alignvar (16)
extern uint8_t TCD0_Destination[];


#endif /* EDMA_H_ */
