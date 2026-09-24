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
* File:             main.c
* Owner:            David Tosenovjan
* Version:          0.0
* Date:             Mar-05-2016
* Classification:   General Business Information
* Brief:			Shows ECC error, correctable and non-correctable
********************************************************************************
* Detailed Description:
* Purpose of the example is to show how to generate Multi-bit or Single-bit
* ECC error in internal FLASH (user must choose it in the option at the end of
* main function).
* Flash over-programming is used to generate a non-correctable (or single-bit)
* ECC error in FLASH. The bad data is accessed then what's generate IVOR1
* exception or FCCU_Alarm_Interrupt. Both function calls MEMU handler.
* Example also offers useful macros for MEMU module.
* The example displays notices in the terminal window (connector J19 on
* MPC57xx_Motherboard)(19200-8-no parity-1 stop bit-no flow control on eSCI_A).
* No other external connection is required.
* ------------------------------------------------------------------------------
* Test HW:         XDEVKIT-MPC5744P
* MCU:             PPC5744PFMLQ8,0N15P,QQAA1515N, Rev2.1B
* Fsys:            200 MHz PLL with 40 MHz crystal reference
* Debugger:        OpenSDA
* Target:          internal_FLASH, RAM
* Terminal:        19200-8-no parity-1 stop bit-no flow control
* EVB connection:  none
********************************************************************************
Revision History:
Ver  Date         Author            Description of Changes
0.0  Mar-05-2016  David Tosenovjan  Initial version
*******************************************************************************/

#include "uart.h"
#include <stdio.h>
#include "MPC5744P.h"
#include <string.h>
#include "MPC5744P_MEMU_macros.h"
#include <string.h>


/*******************************************************************************
* External objects
*******************************************************************************/

/*******************************************************************************
* Global variables
*******************************************************************************/
//extern const uint32_t IntcIsrVectorTable[];
extern void xcptn_xmpl(void);
extern void ClearMCSR(void);
extern void Increment_MCSRR0_to_next_instruction(void);
extern void SetMSR_ME(void);


/*******************************************************************************
* Constants and macros
*******************************************************************************/
#define VLE_IS_ON 1

#define DRUN_MODE   0x3

/* FCCU keys */
#define FCCU_NCFK_KEY 0xAB3498FE

/* CTRLK register OP-keys for FCCU configuration */
#define KEY_OP1  0x913756AF
#define KEY_OP2  0x825A132B
#define KEY_OP16 0x7ACB32F0
#define KEY_OP31 0x29AF8752


/*******************************************************************************
* Local types
*******************************************************************************/

/*******************************************************************************
* Local function prototypes
*******************************************************************************/
void Sys_Init(void);
void FCCU_Init(void);
void ClearNCF(void);
void FCCU_Alarm_Interrupt(void);

void MEMU_handler(void);
void Machine_check_handler(void);

void Generate_noncorrectable_FLASH_ECC_error(void);
void Generate_1bit_FLASH_ECC_error(void);
void Fix_2bit_error_FLASH_data(void);
void Erase_L7_block(void);


/*******************************************************************************
* Global variables
*******************************************************************************/
uint32_t NCF_status[4];
uint32_t Hex_to_ASCII[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};

/* debugging variables */
vint32_t IVOR1_handler_pass_count = 0;
vint32_t noncritical_fault_count = 0;

/*******************************************************************************
* Local functions
*******************************************************************************/

