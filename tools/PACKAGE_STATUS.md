# Platform Package Status

## Package Readiness for Release

### ✅ Ready to Release

1. **framework-freertos-nxp-mpc57xx** (v9.0.0.1)
   - ✅ Has GitHub release URL
   - ✅ SHA256 calculated and set
   - ✅ Auto-downloads from GitHub releases
   - 📦 Package: Downloads from: `https://github.com/dapperfu/platform-nxppowerpc/releases/download/v.0.0.1/freertos-9.0.0_MPC57XXX_public_rel_1.zip`

2. **toolchain-powerpc-eabivle** (v4.9.4.2724867)
   - ✅ Has SHA256: `1b16bd350c52839d2acfa3c873f0f379c17e56b93a9be389f5831cb44d7729c3`
   - ⚠️ URL: Currently `file:///tmp/...` (needs GitHub release URL)
   - 📦 Package: Must be uploaded to GitHub releases

3. **tool-pegdbserver-power** (v1.7.2.201709281658)
   - ✅ Has SHA256: `b2d88f0433ea1803ffd6b41e9b5611390bd7617106be49b167713b0bc108c63d`
   - ⚠️ URL: Currently `file:///tmp/...` (needs GitHub release URL)
   - 📦 Package: Must be uploaded to GitHub releases

### ⚠️ Needs Building from S32DS Installer

4. **library-ewl-powerpc-eabivle** (v2.0.0)
   - ⚠️ Has PLACEHOLDER_URL and PLACEHOLDER_SHA256
   - 📋 Action: Build from S32DS installer using:
     ```bash
     cd tools/library-ewl-powerpc-eabivle
     python3 build.py /path/to/extracted/S32DS/installer --update-package-json
     ```
   - 📦 Package: Must be uploaded to GitHub releases after building

## Package Build Scripts

All packages have individual build scripts:

- ✅ `tools/toolchain-powerpc-eabivle/build.py`
- ✅ `tools/tool-pegdbserver-power/build.py`
- ✅ `tools/library-ewl-powerpc-eabivle/build.py`
- ✅ `tools/framework-freertos-nxp-mpc57xx/build.py` (downloads from GitHub)

## Master Build Script

The `build_all_packages.py` script currently builds:
- ✅ toolchain-powerpc-eabivle
- ✅ tool-pegdbserver-power
- ⚠️ **Missing**: library-ewl-powerpc-eabivle (should be added)

Note: `framework-freertos-nxp-mpc57xx` is not in build_all_packages.py because it downloads from GitHub releases, not from S32DS installer.

## Release Checklist

Before releasing platform:

- [ ] Upload `toolchain-powerpc-eabivle.zip` to GitHub releases
- [ ] Update `toolchain-powerpc-eabivle/package.json` with GitHub release URL
- [ ] Upload `tool-pegdbserver-power.zip` to GitHub releases
- [ ] Update `tool-pegdbserver-power/package.json` with GitHub release URL
- [ ] Build `library-ewl-powerpc-eabivle.zip` from S32DS installer
- [ ] Upload `library-ewl-powerpc-eabivle.zip` to GitHub releases
- [ ] Update `library-ewl-powerpc-eabivle/package.json` with GitHub release URL
- [ ] `framework-freertos-nxp-mpc57xx` is already set up ✅

## Platform Configuration

All packages are registered in `platform.json`:
- ✅ toolchain-powerpc-eabivle
- ✅ library-ewl-powerpc-eabivle
- ✅ tool-pegdbserver-power
- ✅ framework-freertos-nxp-mpc57xx
- ✅ framework-freertos (mainline, optional)

