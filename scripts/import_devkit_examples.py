#!/usr/bin/env python3
"""Import the 19 DEVKIT-MPC5744P examples as in-repo PlatformIO pairings.

Sources (first match wins):
  1. DEVKIT-Makefile/Examples/MPC5744P/<Name>
  2. platform-nxppowerpc-examples/mpc5744p/**/<slug>

Usage:
  python3 scripts/import_devkit_examples.py
  python3 scripts/import_devkit_examples.py /path/to/DEVKIT-Makefile
"""

from __future__ import print_function

import os
import shutil
import sys

EXAMPLES = [
    ("Hello_World", "basics/hello-world"),
    ("Hello_World_PLL", "basics/hello-world-pll"),
    ("Hello_World_PLL_Interrupt", "basics/hello-world-pll-interrupt"),
    ("ADC_MPC5744P", "sensors/adc-mpc5744p"),
    ("eDMA_MPC5744P", "memory/edma-mpc5744p"),
    ("eTimer_MPC5744P", "timing/etimer-mpc5744p"),
    ("eTimer_Freq_Measurement_MPC5744P", "timing/etimer-freq-measurement-mpc5744p"),
    ("FlexCAN_MPC5744P", "communication/flexcan-mpc5744p"),
    ("LINFlexD_UART_MPC5744P", "communication/linflexd-uart-mpc5744p"),
    ("LINFlexD_LIN_Master_MPC5744P", "communication/linflexd-lin-master-mpc5744p"),
    ("LINFlexD_LIN_Slave_MPC5744P", "communication/linflexd-lin-slave-mpc5744p"),
    ("SPI_MPC5744P", "communication/spi-mpc5744p"),
    ("SPI_DMA_MPC5744P", "communication/spi-dma-mpc5744p"),
    ("FLASH_ECC_Error_Injection_MPC5744P", "memory/flash-ecc-error-injection-mpc5744p"),
    ("FCCU_MPC5744P", "safety/fccu-mpc5744p"),
    ("TSENS_MPC5744P", "sensors/tsens-mpc5744p"),
    ("XBIC_DMA_MPC5744P", "memory/xbic-dma-mpc5744p"),
    ("LP_STOP_MPC5744P", "power-management/lp-stop-mpc5744p"),
    ("SIUL_RegisterProtection_MPC5744P", "safety/siul-registerprotection-mpc5744p"),
]

PLATFORMIO_INI = """\
[env:devkit-mpc5744p]
platform = symlink://../..
board = devkit-mpc5744p
framework = baremetal
board_build.ewl_dir = /projects/mpc5744p/S32DS/build_tools/e200_ewl2
"""


def copy_tree(src, dst):
    if os.path.isdir(dst):
        shutil.rmtree(dst)
    shutil.copytree(src, dst)


def find_source(name, slug, extra_roots):
    for root in extra_roots:
        makefile = os.path.join(root, "Examples", "MPC5744P", name)
        if os.path.isdir(os.path.join(makefile, "src")):
            return makefile
        examples_repo = os.path.join(root, "mpc5744p", slug)
        if os.path.isdir(os.path.join(examples_repo, "src")):
            return examples_repo
        # root itself is the examples repo
        direct = os.path.join(root, slug) if not slug.startswith("/") else slug
        if os.path.isdir(os.path.join(direct, "src")):
            return direct
    return None


def import_example(src_example, dst_example):
    os.makedirs(dst_example, exist_ok=True)
    for part in ("src", "include"):
        src_part = os.path.join(src_example, part)
        if not os.path.isdir(src_part):
            raise SystemExit("Missing %s in %s" % (part, src_example))
        copy_tree(src_part, os.path.join(dst_example, part))
        # Makefile examples do not use spaces in filenames.
        dst_part = os.path.join(dst_example, part)
        for fname in os.listdir(dst_part):
            safe = fname.replace(" ", "").replace("+", "_")
            if safe != fname:
                os.rename(os.path.join(dst_part, fname), os.path.join(dst_part, safe))
    common = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "examples", "_common", "intc_sw_handlers.S")
    dest_handler = os.path.join(dst_example, "src", "intc_sw_handlers.S")
    if os.path.isfile(common) and os.path.isfile(dest_handler):
        shutil.copy2(common, dest_handler)
    with open(os.path.join(dst_example, "platformio.ini"), "w") as handle:
        handle.write(PLATFORMIO_INI)


def main():
    extra_roots = []
    if len(sys.argv) > 1:
        extra_roots.append(os.path.abspath(sys.argv[1]))
    extra_roots.extend([
        os.environ.get("DEVKIT_MAKEFILE_DIR"),
        os.environ.get("NXPOWERPC_EXAMPLES_DIR"),
        "/tmp/deps/DEVKIT-Makefile",
        "/tmp/deps/platform-nxppowerpc-examples",
        os.path.normpath(os.path.join(os.path.dirname(__file__), "..", "..", "DEVKIT-Makefile")),
        os.path.normpath(os.path.join(os.path.dirname(__file__), "..", "..", "platform-nxppowerpc-examples")),
    ])
    extra_roots = [r for r in extra_roots if r]

    platform_root = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
    os.makedirs(os.path.join(platform_root, "examples"), exist_ok=True)

    for name, slug in EXAMPLES:
        src = find_source(name, slug, extra_roots)
        if not src:
            raise SystemExit("Could not find sources for %s (%s)" % (name, slug))
        dst = os.path.join(platform_root, "examples", name)
        import_example(src, dst)
        print("Imported %s from %s" % (name, src))
    print("Imported %d examples." % len(EXAMPLES))


if __name__ == "__main__":
    main()
