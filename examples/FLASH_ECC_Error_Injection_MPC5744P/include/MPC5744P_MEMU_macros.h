/*******************************************************************************
* NXP Semiconductors
* (c) Copyright 2016 NXP Semiconductors
* ALL RIGHTS RESERVED.
********************************************************************************
Services performed by NXP in this matter are performed AS IS and without any 
warranty. CUSTOMER retains the final decision relative to the total design
and functionality of the end product. NXP neither guarantees nor will be held
liable by CUSTOMER for the success of this project.
NXP DISCLAIMS ALL WARRANTIES, EXPRESSED, IMPLIED OR STATUTORY INCLUDING,
BUT NOT LIMITED TO, IMPLIED WARRANTY OF MERCHANTABILITY OR FITNESS FOR
A PARTICULAR PURPOSE ON ANY HARDWARE, SOFTWARE ORE ADVISE SUPPLIED 
TO THE PROJECT BY NXP, AND OR NAY PRODUCT RESULTING FROM NXP SERVICES. 
IN NO EVENT SHALL NXP BE LIABLE FOR INCIDENTAL OR CONSEQUENTIAL DAMAGES ARISING
OUT OF THIS AGREEMENT. 
CUSTOMER agrees to hold NXP harmless against any and all claims demands 
or actions by anyone on account of any damage, or injury, whether commercial,
contractual, or tortuous, rising directly or indirectly as a result 
of the advise or assistance supplied CUSTOMER in connection with product, 
services or goods supplied under this Agreement.
********************************************************************************
* File              template.h
* Owner             David Tosenovjan
* Version           0.0
* Date              Mar-05-2016
* Classification    General Business Information
* Brief             Useful macros for MEMU handling
********************************************************************************
* Detailed Description:
********************************************************************************
Revision History:
Ver  Date         Author            Description of Changes
0.0  Mar-05-2016  David Tosenovjan  Initial version
*******************************************************************************/

#ifndef _MEMU_MACROS_H
#define _MEMU_MACROS_H

/*******************************************************************************
* Includes
*******************************************************************************/

/*******************************************************************************
* Constants
*******************************************************************************/

/*******************************************************************************
* Macros 
*******************************************************************************/
/* MEMU_ERR_FLAG macros */

    // Peripheral RAM ECC Correctable Error Detect
#define MEMU_ERR_FLAG_PR_CE   (0x00100000)
    // Peripheral RAM ECC Uncorrectable Error Detect
#define MEMU_ERR_FLAG_PR_UCE  (0x00080000)  
    // Peripheral RAM ECC Correctable error Overflow
#define MEMU_ERR_FLAG_PR_CEO  (0x00040000) 
    // Peripheral RAM ECC Uncorrectable error Overflow
#define MEMU_ERR_FLAG_PR_UCO  (0x00020000)  
    // Peripheral RAM ECC Error buffer Overflow
#define MEMU_ERR_FLAG_PR_EBO  (0x00010000)  
    // Flash ECC Correctable Error Detect
#define MEMU_ERR_FLAG_F_CE    (0x00001000)  
    // Flash ECC Uncorrectable Error Detect
#define MEMU_ERR_FLAG_F_UCE   (0x00000800)  
    // Flash ECC Correctable Error Overflow
#define MEMU_ERR_FLAG_F_CEO   (0x00000400)  
    // Flash ECC Uncorrectable Error Overflow
#define MEMU_ERR_FLAG_F_UCO   (0x00000200)  
    // Flash ECC Error buffer Overflow
#define MEMU_ERR_FLAG_F_EBO   (0x00000100)  
    // System RAM ECC and MBIST Correctable Error Detect
#define MEMU_ERR_FLAG_SR_CE   (0x00000010) 
    // System RAM ECC and MBIST Uncorrectable Error Detect
#define MEMU_ERR_FLAG_SR_UCE  (0x00000008)  
    // System RAM ECC and MBIST Correctable error Overflow
#define MEMU_ERR_FLAG_SR_CEO  (0x00000004)  
    // System RAM ECC and MBIST Uncorrectable error Overflow
#define MEMU_ERR_FLAG_SR_UCO  (0x00000002)  
    // System RAM ECC and MBIST Error buffer Overflow
#define MEMU_ERR_FLAG_SR_EBO  (0x00000001)  

/* MEMU_ERR_FLAG grouped */
#define MEMU_ERR_FLAG_PERRAM_all               (0x001F0000)
#define MEMU_ERR_FLAG_FLASH_all                (0x00001F00)
#define MEMU_ERR_FLAG_SYSRAM_all               (0x0000001F)                          
#define MEMU_ERR_FLAG_correctable_all          (0x00101010)
#define MEMU_ERR_FLAG_uncorrectable_all        (0x00080808)
#define MEMU_ERR_FLAG_correctable_overflows    (0x00040404)
#define MEMU_ERR_FLAG_uncorrectable_overflows  (0x00020202)
#define MEMU_ERR_FLAG_buffer_overflows         (0x00010101)
#define MEMU_ERR_FLAG_PERRAM_overflows         (0x00070000)
#define MEMU_ERR_FLAG_FLASH_overflows          (0x00000700)
#define MEMU_ERR_FLAG_SYSRAM_overflows         (0x00000007)

/*******************************************************************************
* Types
*******************************************************************************/

/*******************************************************************************
* Global variables
*******************************************************************************/
   
/*******************************************************************************
* Global functions
*******************************************************************************/

/*******************************************************************************
* Inline functions
*******************************************************************************/

#endif /* _MEMU_MACROS_H */



    
