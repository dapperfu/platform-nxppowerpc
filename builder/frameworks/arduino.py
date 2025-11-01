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
Arduino Framework for NXP PowerPC VLE

Arduino API compatibility layer for PowerPC VLE microcontrollers.
Built on top of FreeRTOS for multi-tasking support.
"""

from os.path import isdir, join, exists

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

# First, set up FreeRTOS (Arduino runs on top of FreeRTOS)
FRAMEWORK_DIR = None
try:
    FRAMEWORK_DIR = platform.get_package_dir("framework-freertos")
    if not isdir(FRAMEWORK_DIR):
        FRAMEWORK_DIR = None
except Exception:
    pass

if FRAMEWORK_DIR is None:
    import os
    project_dir = env.subst("$PROJECT_DIR")
    local_freertos = os.path.join(project_dir, "lib", "FreeRTOS")
    if isdir(local_freertos):
        FRAMEWORK_DIR = local_freertos
    else:
        raise Exception("FreeRTOS not found. Please install it as a PlatformIO package or place it in lib/FreeRTOS")

if FRAMEWORK_DIR is None or not isdir(FRAMEWORK_DIR):
    raise Exception("FreeRTOS framework directory not found: %s" % FRAMEWORK_DIR)

# FreeRTOS directory structure
if isdir(join(FRAMEWORK_DIR, "FreeRTOS", "Source")):
    FREERTOS_SRC_DIR = join(FRAMEWORK_DIR, "FreeRTOS", "Source")
elif isdir(join(FRAMEWORK_DIR, "Source")):
    FREERTOS_SRC_DIR = join(FRAMEWORK_DIR, "Source")
else:
    FREERTOS_SRC_DIR = FRAMEWORK_DIR

FREERTOS_PORT_DIR = join(FREERTOS_SRC_DIR, "portable", "GCC", "PowerPC")

# Get Arduino framework directory (within platform)
ARDUINO_FRAMEWORK_DIR = join(platform.get_dir(), "frameworks", "arduino")
ARDUINO_CORE_DIR = join(ARDUINO_FRAMEWORK_DIR, "cores", "powerpc")
ARDUINO_LIBRARIES_DIR = join(ARDUINO_FRAMEWORK_DIR, "libraries")

# Add include paths
include_paths = [
    join(FREERTOS_SRC_DIR, "include"),
    FREERTOS_PORT_DIR,
    ARDUINO_CORE_DIR,
    ARDUINO_LIBRARIES_DIR,
    join("$PROJECT_DIR", "include"),
    join("$PROJECT_DIR", "src"),
]

# Add library include paths
if isdir(ARDUINO_LIBRARIES_DIR):
    import os
    for item in os.listdir(ARDUINO_LIBRARIES_DIR):
        lib_path = join(ARDUINO_LIBRARIES_DIR, item)
        if isdir(lib_path):
            include_paths.append(lib_path)

env.Append(
    CPPPATH=include_paths,
    # Prevent PlatformIO library finder from processing FreeRTOS and Arduino
    PIO_LIB_SRC_FILTER=[
        "-<lib/FreeRTOS/>",
        "-<frameworks/arduino/>",
    ]
)

# Add FreeRTOS source files
src_filter = [
    "-<portable/>",
    "+<portable/GCC/PowerPC/>",
]

env.BuildSources(
    join("$BUILD_DIR", "FrameworkFreeRTOS"),
    FREERTOS_SRC_DIR,
    src_filter=src_filter
)

# Add Arduino core source files
if isdir(ARDUINO_CORE_DIR):
    env.BuildSources(
        join("$BUILD_DIR", "FrameworkArduino"),
        ARDUINO_CORE_DIR
    )

# Add Arduino library source files
if isdir(ARDUINO_LIBRARIES_DIR):
    import os
    for item in os.listdir(ARDUINO_LIBRARIES_DIR):
        lib_path = join(ARDUINO_LIBRARIES_DIR, item)
        if isdir(lib_path):
            # Look for .cpp files in the library directory
            env.BuildSources(
                join("$BUILD_DIR", "FrameworkArduino", "libraries", item),
                lib_path
            )

# Add framework defines
env.Append(
    CPPDEFINES=[
        "FREERTOS",
        "ARDUINO",
        "ARDUINO_DEVKIT_MPC5744P",
        ("ARDUINO_ARCH_POWERPC", "1"),
        ("F_CPU", board.get("build.f_cpu", "160000000L")),
    ]
)

print("Arduino framework initialized for NXP PowerPC VLE")
print("  - FreeRTOS Source: %s" % FREERTOS_SRC_DIR)
print("  - Arduino Core: %s" % ARDUINO_CORE_DIR)
print("  - Arduino Libraries: %s" % ARDUINO_LIBRARIES_DIR)

