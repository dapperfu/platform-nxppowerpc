# Framework Package: framework-freertos-nxp-mpc57xx

NXP FreeRTOS 9.0.0 for MPC57xx series microcontrollers with PowerPC_Z4 port support.

## Description

This package provides the NXP-optimized FreeRTOS 9.0.0 release specifically tailored for MPC57xx series microcontrollers. It includes:

- **PowerPC_Z4 port**: Optimized for e200z4 cores with VLE (Variable Length Encoding) instruction support
- **MPC57xx hardware support**: Integrated interrupt controller (INTC) support and hardware-specific optimizations
- **FreeRTOS 9.0.0**: Stable, validated release from NXP

## Key Features

- VLE instruction set support (`se_nop`, `se_sc`, `se_addi`, etc.)
- MPC57xx interrupt controller integration
- e200z4 core optimizations (HID0.ICR, SPRG1 usage)
- Hardware-assisted context switching
- Assembly implementation with VLE mnemonics

## Usage

This framework is automatically used when you specify `framework = freertos` in your `platformio.ini`:

```ini
[env:mpc5748g]
platform = nxppowerpc
board = mpc5748g
framework = freertos
```

The platform builder will automatically use this package if installed, or download it from GitHub releases if not found.

## Package Structure

```
framework-freertos-nxp-mpc57xx/
└── FreeRTOS/
    ├── Source/
    │   ├── include/
    │   ├── portable/
    │   │   └── GCC/
    │   │       └── PowerPC_Z4/  ← NXP-specific port
    │   └── ...
    └── ...
```

## Building the Package

```bash
cd tools/framework-freertos-nxp-mpc57xx
python3 build.py --update-package-json
```

This will:
1. Download the NXP FreeRTOS release from GitHub
2. Extract and repackage it for PlatformIO
3. Calculate SHA256 hashes
4. Update `package.json` with the SHA256

## License

GPL-2.0 WITH FreeRTOS-exception (as per NXP FreeRTOS release)

## References

- NXP FreeRTOS Release: https://github.com/dapperfu/platform-nxppowerpc/releases/download/v.0.0.1/freertos-9.0.0_MPC57XXX_public_rel_1.zip
- Platform FreeRTOS Builder: `builder/frameworks/freertos.py`

