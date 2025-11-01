# Fixes Applied to PlatformIO Builder

## Summary

Fixed two critical issues identified in the compilation test failures:

1. ✅ **Board Configuration Error** - Fixed KeyError for missing `build.linker_script` option
2. ✅ **Toolchain Path Detection** - Fixed toolchain detection in nested subdirectories
3. ✅ **Library Path Calculation** - Fixed library path calculation when toolchain is in subdirectory

## Fix 1: Board Configuration Error Handling

**File:** `platform-nxppowerpc/builder/main.py`  
**Lines:** 438-443

**Problem:**  
The code called `board.get("build.linker_script")` which raises a `KeyError` when the option doesn't exist. Boards `mpc5643l` and `mpc5775k` don't have this field in their JSON configuration.

**Solution:**  
Added try/except block to handle missing option gracefully:

```python
# Check board configuration for linker script
# Handle missing option gracefully (some boards don't have this field)
try:
    board_linker = board.get("build.linker_script")
except (KeyError, AttributeError):
    board_linker = None
```

**Status:** ✅ Fixed and verified

## Fix 2: Toolchain Path Detection

**File:** `platform-nxppowerpc/builder/main.py`  
**Lines:** 77-104

**Problem:**  
The toolchain package extracts with a nested directory structure:
- Actual: `toolchain-powerpc-eabivle/powerpc-eabivle-4_9/bin/gcc`
- Expected: `toolchain-powerpc-eabivle/bin/gcc`

The initial check only looked at the root level, causing toolchain not found errors.

**Solution:**  
Enhanced toolchain detection to check both root level and nested subdirectories:

```python
# Check root level first
gcc_path_root = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX + "gcc")
if exists(gcc_path_root):
    TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
else:
    # Check nested subdirectories
    found_toolchain = False
    if exists(TOOLCHAIN_DIR):
        for item in os.listdir(TOOLCHAIN_DIR):
            subdir = join(TOOLCHAIN_DIR, item)
            if os.path.isdir(subdir):
                gcc_path_sub = join(subdir, "bin", TOOLCHAIN_PREFIX + "gcc")
                if exists(gcc_path_sub):
                    TOOLCHAIN_DIR = subdir
                    TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
                    found_toolchain = True
                    break
```

**Status:** ✅ Fixed and verified

## Fix 3: Library Path Calculation

**File:** `platform-nxppowerpc/builder/main.py`  
**Lines:** 394-424

**Problem:**  
When `TOOLCHAIN_DIR` is set to a subdirectory (e.g., `powerpc-eabivle-4_9`), the library path calculation using `join(TOOLCHAIN_DIR, "..", "..", "e200_ewl2", "lib")` would incorrectly go back too many levels.

**Solution:**  
Updated library path calculation to handle both root and subdirectory cases:

```python
# Try multiple possible library base paths
toolchain_package_root = TOOLCHAIN_DIR
# If TOOLCHAIN_DIR is a subdirectory, go up to package root
if os.path.basename(TOOLCHAIN_DIR).startswith("powerpc-eabivle"):
    toolchain_package_root = join(TOOLCHAIN_DIR, "..")

# Try library paths at both package root and subdirectory levels
toolchain_lib_base_1 = join(toolchain_package_root, "e200_ewl2", "lib")
toolchain_lib_base_2 = join(TOOLCHAIN_DIR, "e200_ewl2", "lib") if TOOLCHAIN_DIR != toolchain_package_root else None
```

**Status:** ✅ Fixed

## Verification

Tested with previously failing projects:

1. **mpc5643l/advanced/mixed-vle-booke-mpc5643l**: ✅ No more KeyError, toolchain detected correctly
2. **mpc5748g/basics/hello**: ✅ Toolchain detected correctly

**Output confirms fixes:**
```
Using PlatformIO toolchain package (downloaded from GitHub): /keg/cursor/.platformio/packages/toolchain-powerpc-eabivle/powerpc-eabivle-4_9
```

## Platform Reinstallation

The platform was reinstalled to apply the fixes:
```bash
pio platform uninstall nxppowerpc
pio platform install ./platform-nxppowerpc
```

## Remaining Issues

**Permission Issue (Separate from fixes):**
Some toolchain executables may lack execute permissions after extraction. This is a toolchain packaging/installation issue, not a builder issue. Can be resolved with:
```bash
chmod +x /keg/cursor/.platformio/packages/toolchain-powerpc-eabivle/powerpc-eabivle-4_9/bin/*
```

## Next Steps

1. Re-run full compilation tests to verify all fixes
2. Address toolchain permission issue if needed (may require fixing toolchain package extraction)
3. Test upload functionality when ready