/*******************************************************************************
Function Name : Sys_Init
Engineer      : Petr Vlna
Date          : Dec-19-2014
Parameters    :
Modifies      :
Returns       :
Notes         : Clock settings 200MHz PLL
Issues        :
*******************************************************************************/
void Sys_Init(void)
{
    //Clk Out
    MC_CGM.AC6_SC.R = 0x02000000;  //Enable PLL0 - clkout0 //MC_CGM_AC6_SC
    MC_CGM.AC6_DC0.B.DE = 1;       //Enable divider for SYSCLK0
    MC_CGM.AC6_DC0.B.DIV = 0;      //Set divider for SYSCLK0

    //Configure RunPeripheralConfiguration regist   ers in ME_RUN_PC0
    MC_ME.RUN_PC[0].B.DRUN = 1;    //Enable DRUN mode for all peripherals

    //Enable external oscilator
    MC_ME.DRUN_MC.B.XOSCON = 1;

    //AUX Clock Selector 3 setup
    MC_CGM.AC3_SC.B.SELCTL =0x01;  //connect (8..40MHz) XTALL to the PLL0 input
    MC_ME.DRUN_MC.B.PLL0ON = 1;    //Enable PLL0 for DRUN mode

    // Set PLL0 to 200MHz
    PLLDIG.PLL0CR.B.CLKCFG = 1;             //Bypass mode PLL0 on
    // RFDPHI1 = 10, RFDPHI = 2, PREDIV = 2, MFD = 14
    PLLDIG.PLL0DV.R = 0x50000000 |0x00020000 |0x00002000 |0x0014 ;

    // Set PPL0 as system clock
    MC_ME.DRUN_MC.B.SYSCLK = 0x2;
    //  Enable system clock divider /4
    MC_CGM.SC_DC0.B.DIV = 0x3;

    //Mode transition to apply the PLL0 setup and set Normal mode with PLL
    MC_ME.MCTL.R = 0x30005AF0;      //DRUN Mode & Key
    MC_ME.MCTL.R = 0x3000A50F;      //DRUN Mode & Key

    while(!MC_ME.GS.B.S_PLL0);      //ME_GS Wait for PLL stabilization.
    while(MC_ME.GS.B.S_MTRANS);     //Waiting for end of transaction
    // ME_GS Check DRUN mode has successfully been entered
    while(MC_ME.GS.B.S_CURRENT_MODE != DRUN_MODE);
}


/*******************************************************************************
Function Name : FCCU_Init
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         :
Issues        :
*******************************************************************************/
void FCCU_Init(void)
{
    /* clear possible faults*/
    ClearNCF();

    /* Unlock configuration */
    FCCU.TRANS_LOCK.R = 0xBC;
    /* provide Config state key */
    FCCU.CTRLK.R = 0x913756AF; //key for OP1
    /* enter config state - OP1 */
    FCCU.CTRL.R = 0x1; //set OP1 - set up FCCU into the CONFIG mode
    /* wait for successful state transition */
    while (FCCU.CTRL.B.OPS != 0x3); //operation status succesful

    /* Non-critical fault enable for all MEMU sources */
    FCCU.NCF_TOE[0].R = 0xFFFFFFFF; //ALARM Timeout Enable
    FCCU.NCF_E[0].R = 0x00FF8000; // NCF[15]-NCF[23]
    FCCU.IRQ_ALARM_EN[0].R = 0x00FF8000;

    /* set up the NORMAL mode of FCCU */
    FCCU.CTRLK.R = 0x825A132B; //key for OP2
    FCCU.CTRL.R = 0x2; //set the OP2 - set up FCCU into the NORMAL mode
    while (FCCU.CTRL.B.OPS != 0x3); //operational status succesful
}


/*******************************************************************************
Function Name : ClearNCF
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         : clears all FCCU faults
Issues        :
*******************************************************************************/
void ClearNCF(void)
{
    volatile uint32_t i,b[4];
    for(i=0;i<4;i++)
    {
        FCCU.NCFK.R = FCCU_NCFK_KEY;
        FCCU.NCF_S[i].R = 0xFFFFFFFF;
        while(FCCU.CTRL.B.OPS != 0x3)
        {

        };              /* wait for the completion of the operation */
        b[i]=FCCU.NCF_S[i].R;
        if(b[i]){}
    }
}


/*******************************************************************************
Function Name : FCCU_Alarm_Interrupt
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         :
Issues        : for simplicity it clears all faults!
*******************************************************************************/
void FCCU_Alarm_Interrupt(void)
{
   uint32_t i;

   MEMU_handler();

   FCCU.CTRL.R = 10; /* OP10 - Read the NCF status registers */
   while(FCCU.CTRL.B.OPS != 0x3)
   {
   };      /* wait for the completion of the operation */

   /* load these regs to the variable field */
   for(i=0;i<3;i++)
   {
       NCF_status[i] = FCCU.NCF_S[i].R;
   }

   noncritical_fault_count++;
   ClearNCF();
}


