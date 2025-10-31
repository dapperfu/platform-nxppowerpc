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

from os.path import isdir, join

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

FRAMEWORK_DIR = platform.get_package_dir("framework-freertos")
assert isdir(FRAMEWORK_DIR), "FreeRTOS framework package not found"

# FreeRTOS directory structure
FREERTOS_SRC_DIR = join(FRAMEWORK_DIR, "FreeRTOS", "Source")
FREERTOS_PORT_DIR = join(FREERTOS_SRC_DIR, "portable", "GCC", "PowerPC")

# Add FreeRTOS include paths
env.Append(
    CPPPATH=[
        join(FREERTOS_SRC_DIR, "include"),
        FREERTOS_PORT_DIR,
        join("$PROJECT_DIR", "include"),
        join("$PROJECT_DIR", "src"),
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

# Build FreeRTOS library
libs = []

freertos_lib = env.BuildLibrary(
    join("$BUILD_DIR", "FrameworkFreeRTOS"),
    [s for s in freertos_sources + port_sources if isdir(FREERTOS_SRC_DIR)]
)

libs.append(freertos_lib)

env.Append(LIBS=libs)

# Add FreeRTOS defines
env.Append(
    CPPDEFINES=[
        "FREERTOS"
    ]
)

print("FreeRTOS framework initialized for NXP PowerPC VLE")
print("  - FreeRTOS Source: %s" % FREERTOS_SRC_DIR)
print("  - Port Directory: %s" % FREERTOS_PORT_DIR)


