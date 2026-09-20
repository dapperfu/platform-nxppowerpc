.globl Increment_MCSRR0_to_next_instruction
.globl ClearMCSR
.globl ClearMSR_EE
.globl SetMSR_ME
.globl	 IVOR1_Handler

.extern	 Machine_check_handler

/*******************************************************************************
Function Name : Increment_MCSRR0_to_next_instruction
Engineer      : David Tosenovjan
Date          : Sep-16-2015
Parameters    :
Modifies      :
Returns       :
Notes         : - based on algorithm described in AN4648
                - applicable for IVOR1 (stores address to MCSRR0)
Issues        :
*******************************************************************************/

Increment_MCSRR0_to_next_instruction:


    /* MCSRR0->r5 */
	mfspr r5, 570

    /* determine opcode @ MCSRR0 */
	se_lhz r4,0(r5)

    /* check bit 31,28 only*/
	e_andi. r3,r4,0x9000
	e_cmpli 0x0, r3, 0x1000

   e_bne __machine_check_adjust_for_16bit_opcode

        /* 0xx1 => 32 bit*/
    se_addi r5, 2

    __machine_check_adjust_for_16bit_opcode:

        /* all others just 16 bit long*/
    se_addi r5, 2

        /* save adjusted return address*/
    mtspr 570, r5

    se_blr

/*******************************************************************************
Function Name : ClearMCSR
Engineer      : David Tosenovjan
Date          : Sep-16-2015
Parameters    :
Modifies      :
Returns       :
Notes         : When MSR[ME] is set, error flags must be cleared to prevent
                machine check exception to recall.
Issues        :
*******************************************************************************/
ClearMCSR:
    /* load mask */
	e_lis r4, 0xFFFF
	se_subi r4, 0x0001

    /* Clear MCSR */
    mtspr 572, r4

    se_blr

/*******************************************************************************
Function Name : ClearMSR_EE
Engineer      : David Tosenovjan
Date          : Sep-16-2015
Parameters    :
Modifies      :
Returns       :
Notes         : Negates the MSR[EE].
Issues        :
*******************************************************************************/
ClearMSR_EE:
    /* read spr MSR */
	mfmsr r3

    /* load mask to negate the EE bit */
	e_lis r4, 0xFFFF
	e_add16i r4, r4, 0x7FFF


    /* clear EE bit */
	se_and r3, r4

    /* write back to MSR */
    mtmsr r3

	se_blr

/*******************************************************************************
Function Name : SetMSR_ME
Engineer      : David Tosenovjan
Date          : Sep-16-2015
Parameters    :
Modifies      :
Returns       :
Notes         : Asserts the MSR[ME].
Issues        :
*******************************************************************************/
SetMSR_ME:
    /* read spr MSR */
	mfmsr r3

    /* load mask to assert the ME bit */
	e_lis r4, 0x0000
	e_add16i r4, r4, 0x1000

    /* set ME bit */
	se_or r3, r4

    /* write back to MSR */
    mtmsr r3

    se_blr

/*******************************************************************************
Function Name : IVOR1_Handler
Engineer      : David Tosenovjan
Date          : Sep-16-2015
Parameters    :
Modifies      :
Returns       :
Notes         : Prologue and Epilogue for Machine Check exception (IVOR1).
				Like IVOR4_Handler, prologue saves necessary registers to stack;
				Epilogue restores the saved registers once ISR has been executed.

				IMPORTANT: In Vector.c, change line "e_b IVOR1_Vector" to "e_b IVOR1_Handler".
				This enables the program to branch to this function instead of the dummy
				function IVOR1_Vector.

				IMPORTANT: Also in Vector.c, add the line "PPCASMF(.extern IVOR1_Handler)".
				This makes Vector.c aware of the function IVOR1_Handler.
Issues        :
*******************************************************************************/

IVOR1_Handler:
prolog_IVOR1:
    e_stwu   r1, -0x50(r1)            # Create stack frame and store back chain
    se_stw   r0,  0x20(r1)            # Store GPR0 (working register)
    se_mfctr r0
    se_stw   r0,  0x10(r1)            # Store CTR
    mfxer    r0
    se_stw   r0,  0x14(r1)            # Store XER
    mfcr     r0
    se_stw   r0,  0x18(r1)            # Store CR
    se_mflr  r0
    se_stw   r0,  0x1C(r1)            # Store LR
    se_stw   r3,  0x24(r1)            # Store GPR3
    se_stw   r4,  0x28(r1)            # Store GPR4
    se_stw   r5,  0x2C(r1)            # Store GPR5
    se_stw   r6,  0x30(r1)            # Store GPR6
    se_stw   r7,  0x34(r1)            # Store GPR7
    e_stw    r8,  0x38(r1)            # Store GPR8
    e_stw    r9,  0x3C(r1)            # Store GPR9
    e_stw    r10, 0x40(r1)            # Store GPR10
    e_stw    r11, 0x44(r1)            # Store GPR11
    e_stw    r12, 0x48(r1)            # Store GPR12

    mfmsr    r0                       # As function prologs/epilogs may contain
    e_or2is  r0, 0x0200               # SPE instruction, enable SPE, as MSR
    mtmsr    r0                       # register is cleared at this point

    e_lis    r0,  Machine_check_handler@h
    e_or2i   r0,  Machine_check_handler@l

    mtlr     r0                       # Store LR
    se_blrl                           # Branch to ISR, return here

epilog_IVOR1:
    se_lwz   r3,  0x24(r1)            # Restore GPR3
    se_lwz   r4,  0x28(r1)            # Restore GPR4
    se_lwz   r5,  0x2C(r1)            # Restore GPR5
    se_lwz   r6,  0x30(r1)            # Restore GPR6
    se_lwz   r7,  0x34(r1)            # Restore GPR7
    e_lwz    r8,  0x38(r1)            # Restore GPR8
    e_lwz    r9,  0x3C(r1)            # Restore GPR9
    e_lwz    r10, 0x40(r1)            # Restore GPR10
    e_lwz    r11, 0x44(r1)            # Restore GPR11
    e_lwz    r12, 0x48(r1)            # Restore GPR12
    se_lwz   r0,  0x10(r1)
    se_mtctr r0                       # Restore CTR
    se_lwz   r0,  0x14(r1)
    mtxer r0                          # Restore XER
    se_lwz   r0,  0x18(r1)
    mtcr  r0                          # Restore CR
    se_lwz   r0,  0x1C(r1)
    se_mtlr  r0                       # Restore LR

    se_lwz   r0,  0x20(r1)            # Restore GPR0 (working register)
    e_add16i r1, r1, 0x50             # Clean up stack
    se_rfmci                          # Return from machine check
