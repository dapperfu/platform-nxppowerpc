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

FREERTOS_PORT_DIR = join(FREERTOS_SRC_DIR, "portable", "GCC", "PowerPC")

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

# Add port-specific sources (check if PowerPC port exists, otherwise use generic)
port_sources = [
    join(FREERTOS_PORT_DIR, "port.c"),
]

# Check if port.c exists, if not, we may need to use a different port
# For now, assume PowerPC port exists
for source in port_sources:
    if not isdir(FREERTOS_PORT_DIR):
        # If PowerPC port doesn't exist, fall back to a generic approach
        # In practice, users may need to provide their own port
        print("Warning: PowerPC FreeRTOS port not found. You may need to provide your own port implementation.")
        break

# Add FreeRTOS source files directly to the build - compile specific files only
# Filter out sources that don't exist
valid_freertos_sources = [env.subst(s) for s in freertos_sources if exists(s)]
valid_port_sources = [env.subst(s) for s in port_sources if exists(s)]

# Create source filter to exclude all portable ports except PowerPC
src_filter = [
    "-<portable/>",  # Exclude all portable directories
    "+<portable/GCC/PowerPC/>",  # Include only PowerPC port
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


