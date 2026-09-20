#!/usr/bin/env bash
# Build the EWL C99 / libm / runtime archives used by ewl_c9x_noio.specs
# for MPC5744P (e200z4 hard-float VLE): libc99.a, libm.a, librt.a.
set -euo pipefail

EWL_DIR="${VLE_EWL_DIR:-${HOME}/e200_ewl2}"
if [[ ! -d "${EWL_DIR}/EWL_C" ]]; then
    echo "EWL source not found at ${EWL_DIR}. Set VLE_EWL_DIR." >&2
    exit 1
fi

TOOLS="${POWERPC_TOOLS:-}"
if [[ -z "${TOOLS}" ]]; then
    for cand in \
        "${HOME}/.platformio/packages/toolchain-powerpc-eabivle/powerpc-eabivle-4_9" \
        /projects/mpc5744p/S32DS/build_tools/powerpc-eabivle-4_9; do
        if [[ -x "${cand}/bin/powerpc-eabivle-gcc" ]]; then
            TOOLS="${cand}"
            break
        fi
    done
fi
if [[ -z "${TOOLS}" || ! -x "${TOOLS}/bin/powerpc-eabivle-gcc" ]]; then
    echo "powerpc-eabivle-gcc not found. Set POWERPC_TOOLS to the toolchain root." >&2
    exit 1
fi

export POWERPC_TOOLS="${TOOLS}"
export PATH="${TOOLS}/bin:${PATH}"

echo "Building EWL e200z4/fp from ${EWL_DIR} with ${TOOLS}"

make -C "${EWL_DIR}/EWL_C" -f EWL_C.GCC.mak \
    PLATFORM=PA \
    POWERPC_TOOLS="${TOOLS}" \
    TARGET=e200z4/fp/libc99

make -C "${EWL_DIR}/EWL_C" -f EWL_C.GCC.mak \
    PLATFORM=PA \
    POWERPC_TOOLS="${TOOLS}" \
    TARGET=e200z4/fp/libm

make -C "${EWL_DIR}/EWL_Runtime" -f EWL_Runtime.GCC.mak \
    PLATFORM=PA \
    POWERPC_TOOLS="${TOOLS}" \
    TARGET=e200z4/fp/librt

DEST="${EWL_DIR}/lib/e200z4/fp"
for lib in libc99.a libm.a librt.a; do
    if [[ ! -f "${DEST}/${lib}" ]]; then
        echo "Missing ${DEST}/${lib}" >&2
        exit 1
    fi
    echo "OK ${DEST}/${lib} ($(wc -c < "${DEST}/${lib}") bytes)"
done
