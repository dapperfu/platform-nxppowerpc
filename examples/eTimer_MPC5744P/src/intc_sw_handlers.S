# Rewritten for GCC 4.9.4 PowerPC VLE assembler compatibility
    .globl   IVOR4_Handler
    
    .equ  INTC_IACKR, 0xfc040020
    .equ  INTC_EOIR,  0xfc040030
    
    .align 4

IVOR4_Handler:
prologue:
    # Stack frame: calculate r1-0x50, store at that address
    e_add16i    r1, r1, -0x50
    # Store backchain - use absolute addressing via temp reg
    # Actually, we need the old r1 value... use r0
    e_add16i    r0, r1, 0x50          # r0 = old r1
    se_stw      r0, 0(r1)             # Store backchain
    
    # Save SRR - calculate address in r0
    e_add16i    r0, r1, 0x0c
    e_stmvsrrw  0(r0)
    
    # Save r3 to stack at 0x08(r1) - calculate address
    e_mr        r0, r3                 # Save r3 temporarily
    e_add16i    r3, r1, 0x08
    se_stw      r0, 0(r3)
    e_mr        r3, r0                 # Restore r3
    
    # Load INTC_IACKR address (using explicit calculation)
    e_lis       r3, 0xfc04
    e_or2i      r3, r3, 0x0020
    e_lwz       r3, 0(r3)              # Load IACKR value
    
    wrteei      1
    se_lwz      r3, 0(r3)              # Load ISR address
    
    # Save SPRs - calculate address
    e_add16i    r0, r1, 0x14
    e_stmvsprw  0(r0)
    
    se_mtLR     r3
    
    # Save GPRs - calculate address
    e_add16i    r0, r1, 0x24
    e_stmvgprw  0(r0)
    
    se_blrl

epilogue:
    # Restore SPRs
    e_add16i    r0, r1, 0x14
    e_lmvsprw   0(r0)
    
    # Restore GPRs
    e_add16i    r0, r1, 0x24
    e_lmvgprw   0(r0)
    
    # Load INTC_EOIR and write
    e_lis       r3, 0xfc04
    e_or2i      r3, r3, 0x0030
    
    mbar
    wrteei      0
    se_stw      r3, 0(r3)              # Write r3 to INTC_EOIR
    
    # Restore r3
    e_add16i    r0, r1, 0x08
    se_lwz      r3, 0(r0)
    
    # Restore SRR
    e_add16i    r0, r1, 0x0c
    e_lmvsrrw   0(r0)
    
    # Reclaim stack
    e_add16i    r1, r1, 0x50
    
    se_rfi
