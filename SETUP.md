# Platform Setup Guide

This guide explains how to set up the NXP PowerPC VLE platform for PlatformIO, including using local toolchain binaries.

## Platform Installation

The platform must be installed before use. You can install it from the local directory:

```bash
cd <your-workspace-directory>
source venv/bin/activate
pio platform install <path-to-platform-nxppowerpc>
```

Or from a git repository:

```bash
pio platform install https://github.com/dapperfu/platform-nxppowerpc.git
```

## Using Local Toolchain Zip File

If you have a local zip file with the PowerPC toolchain binaries, follow these steps:

### Step 1: Place the Zip File

Place your toolchain zip file in one of these locations (in order of preference):

1. `platform-nxppowerpc/tools/toolchain-powerpc-eabivle/toolchain/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip`
2. `platform-nxppowerpc/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip`
3. `gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip` (in current directory or subdirectories)

Or any filename matching `*powerpc*.zip` or `*eabivle*.zip` in the platform directory tree.

### Step 2: Configure Package to Use Local File

Run the configuration script:

```bash
cd <your-workspace-directory>/platform-nxppowerpc/tools/toolchain-powerpc-eabivle
python3 configure-local-toolchain.py
```

This will automatically:
- Find your zip file
- Update `package.json` to use a `file://` URL
- Print instructions for the next steps

### Step 3: Reinstall the Toolchain Package

After configuration, reinstall the toolchain package:

```bash
cd <your-workspace-directory>/platform-nxppowerpc-examples/mpc5744p/memory/edma-mpc5744p
pio pkg uninstall toolchain-powerpc-eabivle
pio pkg install toolchain-powerpc-eabivle
```

### Alternative: Manual Configuration

If you prefer to configure manually:

1. Edit `platform-nxppowerpc/tools/toolchain-powerpc-eabivle/package.json`
2. Update the `urls.linux_x86_64` field to use a `file://` URL:

```json
"urls": {
  "linux_x86_64": "file:///absolute/path/to/your/toolchain.zip"
}
```

3. Reinstall the package as shown above

## Verification

To verify everything is working:

```bash
cd <your-workspace-directory>/platform-nxppowerpc-examples/mpc5744p/memory/edma-mpc5744p
pio run
```

You should see either:
- `Using PlatformIO toolchain package: ...` (if using PlatformIO package)
- `Using system toolchain: ...` (if using system-installed toolchain)

## Troubleshooting

### Platform Not Found Error

If you see:
```
UnknownPackageError: Could not find the package with 'nxppowerpc' requirements
```

Install the platform first:
```bash
pio platform install <path-to-platform-nxppowerpc>
```

### Toolchain Not Found

If the toolchain is not found:

1. Verify the zip file is in one of the expected locations
2. Run the configuration script: `python3 configure-local-toolchain.py`
3. Reinstall the toolchain package
4. Check that the zip file contains `bin/powerpc-eabivle-gcc` after extraction

### Build Errors

If you see build errors, verify:
- The platform is installed: `pio platform list | grep nxppowerpc`
- The toolchain package is installed: `pio pkg list | grep toolchain-powerpc`
- The project has a valid linker script (check `platformio.ini`)