/*******************************************************************************
Function Name : MEMU_handler
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         :
*******************************************************************************/
void MEMU_handler(void)
{
    uint32_t MEMU_ERR_FLAG_value = 0;
    uint32_t i, j;

    /* examine source of ECC error */
    /* for the purpose of this example only FLASH errors are being examined */

    MEMU_ERR_FLAG_value = MEMU.ERR_FLAG.R;

    if (MEMU_ERR_FLAG_value & MEMU_ERR_FLAG_FLASH_all)
    {
        //printf("FLASH ECC error found\r\n");
    	uint8_t message0[] = {"FLASH ECC error found\r\n"};
    	TransmitData((const char*)message0, strlen((const char*)message0));
        if (MEMU_ERR_FLAG_value & MEMU_ERR_FLAG_F_UCE)
        {
            if (1 == MEMU.FLASH_UNCERR_STS.B.VLD)
            {

            	/* printf("It is non-correctable error at address "); */
            	uint32_t data = MEMU.FLASH_UNCERR_ADDR.R; //Need to print the value of this register. But convert to ASCII first
            	uint8_t data_ASCII[9] = {0}; //ASCII array for converting data to ASCII, null terminated
            	uint8_t message0[] = {"It is non-correctable error at address "};
            	TransmitData((const char*)message0, strlen((const char*)message0));

                /* printf("0x%08X\r\n",MEMU.FLASH_UNCERR_ADDR.R); */

            	uint8_t message1_0[] = {"0x"};
            	TransmitData((const char*)message1_0, strlen((const char*)message1_0));

            	/* Convert data to ASCII. Leave the last character 0, because
            	 * it is the null terminator.
            	 */
            	for(i = 0; i < 8; i++){
            		/* Take the most significant 4 bits. The value of
            		 * those 4 bits is the offset of the ASCII lookup table.
            		 */
            		uint8_t offset = (data & 0xF0000000) >> 28; //offset has the value of the most significant 4 bits then shifted to LSB
            		data_ASCII[i] = Hex_to_ASCII[offset];

            		/* Shift the next hex digit (4 bits) to the MSB */
            		data = data << 4;
            	}

            	/* Once all 8 hex characters stored in data_ASCII, print it. */
            	TransmitData((const char*)data_ASCII, strlen((const char*)data_ASCII));

            	uint8_t message1_1[] = {"\n\r"};
            	TransmitData((const char*)message1_1, strlen((const char*)message1_1));

                // invalidate the entry
                MEMU.FLASH_UNCERR_STS.R = 0x80000000;

                // remove the reson of ECC error (application specific)
                Fix_2bit_error_FLASH_data();
            }
        }

        if (MEMU_ERR_FLAG_value & MEMU_ERR_FLAG_F_CE)
        {
            for(i=0;i<20;i++)
            {
                if (1 == MEMU.FLASH_CERR[i].STS.B.VLD)
                {
                	uint32_t data = MEMU.FLASH_CERR[i].ADDR.R;
                	uint8_t data_ASCII[9] = {0}; //Last character is null terminator

                    /* printf("It is correctable error at address "); */
                	uint8_t message0[] = {"It is correctable error at address "};
                	TransmitData((const char*)message0, strlen((const char*)message0));

                    /* printf("0x%08X\r\n",MEMU.FLASH_CERR[i].ADDR.R); */
                	uint8_t message1_0[] = {"0x"};
                	TransmitData((const char*)message1_0, strlen((const char*)message1_0));


                	/* Convert data to ASCII. Leave the last character 0, because
                	 * it is the null terminator.
                	 */
                	for(j = 0; j < 8; j++){
                		/* Take the most significant 4 bits. The value of
                		 * those 4 bits is the offset of the ASCII lookup table.
                		 */
                		uint8_t offset = (data & 0xF0000000) >> 28; //offset has the value of the most significant 4 bits then shifted to LSB
                		data_ASCII[j] = Hex_to_ASCII[offset];

                		/* Shift the next hex digit (4 bits) to the MSB */
                		data = data << 4;
                	}

                	/* Once all 8 hex characters stored in data_ASCII, print it. */
                	TransmitData((const char*)data_ASCII, strlen((const char*)data_ASCII));

                	uint8_t message1_1[] = {"\n\r"};
                	TransmitData((const char*)message1_1, strlen((const char*)message1_1));

                    // invalidate the entry
                    MEMU.FLASH_CERR[i].STS.R = 0x80000000;

                    /* there is no need to correct single bit errors */
                    /* printf("Single bit errors ignored\r\n"); */
                    uint8_t message2[] = {"Single bit errors ignored\r\n"};
                    TransmitData((const char*)message2, strlen((const char*)message2));

                }
            }

        }

        if (MEMU_ERR_FLAG_value & MEMU_ERR_FLAG_FLASH_overflows)
        {
            /* printf("Overflow!"); */
        	uint8_t message0[] = {"Overflow!"};
        	TransmitData((const char*)message0, strlen((const char*)message0));
            while(1)
            {
                //asm("nop");
            	//__asm__("nop");
            	__asm__("e_ori 0,0,0");
            }
        }

    }

    /* check other possible sources */
    else if (MEMU_ERR_FLAG_value & MEMU_ERR_FLAG_PERRAM_all)
    {
        /* printf("Peripheral RAM ECC error found\r\n"); */
    	uint8_t message0[] = {"Peripheral RAM ECC error found\r\n"};
    	TransmitData((const char*)message0, strlen((const char*)message0));
    }

    else if (MEMU_ERR_FLAG_value & MEMU_ERR_FLAG_SYSRAM_all)
    {
        /* printf("System RAM ECC error found\r\n"); */
    	uint8_t message0[] = {"System RAM ECC error found\r\n"};
    	TransmitData((const char*)message0, strlen((const char*)message0));
    }

    /* clear all MEMU error flag flags to stop FCCU fault indication */
    MEMU.ERR_FLAG.R = 0xFFFFFFFF;
}


