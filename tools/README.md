# PlatformIO Tooling

This directory contains tools for building PlatformIO packages from extracted S32 Design Studio installers.

## Overview

The tools in this directory can extract and package:
- **Toolchain**: GCC PowerPC EABI VLE toolchain (`toolchain-powerpc-eabivle`)
- **PEGDBServer**: P&E Micro GDB Server for debugging (`tool-pegdbserver-power`)

## Tools

### 1. `analyze_s32ds_installer.py`

Analyzes an extracted S32DS installer and identifies all components.

**Usage:**
```bash
python3 tools/analyze_s32ds_installer.py <installer_root> [--json <output.json>]
```

**Example:**
```bash
python3 tools/analyze_s32ds_installer.py /home/jed/Downloads/extract_S32DS_Power_Linux/installer
python3 tools/analyze_s32ds_installer.py /home/jed/Downloads/extract_S32DS_Power_Linux/installer --json analysis.json
```

### 2. `extract_toolchain.py`

Extracts and packages the GCC toolchain from the installer.

**Usage:**
```bash
python3 tools/extract_toolchain.py <installer_root> <output_dir> [package_name]
```

**Example:**
```bash
python3 tools/extract_toolchain.py \
  /home/jed/Downloads/extract_S32DS_Power_Linux/installer \
  /tmp/toolchain-package
```

### 3. `extract_pegdbserver.py`

Extracts and packages the PEGDBServer debugger tool from the installer.

**Usage:**
```bash
python3 tools/extract_pegdbserver.py <installer_root> <output_dir> [package_name]
```

**Example:**
```bash
python3 tools/extract_pegdbserver.py \
  /home/jed/Downloads/extract_S32DS_Power_Linux/installer \
  /tmp/pegdbserver-package
```

### 4. `build_all_packages.py`

Master script that coordinates building all packages.

**Usage:**
```bash
python3 tools/build_all_packages.py <installer_root> [options]
```

**Options:**
- `--output, -o <dir>`: Output directory (default: `/tmp/pio-packages`)
- `--platform-root, -p <dir>`: Platform root directory (updates package.json files)
- `--toolchain-only`: Only build toolchain package
- `--pegdbserver-only`: Only build pegdbserver package

**Example:**
```bash
# Build all packages
python3 tools/build_all_packages.py \
  /home/jed/Downloads/extract_S32DS_Power_Linux/installer \
  --output /tmp/pio-packages \
  --platform-root /projects/platformio/platform-nxppowerpc

# Build only toolchain
python3 tools/build_all_packages.py \
  /home/jed/Downloads/extract_S32DS_Power_Linux/installer \
  --toolchain-only \
  --output /tmp/pio-packages
```

## Using Makefiles

Each tool package directory has a Makefile with a `from-installer` target.

### Toolchain Package

```bash
cd tools/toolchain-powerpc-eabivle
make from-installer S32DS_INSTALLER_ROOT=/home/jed/Downloads/extract_S32DS_Power_Linux/installer
```

This will:
1. Extract the toolchain from the installer
2. Create a PlatformIO package zip file
3. Update `package.json` with SHA256 and version

### PEGDBServer Package

```bash
cd tools/tool-pegdbserver-power
make from-installer S32DS_INSTALLER_ROOT=/home/jed/Downloads/extract_S32DS_Power_Linux/installer
```

## Package Structure

### Toolchain Package

```
toolchain-powerpc-eabivle.zip
└── toolchain-powerpc-eabivle/
    └── powerpc-eabivle-4_9/
        ├── bin/
        │   ├── powerpc-eabivle-gcc
        │   ├── powerpc-eabivle-g++
        │   └── ...
        ├── include/
        ├── lib/
        └── ...
```

### PEGDBServer Package

```
tool-pegdbserver-power.zip
└── tool-pegdbserver-power/
    └── tools/
        └── pegdbserver/
            ├── bin/
            │   └── pegdbserver_power_console
            └── gdi/
                └── P&E/
                    ├── *.add (device definitions)
                    ├── *.pcp (flash algorithms)
                    └── ...
```

## Workflow

1. **Extract S32DS Installer** (separate project)
   - Extract the S32 Studio installer to a directory (e.g., `/home/jed/Downloads/extract_S32DS_Power_Linux/installer`)

2. **Analyze Installer** (optional)
   ```bash
   python3 tools/analyze_s32ds_installer.py /home/jed/Downloads/extract_S32DS_Power_Linux/installer
   ```

3. **Build Packages**
   ```bash
   # Option 1: Use master script (recommended)
   python3 tools/build_all_packages.py \
     /home/jed/Downloads/extract_S32DS_Power_Linux/installer \
     --output /tmp/pio-packages \
     --platform-root /projects/platformio/platform-nxppowerpc
   
   # Option 2: Use individual Makefiles
   cd tools/toolchain-powerpc-eabivle
   make from-installer S32DS_INSTALLER_ROOT=/home/jed/Downloads/extract_S32DS_Power_Linux/installer
   
   cd ../tool-pegdbserver-power
   make from-installer S32DS_INSTALLER_ROOT=/home/jed/Downloads/extract_S32DS_Power_Linux/installer
   ```

4. **Update Package URLs** (if using local packages)
   - The scripts automatically update `package.json` files to use `file://` URLs
   - Or manually edit `package.json` to point to the zip file location

5. **Test Packages**
   ```bash
   # Install platform
   pio platform install /projects/platformio/platform-nxppowerpc
   
   # PlatformIO will automatically install packages from package.json
   ```

## Requirements

- Python 3.6+
- Standard library modules (no external dependencies)
- Standard Unix tools: `zip`, `unzip`, `sha256sum` (or `shasum`)

