# NXP PowerPC VLE: PlatformIO platform for DEVKIT-MPC5744P

Development platform for the **NXP DEVKIT-MPC5744P** board (MPC5744P / e200z4) using PlatformIO and the PowerPC EABI VLE toolchain.

This milestone supports **one board** and **one CPU**. `pio run` produces flash images (ELF, HEX, BIN, S19). Upload via OpenSDA is not implemented yet.

## Supported target

| Board | MCU | CPU | Clock | Flash | RAM | Framework |
|-------|-----|-----|-------|-------|-----|-----------|
| DEVKIT-MPC5744P | MPC5744P | e200z4 | 160 MHz | 2 MB (text) | 384 KB | baremetal |

## Build

```bash
cd examples/Hello_World
pio run
```

Outputs under `.pio/build/devkit-mpc5744p/`: `firmware.elf`, `.hex`, `.bin`, `.s19`, `.map`.

```bash
./scripts/build_all_examples.sh
```

Upload (`pio run -t upload`) is TBD. See [docs/OPENSDA_FLASHER_ANALYSIS.md](docs/OPENSDA_FLASHER_ANALYSIS.md).

## Examples

The 19 DEVKIT Makefile / NXP example pairings live under `examples/`. Re-sync:

```bash
python3 scripts/import_devkit_examples.py
```

## License

Apache License 2.0
