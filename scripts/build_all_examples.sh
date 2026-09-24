#!/usr/bin/env bash
# Build every in-repo DEVKIT-MPC5744P PlatformIO example and check artifacts.
set -euo pipefail

export PATH="${HOME}/.local/bin:${PATH}"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
FAILED=0
COUNT=0
OBJDUMP=""
for cand in \
    "${HOME}/.platformio/packages/toolchain-powerpc-eabivle/powerpc-eabivle-4_9/bin/powerpc-eabivle-objdump"; do
    if [[ -x "${cand}" ]]; then
        OBJDUMP="${cand}"
        break
    fi
done

for ini in "${ROOT}"/examples/*/platformio.ini; do
    example="$(dirname "$ini")"
    name="$(basename "$example")"
    COUNT=$((COUNT + 1))
    echo "========== Building ${name} =========="
    if ! (cd "$example" && pio run); then
        echo "FAIL ${name} (pio run)"
        FAILED=1
        continue
    fi
    build="${example}/.pio/build/devkit-mpc5744p"
    missing=0
    for art in firmware.elf firmware.hex firmware.bin firmware.s19 firmware.map; do
        if [[ ! -f "${build}/${art}" ]]; then
            echo "FAIL ${name}: missing ${art}"
            missing=1
        fi
    done
    if [[ "${missing}" -ne 0 ]]; then
        FAILED=1
        continue
    fi
    if [[ -n "${OBJDUMP}" ]]; then
        sections="$("${OBJDUMP}" -h "${build}/firmware.elf")"
        for sec in .rchw .cpu0_reset_vector .startup; do
            if ! grep -q "${sec}" <<<"${sections}"; then
                echo "FAIL ${name}: missing section ${sec}"
                FAILED=1
                missing=1
            fi
        done
    fi
    if [[ "${missing}" -eq 0 ]]; then
        echo "OK ${name}"
    fi
done

echo "Built ${COUNT} examples."
if [[ "${FAILED}" -ne 0 ]]; then
    echo "One or more examples failed."
    exit 1
fi
echo "All examples built."
