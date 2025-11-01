# Startup Code Templates

This directory contains board-specific startup code templates for NXP PowerPC VLE microcontrollers.

## Automatic Inclusion

The baremetal framework automatically includes the appropriate startup code template if:
1. No user-provided startup code is found in the `src/` directory
2. The template matches the selected board

## Supported Boards

Startup code templates are available for:
- `mpc5744p_startup.S` - MPC-5744P
- `mpc5748g_startup.S` - MPC-5748G
- `mpc5748p_startup.S` - MPC-5748P
- `mpc5643l_startup.S` - MPC-5643L
- `mpc5646c_startup.S` - MPC-5646C
- `mpc5775k_startup.S` - MPC-5775K

## Detection Logic

The framework checks for user-provided startup code in this order:
1. Files named `startup.S`, `startup.s`, or `startup.c` in `src/`
2. Any `.S` or `.s` file in `src/` containing the `_start` symbol

If none are found, the platform template is automatically included.

## Overriding the Template

To use your own startup code, simply place a file named `startup.S` (or `startup.s` or `startup.c`) in your project's `src/` directory. The framework will detect it and use your file instead of the template.

## Template Features

Each startup template provides:
- Software Watchdog Timer (SWT) disabling
- Branch Target Buffer (BTB) initialization
- CPU register initialization
- SRAM ECC initialization (where applicable)
- Data section copying from flash to RAM
- BSS section zero-initialization
- Stack pointer initialization
- Small data area (SDA) base register setup
- Call to `main()`

## Customization

If you need to customize startup behavior, you can:
1. Copy the template from this directory to your project's `src/` as `startup.S`
2. Modify it according to your needs
3. The framework will automatically use your custom version

