/*****************************************************************************/
/* FILE NAME: smpu.c                           COPYRIGHT (c) Freescale 2015  */
/*                                                      All Rights Reserved  */
/* Description: Simple basic system memory protection unit and enable.       */
/*              Defines chip memory regions, including cache a inhibited     */
/*              region for data shared among masters.                        */
/*                                                                           */
/*****************************************************************************/	
/* REV      AUTHOR        DATE        DESCRIPTION OF CHANGE                  */
/* ---   -----------    ----------    ---------------------                  */
/* 1.0	  S Mihalik     09 Apr 2015   Initial Version                        */
/*****************************************************************************/

#include "project.h" 
#include "smpu.h" 
#include "edma.h" 

/*****************************************************************************/
/* smpu_config                                                               */
/*                                                                           */  
/* SMPU 0 Region Descriptor Summary: (disabled - not used here)              */
/*                                                                           */  
/* SMPU 1 Region Descriptor Summary:                                         */
/*   0  0x4000_0000 to 0x4000_BFFF  All 768K SRAM for MPC5748G               */
/*   1  Variable TCD0_Destination array, 16B, inside SRAM, cache inhibited.  */   
/*      Note: SMPU attributes like CI are OR'd together in overllaped space. */
/*****************************************************************************/

void smpu_config(void) {

	SMPU_0.CESR0.B.GVLD = 0;                        // First ensure module disabled before configuring regions

    // Configure Flash region
    SMPU_0.RGD[0].WORD0.R = 0x40000000;             // start address
    SMPU_0.RGD[0].WORD1.R = 0x4005FFFF;             // end address
    SMPU_0.RGD[0].WORD2.R = 0xFFFFFFFF;             // access control - all bus masters have RW access
    SMPU_0.RGD[0].WORD3.B.CI = 1;					// Inhibit cache for this memory region
    SMPU_0.RGD[0].WORD3.B.VLD = 1;                  // region descriptor valid

    /* After SMPU initialization, any memory access to a location that is not protected by SMPU will generate an exception.
     * This inlcudes peripheral spaces, RAM, FLASH, everything that is used.
     * Demo uses SIUL2, MC_ME, PLLDIG, MC_CGM, DMA, and FLASH after this function is executed.
     * All these regions must be protected. Refer to memory map chapter of MPC574xP RM for more information.
     * However, it is common practice to configure a region for the entire flash block and one for the entire
     * peripheral space.
     */

    /* Configure large flash */
    SMPU_0.RGD[1].WORD0.R = 0x01000000;				//start address
    SMPU_0.RGD[1].WORD1.R = 0x01FFFFFF;				//end address
    SMPU_0.RGD[1].WORD2.R = 0xFFFFFFFF;				// all bus masters have RW control
    SMPU_0.RGD[1].WORD3.B.VLD = 1;                  // Cache init = 0. region descriptor valid

    /* Configure peripheral space */
    SMPU_0.RGD[2].WORD0.R = 0xF8000000;		//Start address of peripheral space
    SMPU_0.RGD[2].WORD1.R = 0xFFFFFFFF;		//End Address of peripheral space
    SMPU_0.RGD[2].WORD2.R = 0xFFFFFFFF;		//all bus masters have RW access to the region
    SMPU_0.RGD[2].WORD3.B.VLD = 1;			//Cache init = 0. Region is valid

    SMPU_0.CESR0.B.GVLD = 1;                        // Enable the module

}
