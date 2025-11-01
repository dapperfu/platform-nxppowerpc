# Quick Start: Using Local Toolchain Zip

If you have a toolchain zip file and want PlatformIO to use it instead of downloading from the internet:

## Quick Setup (3 steps)

### 1. Place your zip file here:
```bash
mkdir -p platform-nxppowerpc/tools/toolchain-powerpc-eabivle/toolchain
# Copy your zip file to:
# /projects/platformio/platform-nxppowerpc/tools/toolchain-powerpc-eabivle/toolchain/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip
```

Or place it anywhere in `/projects/platformio/` - the script will find it.

### 2. Run the configuration script:
```bash
cd platform-nxppowerpc/tools/toolchain-powerpc-eabivle
python3 configure-local-toolchain.py
```

### 3. Reinstall the toolchain package:
```bash
cd platform-nxppowerpc-examples/mpc5744p/memory/edma-mpc5744p
pio pkg uninstall toolchain-powerpc-eabivle
pio pkg install toolchain-powerpc-eabivle
```

That's it! Now `pio run` will use your local toolchain zip file.

## Alternative: System Toolchain

If you prefer to use a system-installed toolchain, you can place the extracted toolchain in:
- `/opt/powerpc-eabivle/bin/`
- `/usr/local/powerpc-eabivle/bin/`

The platform builder will automatically detect it.

