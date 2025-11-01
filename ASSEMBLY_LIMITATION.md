# GCC 4.9.4 PowerPC VLE Assembler Limitation

## Problem
GCC 4.9.4's PowerPC VLE assembler reports "unsupported relocation" errors when
compiling `intc_sw_handlers.S` files. The issue occurs with register indirect
addressing syntax like `offset(rX)` in VLE instructions such as:
- `e_stwu r1,-0x50(r1)`
- `e_stmvsrrw 0x0c(r1)`
- `e_lwz r3, INTC_IACKR@l(r3)`

## Root Cause
The assembler misinterprets register indirect addressing as relocation expressions,
even with simple syntax like `0(r1)`. This appears to be a bug/limitation in
GCC 4.9.4's PowerPC VLE assembler support.

## Status
- ✅ Fixed @ha/@l relocations (converted to direct constants)
- ✅ Fixed register indirect syntax spacing
- ❌ Fundamental register indirect addressing still fails

## Workarounds
1. **Use pre-compiled object files**: Compile assembly files separately with
   a toolchain that supports the syntax, then link the .o files
2. **Rewrite in C**: Convert interrupt handlers to C with inline assembly
3. **Upgrade toolchain**: Use a newer GCC version if available
4. **Patch assembler**: Apply fixes if available in newer toolchain versions

## Examples Affected
All 45 examples with `intc_sw_handlers.S` files fail to compile due to this limitation.
The 2 framework examples that work don't include these assembly files.