/*******************************************************************************
Function Name : Machine_check_handler
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         : Function handles Machine_check_handler
*******************************************************************************/
//__interrupt
void Machine_check_handler(void)
{

    MEMU_handler();

    Increment_MCSRR0_to_next_instruction();

    ClearMCSR();

    IVOR1_handler_pass_count++;

}


/*******************************************************************************
Function Name : Generate_noncorrectable_FLASH_ECC_error
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         :
Issues        :
*******************************************************************************/
void Generate_noncorrectable_FLASH_ECC_error(void)
{
    register uint32_t test_read = 0;

    /**********   Flash partitioning on MPC5744P               **********/
    // 4x 16 KB in partition 0/1 (2x blocks EEPROM emulation enabled)
    // 2x 32 KB in partition 2/3 (EEPROM emulation enabled)
    // 6x 64 KB in partition 4/5
    // 8x 256 KB in partition 6/7

    /**********   Flash blocks on MPC5744P                     **********/
    // Data flash memory - no overlay
    // 0x00800000 - 0x00803FFF 16 16KB EEPROM low block 0
    // 0x00804000 - 0x00807FFF 16 16KB EEPROM low block 1
    // 0x00808000 - 0x0080FFFF 32 32KB EEPROM mid block 0
    // 0x00810000 - 0x00817FFF 32 32KB EEPROM mid block 1

    // Small flash memory blocks - no overlay
    // 0x00F98000 - 0x00F9BFFF 16 16KB low flash memory block 2 (boot 0)
    // 0x00F9C000 - 0x00F9FFFF 16 16KB low flash memory block 3 (boot 1)

    // Medium flash memory blocks - no overlay
    // 0x00FA0000 - 0x00FAFFFF 64 64KB high flash memory block 0 (boot 2)
    // 0x00FB0000 - 0x00FBFFFF 64 64KB high flash memory block 1 (boot 3)
    // 0x00FC0000 - 0x00FCFFFF 64 64KB high flash memory block 2
    // 0x00FD0000 - 0x00FDFFFF 64 64KB high flash memory block 3
    // 0x00FE0000 - 0x00FEFFFF 64 64KB high flash memory block 4
    // 0x00FF0000 - 0x00FFFFFF 64 64KB high flash memory block 5

    // Large flash memory blocks - no overlay
    // 0x01000000 - 0x0103FFFF 256 256KB flash memory block 0 (boot 4)
    // 0x01040000 - 0x0107FFFF 256 256KB flash memory block 1 (boot 5)
    // 0x01080000 - 0x010BFFFF 256 256KB flash memory block 2 (boot 6)
    // 0x010C0000 - 0x010FFFFF 256 256KB flash memory block 3 (boot 7)
    // 0x01100000 - 0x0113FFFF 256 256KB flash memory block 4
    // 0x01140000 - 0x0117FFFF 256 256KB flash memory block 5
    // 0x01180000 - 0x011BFFFF 256 256KB flash memory block 6
    // 0x011C0000 - 0x011FFFFF 256 256KB flash memory block 7


    /* Generate flash ECC error */

    // step1. unlock all blocks (for simplicity)
    C55FMC.LOCK0.R = 0;
    C55FMC.LOCK1.R = 0;
    C55FMC.LOCK2.R = 0;
    C55FMC.LOCK3.R = 0;

    // step2. erase the large block 7 (0x011C0000 - 0x011FFFFF)
    C55FMC.MCR.B.ERS = 1;
    C55FMC.SEL2.R = 0x00000080; // select the large block 7
    *(unsigned int*)0x011C0000 = 0xFFFFFFFF;    //interlock write
    C55FMC.MCR.B.EHV = 1;
    while(C55FMC.MCR.B.DONE == 0);
    C55FMC.MCR.B.EHV = 0;
    C55FMC.MCR.B.ERS = 0;

    // step3. program data
    C55FMC.MCR.B.PGM = 1;
    *(unsigned int*)0x011C0000 = 0x00450000;    //interlock write
    *(unsigned int*)0x011C0004 = 0x00000000;

    C55FMC.MCR.B.EHV = 1;
    while(C55FMC.MCR.B.DONE == 0);
    C55FMC.MCR.B.EHV = 0;
    C55FMC.MCR.B.PGM = 0;

    // step4. over-program data - this generates ECC error
    C55FMC.MCR.B.PGM = 1;
    *(unsigned int*)0x011C0000 = 0x00580000;    //interlock write
    *(unsigned int*)0x011C0004 = 0x00000000;
    C55FMC.MCR.B.EHV = 1;
    while(C55FMC.MCR.B.DONE == 0);
    C55FMC.MCR.B.EHV = 0;
    C55FMC.MCR.B.PGM = 0;

    // step 5. now here the ECC is checked and non-correctable error found
    test_read = *(unsigned int*)0x011C0000;
    //test_read = *(unsigned int*)0x011C0004;

    /* Without this line, the compiler will optimize out the dummy read. */
    if(test_read){
    	return;
    }

}


