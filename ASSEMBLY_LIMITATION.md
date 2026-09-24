# GCC 4.9.4 PowerPC VLE assembler

Compile `intc_sw_handlers.S` with the Makefile machine flags
(`-mcpu=e200z4 -mbig -mvle -mregnames -mhard-float`) through `powerpc-eabivle-gcc`.
Do not pass `-specs` twice on `.S` files (SCons applies ASFLAGS+ASPPFLAGS).
