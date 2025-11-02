# Tool Package: tool-pegdbserver-power

P&E Microcomputer Systems GDB Server for Power Architecture microcontrollers.

## Source

This tool package is sourced from the git repository using the builder located in the `build_platformio_pegdbserver` directory.

**Repository:** https://github.com/dapperfu/platform-nxppowerpc.git  
**Builder:** `build_platformio_pegdbserver/build_pio_package.py`  
**Git Source:** The package.json references the git repository as the source using GitHub archive URLs.

## Build Process

The package is built from the P&E GDB Server components found in S32 Design Studio installations. The build process:

1. Extracts the `pegdbserver_power_console` binary
2. Copies device definition files (`.add`)
3. Copies flash programming algorithms (`.pcp`)
4. Includes all required shared libraries and XML configuration files

## Package Information

- **Version:** 1.7.2.201709281658
- **P&E DLL Version:** v651
- **Build Date:** 170928
- **Source:** S32 Design Studio for Power Architecture 2017.R1
- **License:** Proprietary (P&E Microcomputer Systems)

## Contents

After installation, the package contains:

- `tools/pegdbserver/bin/pegdbserver_power_console` - Main GDB server executable
- `tools/pegdbserver/gdi/P&E/` - Device definitions and flash algorithms
  - `pedebug_ppcnexus_*.add` - Device definition files (65+ files)
  - `nxp_*.pcp` - Flash programming algorithm files (200+ files)
  - `*.so` - Required shared libraries
  - `*.xml` - Device target XML files
  - `*.mac` - Macro files for device initialization

## Usage

This tool is automatically installed by PlatformIO when using the `nxppowerpc` platform with debugging/uploading enabled.

### Command Line Arguments

- `-device=<name>` - Target device (e.g., MPC5748G, MPC5744P)
- `-interface=OPENSDA` - Hardware interface type
- `-port=<string>` - Hardware port (e.g., USB1)
- `-speed=<kHz>` - JTAG speed (default: 5000)
- `-startserver` - Start GDB server
- `-serverport=<n>` - Server TCP port (default: 7224)
- `-gdbmiport=<n>` - GDB/MI TCP port (default: 6224)
- `-singlesession` - Allow only one session
- `-devicelist` - List supported devices
- `-getportlist` - List available hardware ports

## Building from Source

To build this package from S32DS installer:

```bash
cd tools/tool-pegdbserver-power
python3 build.py /path/to/extracted/s32ds/installer --update-package-json
```

Or use the master build script:

```bash
python3 tools/build_all_packages.py \
  /path/to/extracted/s32ds/installer \
  --output /tmp/pio-packages \
  --platform-root /projects/platformio/platform-nxppowerpc
```

## Building from Extracted Installer

The package can be built directly from an extracted S32DS installer:

```bash
# Build the package from extracted installer
cd tools/tool-pegdbserver-power
python3 build.py \
  /home/jed/Downloads/extract_S32DS_Power_Linux/installer \
  --output build \
  --update-package-json

# The package.json will be updated with file:// URL and SHA256
```

## License

This package contains proprietary software from P&E Microcomputer Systems. Please refer to the S32 Design Studio license terms for usage restrictions.

