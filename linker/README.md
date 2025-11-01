# Linker Scripts

This directory contains board-specific linker scripts for NXP PowerPC VLE microcontrollers.

## Automatic Linker Script Selection

The PlatformIO builder automatically selects the appropriate linker script using the following priority:

1. **User-specified**: If `-T path/to/linker.ld` is in `build_flags`, that is used
2. **Board configuration**: `board.build.linker_script` in board.json
3. **Project-level**: `linker.ld` in the project root
4. **Platform default**: Board-specific script from `platform/linker/`

## Board Configuration

Linker scripts can be configured in board JSON files:

```json
{
  "build": {
    "mcu": "mpc5748g",
    "linker_type": "flash",        // "flash" or "ram"
    "linker_script": "57xx_flash.ld"  // Optional: specific script name
  }
}
```

## Available Linker Scripts

- `57xx_flash.ld` - MPC57xx series flash linker script (default for MPC5744P, MPC5748G, MPC5748P)
- `57xx_ram.ld` - MPC57xx series RAM linker script
- `mpc5744p_flash.ld` - MPC5744P-specific flash linker script
- `mpc5748g_flash.ld` - MPC5748G-specific flash linker script

## Using Custom Linker Scripts

### Option 1: Project-level override
Place `linker.ld` in your project root:

```ini
; platformio.ini
[env:mpc5748g]
platform = nxppowerpc
board = mpc5748g
; Linker script auto-detected from PROJECT_DIR/linker.ld
```

### Option 2: Explicit path in build_flags
```ini
[env:mpc5748g]
platform = nxppowerpc
board = mpc5748g
build_flags =
    -T ${PROJECT_DIR}/custom_linker.ld
```

### Option 3: Use RAM variant
Set in board.json:
```json
{
  "build": {
    "linker_type": "ram"
  }
}
```

Or override in platformio.ini:
```ini
[env:mpc5748g]
board = mpc5748g
build_flags =
    -T ${PLATFORM_DIR}/linker/57xx_ram.ld
```

## Adding New Linker Scripts

To add a linker script for a new board:

1. Place the linker script in `platform/linker/` with naming convention:
   - `{mcu}_{type}.ld` (e.g., `mpc5775k_flash.ld`)
   - Or series-based: `57xx_flash.ld`, `56xx_flash.ld`

2. Update the board JSON to reference it:
```json
{
  "build": {
    "mcu": "mpc5775k",
    "linker_script": "mpc5775k_flash.ld"
  }
}
```

3. The builder will automatically find and use it.