/*******************************************************************************
Function Name : Generate_1bit_FLASH_ECC_error
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         :
Issues        :
*******************************************************************************/
void Generate_1bit_FLASH_ECC_error(void)
{
    register uint32_t test_read = 0;

    /* Enable single bit ECC error reporting in flash controller */

    // Enable UTest mode
    C55FMC.UT0.R = 0xF9F99999;
    // Enable single bit error correction
    C55FMC.UT0.B.SBCE = 1;
    // Finish the UTest mode by writing UT0[UTE] with 0.
    C55FMC.UT0.B.UTE = 0;

    /* Now generate flash ECC error */

    // step1. unlock all blocks (for simplicity)
    C55FMC.LOCK0.R = 0;
    C55FMC.LOCK1.R = 0;
    C55FMC.LOCK2.R = 0;
    C55FMC.LOCK3.R = 0;

    // step2. erase the large block 7 (0x011C0000 - 0x011FFFFF)
    C55FMC.MCR.B.ERS = 1;
    C55FMC.SEL2.R = 0x00000080; // select the large block 7
    *(unsigned int*)0x011C0000 = 0xFFFFFFFF;    //interlock write
    C55FMC.MCR.B.EHV = 1;
    while(C55FMC.MCR.B.DONE == 0);
    C55FMC.MCR.B.EHV = 0;
    C55FMC.MCR.B.ERS = 0;

    // step3. program data
    C55FMC.MCR.B.PGM = 1;
    *(unsigned int*)0x011C0000 = 0xFFFFFFFF;    //interlock write
    *(unsigned int*)0x011C0004 = 0x00000000;

    C55FMC.MCR.B.EHV = 1;
    while(C55FMC.MCR.B.DONE == 0);
    C55FMC.MCR.B.EHV = 0;
    C55FMC.MCR.B.PGM = 0;

    // step4. over-program data - this generates ECC error
    C55FMC.MCR.B.PGM = 1;
    *(unsigned int*)0x011C0000 = 0xFFFFFFFF;    //interlock write
    *(unsigned int*)0x011C0004 = 0x00000001;
    C55FMC.MCR.B.EHV = 1;
    while(C55FMC.MCR.B.DONE == 0);
    C55FMC.MCR.B.EHV = 0;
    C55FMC.MCR.B.PGM = 0;

    // step 5. now here the ECC is checked and non-correctable error found
    test_read = *(unsigned int*)0x011C0000;
    //test_read = *(unsigned int*)0x011C0004;

    /* Without this line, the compiler will optimize out the dummy read. */
    if(test_read){
    	return;
    }

}


