# Copyright 2014-present PlatformIO <contact@platformio.org>
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
FreeRTOS Framework for NXP PowerPC VLE

FreeRTOS real-time operating system support for PowerPC VLE microcontrollers.
This builder configures FreeRTOS source files and port files for PowerPC VLE.
"""

from os.path import isdir, join, exists

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

# Try to get FreeRTOS from package, fallback to local lib directory
FRAMEWORK_DIR = None
try:
    FRAMEWORK_DIR = platform.get_package_dir("framework-freertos")
    if not isdir(FRAMEWORK_DIR):
        FRAMEWORK_DIR = None
except Exception:
    pass

if FRAMEWORK_DIR is None:
    # Try local lib directory using PlatformIO environment expansion
    import os
    project_dir = env.subst("$PROJECT_DIR")
    local_freertos = os.path.join(project_dir, "lib", "FreeRTOS")
    if isdir(local_freertos):
        FRAMEWORK_DIR = local_freertos
    else:
        raise Exception("FreeRTOS not found. Please install it as a PlatformIO package or place it in lib/FreeRTOS")

if FRAMEWORK_DIR is None or not isdir(FRAMEWORK_DIR):
    raise Exception("FreeRTOS framework directory not found: %s" % FRAMEWORK_DIR)

# FreeRTOS directory structure - check if it's the kernel repo structure
if isdir(join(FRAMEWORK_DIR, "FreeRTOS", "Source")):
    # Full FreeRTOS distribution structure
    FREERTOS_SRC_DIR = join(FRAMEWORK_DIR, "FreeRTOS", "Source")
elif isdir(join(FRAMEWORK_DIR, "Source")):
    # Kernel-only repo structure
    FREERTOS_SRC_DIR = join(FRAMEWORK_DIR, "Source")
else:
    # Assume flat structure
    FREERTOS_SRC_DIR = FRAMEWORK_DIR

# Try NXP-specific PowerPC_Z4 port first (for MPC57xx boards), then fall back to generic PowerPC
FREERTOS_PORT_DIR = None
POWERPC_PORT_NAME = None

# Check for NXP-specific PowerPC_Z4 port (recommended for MPC57xx)
powerpc_z4_port = join(FREERTOS_SRC_DIR, "portable", "GCC", "PowerPC_Z4")
if isdir(powerpc_z4_port) and exists(join(powerpc_z4_port, "port.c")):
    FREERTOS_PORT_DIR = powerpc_z4_port
    POWERPC_PORT_NAME = "PowerPC_Z4"
    print("Found NXP PowerPC_Z4 port (MPC57xx-optimized)")
else:
    # Fall back to generic PowerPC port (mainline FreeRTOS)
    powerpc_port = join(FREERTOS_SRC_DIR, "portable", "GCC", "PowerPC")
    if isdir(powerpc_port) and exists(join(powerpc_port, "port.c")):
        FREERTOS_PORT_DIR = powerpc_port
        POWERPC_PORT_NAME = "PowerPC"
        print("Found generic PowerPC port (mainline FreeRTOS)")
        print("Warning: Generic PowerPC port may not support MPC57xx VLE instructions")
    else:
        raise Exception(
            "No PowerPC FreeRTOS port found. Expected either:\n"
            "  - portable/GCC/PowerPC_Z4/ (NXP MPC57xx optimized)\n"
            "  - portable/GCC/PowerPC/ (generic/mainline)\n\n"
            "For MPC57xx boards, use NXP FreeRTOS release from:\n"
            "  https://github.com/dapperfu/platform-nxppowerpc/releases/download/v.0.0.1/freertos-9.0.0_MPC57XXX_public_rel_1.zip"
        )

# Add FreeRTOS include paths
env.Append(
    CPPPATH=[
        join(FREERTOS_SRC_DIR, "include"),
        FREERTOS_PORT_DIR,
        join("$PROJECT_DIR", "include"),
        join("$PROJECT_DIR", "src"),
    ],
    # Prevent PlatformIO library finder from processing FreeRTOS
    PIO_LIB_SRC_FILTER=[
        "-<lib/FreeRTOS/>",  # Exclude FreeRTOS from library finder
    ]
)

# Add FreeRTOS source files
freertos_sources = [
    join(FREERTOS_SRC_DIR, "croutine.c"),
    join(FREERTOS_SRC_DIR, "event_groups.c"),
    join(FREERTOS_SRC_DIR, "list.c"),
    join(FREERTOS_SRC_DIR, "queue.c"),
    join(FREERTOS_SRC_DIR, "stream_buffer.c"),
    join(FREERTOS_SRC_DIR, "tasks.c"),
    join(FREERTOS_SRC_DIR, "timers.c"),
]

# Add port-specific sources
port_sources = [
    join(FREERTOS_PORT_DIR, "port.c"),
]

# Check for assembly file (NXP port uses portasm.s)
portasm_file = join(FREERTOS_PORT_DIR, "portasm.s")
if exists(portasm_file):
    port_sources.append(portasm_file)
    print("Found portasm.s (NXP VLE assembly port)")

# Add FreeRTOS source files directly to the build - compile specific files only
# Filter out sources that don't exist
valid_freertos_sources = [env.subst(s) for s in freertos_sources if exists(s)]
valid_port_sources = [env.subst(s) for s in port_sources if exists(s)]

# Create source filter to exclude all portable ports except the selected one
src_filter = [
    "-<portable/>",  # Exclude all portable directories
    "+<portable/GCC/%s/>" % POWERPC_PORT_NAME,  # Include only selected PowerPC port
]

# Add FreeRTOS sources with filter
env.BuildSources(
    join("$BUILD_DIR", "FrameworkFreeRTOS"),
    FREERTOS_SRC_DIR,
    src_filter=src_filter
)

# Add FreeRTOS defines
env.Append(
    CPPDEFINES=[
        "FREERTOS"
    ]
)

print("FreeRTOS framework initialized for NXP PowerPC VLE")
print("  - FreeRTOS Source: %s" % FREERTOS_SRC_DIR)
print("  - Port Directory: %s" % FREERTOS_PORT_DIR)
print("  - Port Name: %s" % POWERPC_PORT_NAME)


