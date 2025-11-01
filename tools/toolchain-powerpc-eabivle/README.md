# Toolchain Package: toolchain-powerpc-eabivle

PowerPC EABI VLE toolchain package for PlatformIO.

## Golden Source

The official toolchain release is hosted on GitHub:

**Release:** v0.0.1  
**File:** `gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip`  
**URL:** https://github.com/dapperfu/platform-nxppowerpc/releases/download/v.0.0.1/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip

## Local Zip File Support

If you have a local zip file with the toolchain binaries, you can configure PlatformIO to use it:

1. Place the toolchain zip file in one of these locations:
   - `platform-nxppowerpc/tools/toolchain-powerpc-eabivle/toolchain/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip`
   - `~/.platformio/packages/toolchain-powerpc-eabivle-source/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip`

2. Update `package.json` to use `file://` URL pointing to your local zip file:

```json
"urls": {
  "linux_x86_64": "file:///absolute/path/to/your/toolchain.zip"
}
```

3. Reinstall the platform package:
```bash
pio pkg uninstall toolchain-powerpc-eabivle
pio pkg install toolchain-powerpc-eabivle
```

Alternatively, you can use the `configure-local-toolchain.py` script which will automatically detect and configure a local zip file.

## Package Information

- **Version:** 4.9.4.2724867 (GCC 4.9.4)
- **Target:** PowerPC e200 EABI VLE
- **Platform:** Linux x86_64
- **License:** GPL-3.0

## Contents

- `powerpc-eabivle-gcc` - C compiler
- `powerpc-eabivle-g++` - C++ compiler  
- `powerpc-eabivle-ar` - Archiver
- `powerpc-eabivle-objcopy` - Object file converter
- `powerpc-eabivle-size` - Size utility
- Standard GCC toolchain utilities

## Installation

This package should be automatically installed by PlatformIO when using the platform. If automatic installation fails, you can:

1. Download the toolchain from the golden source URL above
2. Extract and place in a standard location
3. The platform builder will detect it automatically

## License

This toolchain is provided under GPL-3.0 license as part of the GCC toolchain.
