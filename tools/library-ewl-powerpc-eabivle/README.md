# Library Package: library-ewl-powerpc-eabivle

Embedded Workbench Library (EWL) runtime libraries for PowerPC e200 cores.

## Description

This package provides the EWL (Embedded Workbench Library) runtime libraries required for linking C and C++ applications compiled with the PowerPC EABI VLE toolchain. EWL provides standard C library implementations optimized for embedded PowerPC e200 cores.

## Key Features

- Standard C library (`libc`) for e200 cores
- Standard C++ library (`libc++`) support
- Math library (`libm`) with hardware floating-point support
- Optimized for e200z0, e200z4, and e200z6 cores
- VLE (Variable Length Encoding) instruction set support
- EABI calling conventions

## Library Structure

```
library-ewl-powerpc-eabivle/
└── e200_ewl2/
    └── lib/
        ├── e200z0/          # e200z0 core libraries
        ├── e200z4/          # e200z4 core libraries
        │   └── spe/         # Signal Processing Engine variant
        └── e200z6/          # e200z6 core libraries (fallback)
```

## Usage

This library is automatically used by the platform when compiling projects. The builder automatically detects and links the appropriate libraries based on the board's CPU variant:

- `e200z0`: Uses `e200z0/` libraries
- `e200z4`: Uses `e200z4/` libraries (or `e200z4/spe/` if SPE variant)
- `e200z6`: Uses `e200z6/` libraries (fallback)

The libraries are linked automatically via:
- `-lm` (math library)
- `-lc` (C standard library)

## Building the Package

```bash
cd tools/library-ewl-powerpc-eabivle
python3 build.py \
  /path/to/extracted/S32DS/installer \
  --output build \
  --update-package-json
```

This will:
1. Extract EWL libraries from the S32DS installer
2. Create a PlatformIO package zip file
3. Calculate SHA256 hash
4. Update `package.json` with SHA256 and version (if `--update-package-json` is used)

## Integration with Platform

The platform builder (`builder/main.py`) automatically:
1. Checks for `library-ewl-powerpc-eabivle` package in PlatformIO packages
2. Adds library search paths based on CPU variant
3. Links standard C and math libraries

## License

Proprietary (NXP/Freescale)

## References

- NXP S32 Design Studio
- PowerPC e200 Core Reference Manuals
- EWL Documentation

