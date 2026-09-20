# Quick start: DEVKIT-MPC5744P

## 1. Toolchain and EWL

- Toolchain package: `toolchain-powerpc-eabivle` (GCC 4.9.4 EABI VLE)
- EWL: set `VLE_EWL_DIR` or place `e200_ewl2` next to the toolchain / in `$HOME/e200_ewl2`
- Link uses EWL `ewl_c9x_noio.specs` (`-lc99 -lm -lrt -lgcc`), not newlib `-lc`
- If the tree has headers/specs but no archives, build them: `./scripts/build_ewl_e200z4.sh`

If `libexec/.../cc1` is not executable after unzip, `chmod +x` that file.

## 2. Build Hello_World

```bash
cd examples/Hello_World
pio run
```

## 3. Build every 1:1 example

```bash
./scripts/build_all_examples.sh
```

## 4. Flash

Not wired yet.