/*******************************************************************************
Function Name : Fix_2bit_error_FLASH_data
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         :
*******************************************************************************/
void Fix_2bit_error_FLASH_data(void)
{
    // erase block as example of data correction (application specific)
    Erase_L7_block();
}


/*******************************************************************************
Function Name : Erase_L7_block
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         :
*******************************************************************************/
void Erase_L7_block(void)
{
    // step1. unlock all blocks (for simplicity)
    C55FMC.LOCK0.R = 0;
    C55FMC.LOCK1.R = 0;
    C55FMC.LOCK2.R = 0;
    C55FMC.LOCK3.R = 0;

    // step2. erase the large block 7 (0x011C0000 - 0x011FFFFF)
    C55FMC.MCR.B.ERS = 1;
    C55FMC.SEL2.R = 0x00000080; // select the large block 7
    *(unsigned int*)0x011C0000 = 0xFFFFFFFF;    //interlock write
    C55FMC.MCR.B.EHV = 1;
    while(C55FMC.MCR.B.DONE == 0);
    C55FMC.MCR.B.EHV = 0;
    C55FMC.MCR.B.ERS = 0;

    /* printf("L7 flash block erased (0x011C0000 - 0x011FFFFF) \r\n"); */
    uint8_t message0[] = {"L7 flash block erased (0x011C0000 - 0x011FFFFF) \r\n"};
    TransmitData((const char*)message0, strlen((const char*)message0));
}

/*******************************************************************************
* Global functions
*******************************************************************************/

/*******************************************************************************
Function Name : main
Engineer      : David Tosenovjan
Date          : Mar-05-2016
Parameters    :
Modifies      :
Returns       :
Notes         :
*******************************************************************************/
int main (void)
{
	uint8_t ch=0;

    Sys_Init();

    LINFlexD_1_Init();

    /* printf("ECC error injection example\r\n"); */
    uint8_t message0[] = {"ECC error injection example\r\n"};
    TransmitData((const char*)message0, strlen((const char*)message0));

    xcptn_xmpl();

    /* Set interrupt priority for FCCU (PSR 488). */
    INTC_0.PSR[488].R = 0x801F;

    FCCU_Init();



    SetMSR_ME();

    /* Generate error. Wait for user input */

    uint8_t message_loop0[] = {"\n\rPress '1' to generate noncorrectable error or '2' to generate 1-bit error.\n\r"};
    uint8_t message_loop1[] = {"\n\rInvalid input! Reset system and try again.\n\r"};
    uint8_t end_message[] = {"\n\rEnd of MPC5744P ECC error test.\n\r"};
    TransmitData((const char*)message_loop0,strlen((const char*)message_loop0));
    ReceiveData((char*)&ch);
    /**************************************************************************/
    /*                                                                        */
    /* Choose which ECC error is supposed to be injected !					  */
    /* Enter '1' for noncorrectable error									  */
    /* Enter '2' for 1-bit error 											  */
    /*                                                                        */
    /**************************************************************************/
    switch(ch){
    case '1':
    	Generate_noncorrectable_FLASH_ECC_error();
    	break;
    case '2':
    	Generate_1bit_FLASH_ECC_error();
    	break;
    default:
    	TransmitData((const char*)message_loop1,strlen((const char*)message_loop1));
    	break;
    }

    TransmitData((const char*)end_message,strlen((const char*)end_message));

    while(1);

  return 0;
}//main
