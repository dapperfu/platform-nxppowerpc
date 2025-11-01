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

To build this package manually:

```bash
cd build_platformio_pegdbserver
python3 build_pio_package.py --s32ds-root /path/to/s32ds --output-dir /path/to/output
```

Or using the Makefile:

```bash
cd build_platformio_pegdbserver
make package S32DS_ROOT=/path/to/s32ds
```

## Local Testing

For local testing with a specific S32DS installation, the package can be built and configured to use a local file:// URL:

```bash
# Build the package from local S32DS installation
cd /projects/platformio/platform-nxppowerpc
python3 build_platformio_pegdbserver/build_pio_package.py \
  --s32ds-root /home/jed/NXP/S32DS_Power_v2017.R1 \
  --output-dir tools/tool-pegdbserver-power/package \
  --no-archive

# Create zip archive
cd tools/tool-pegdbserver-power/package
zip -r tool-pegdbserver-power.zip tool-pegdbserver-power/

# Update package.json to use file:// URL (currently configured for local testing)
# The package.json urls field should point to:
# file:///projects/platformio/platform-nxppowerpc/tools/tool-pegdbserver-power/package/tool-pegdbserver-power.zip
```

The package is currently configured for local testing using the S32DS installation at `/home/jed/NXP/S32DS_Power_v2017.R1`.

## License

This package contains proprietary software from P&E Microcomputer Systems. Please refer to the S32 Design Studio license terms for usage restrictions.

