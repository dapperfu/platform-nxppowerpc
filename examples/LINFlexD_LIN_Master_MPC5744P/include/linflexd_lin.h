/*
 * linflexd_lin.h
 *
 *  Created on: Mar 2, 2016
 *      Author: B55457
 */

#ifndef LINFLEXD_LIN_H_
#define LINFLEXD_LIN_H_

#include "derivative.h"
#include "project.h"

#define SELFTEST 0

void initLINFlexD_1 (void);
void transmitLINframe_1 (void);
void receiveLINframe_1(void);

void initLINFlexD_0(void);
void transmitLINframe_0(void);
void receiveLINframe_0(void);


#endif /* LINFLEXD_LIN_H_ */
