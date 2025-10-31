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
NXP PowerPC VLE PlatformIO Builder

Builds firmware for NXP PowerPC VLE microcontrollers using PlatformIO's
toolchain package system for cross-compilation.
"""

from os.path import join, exists
import os

from SCons.Script import (COMMAND_LINE_TARGETS, AlwaysBuild, Builder, Default,
                          DefaultEnvironment)

env = DefaultEnvironment()
platform = env.PioPlatform()

# Try to get toolchain package directory, fallback to system toolchain
TOOLCHAIN_DIR = None
TOOLCHAIN_PREFIX = "powerpc-eabivle-"
SYSTEM_TOOLCHAIN_PATHS = [
    "/tmp/toolchain/S32DS/build_tools/powerpc-eabivle-4_9/bin",
    "/S32DS/build_tools/powerpc-eabivle-4_9/powerpc-eabivle/bin",
    "/opt/powerpc-eabivle/bin",
    "/usr/local/powerpc-eabivle/bin",
]

try:
    TOOLCHAIN_DIR = platform.get_package_dir("toolchain-powerpc-eabivle")
except Exception:
    # Package not found, try system toolchain
    for sys_path in SYSTEM_TOOLCHAIN_PATHS:
        if exists(join(sys_path, TOOLCHAIN_PREFIX + "gcc")):
            TOOLCHAIN_DIR = sys_path
            # Use full paths for system toolchain
            TOOLCHAIN_PREFIX = join(sys_path, TOOLCHAIN_PREFIX)
            break
    
    if TOOLCHAIN_DIR is None:
        # Still not found, try PATH
        import shutil
        if shutil.which(TOOLCHAIN_PREFIX + "gcc"):
            TOOLCHAIN_PREFIX = TOOLCHAIN_PREFIX
            print("Using system toolchain from PATH")
        else:
            raise Exception("PowerPC EABI VLE toolchain not found. Please install it or set up the PlatformIO package.")

# Get board configuration
board = env.BoardConfig()

# PowerPC VLE machine flags
machine_flags = [
    "-meabi",
    "-mhard-float",
    "-mspe",
    f"-mcpu={board.get('build.cpu', 'e200z4')}"
]

# Configure toolchain
# PlatformIO will find tools in the toolchain package's bin directory
env.Replace(
    # Tool names - PlatformIO will locate them in the toolchain package
    AR=TOOLCHAIN_PREFIX + "ar",
    AS=TOOLCHAIN_PREFIX + "as",
    CC=TOOLCHAIN_PREFIX + "gcc",
    CXX=TOOLCHAIN_PREFIX + "g++",
    OBJCOPY=TOOLCHAIN_PREFIX + "objcopy",
    OBJDUMP=TOOLCHAIN_PREFIX + "objdump",
    RANLIB=TOOLCHAIN_PREFIX + "ranlib",
    SIZETOOL=TOOLCHAIN_PREFIX + "size",
    LINK="$CC",
    
    ARFLAGS=["rc"],
    
    PIODEBUGFLAGS=["-O0", "-g3", "-ggdb", "-gdwarf-2"],
    
    SIZEPROGREGEXP=r"^(?:\.text|\.data|\.rodata|\.vectors)\s+([0-9]+).*",
    SIZEDATAREGEXP=r"^(?:\.data|\.bss|\.noinit)\s+(\d+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD='$SIZETOOL -B -d $SOURCES',
    
    PROGSUFFIX=".elf"
)

# Configure build flags
env.Append(
    ASFLAGS=machine_flags,
    ASPPFLAGS=[
        "-x", "assembler-with-cpp"
    ],
    
    CCFLAGS=machine_flags + [
        "-Os",
        "-ffunction-sections",
        "-fdata-sections",
        "-Wall",
        "-Wextra"
    ],
    
    CXXFLAGS=[
        "-fno-exceptions",
        "-fno-rtti",
        "-fno-threadsafe-statics"
    ],
    
    CPPDEFINES=[
        ("F_CPU", board.get("build.f_cpu", "120000000L"))
    ],
    
    LINKFLAGS=machine_flags + [
        "-Os",
        "-Wl,-gc-sections",
    ],
)

# Find toolchain library path and add to LIBPATH
if TOOLCHAIN_DIR:
    toolchain_lib_base = join(TOOLCHAIN_DIR, "..", "..", "e200_ewl2", "lib")
    cpu_variant = board.get('build.cpu', 'e200z4')
    # Try to find library path for this CPU variant
    potential_lib_paths = [
        join(toolchain_lib_base, cpu_variant),
        join(toolchain_lib_base, cpu_variant, "spe"),
        join(toolchain_lib_base, "e200z6"),  # Fallback
    ]
    
    for lib_path in potential_lib_paths:
        expanded_lib_path = env.subst(lib_path)
        if exists(expanded_lib_path):
            env.Append(LIBPATH=[lib_path])
            env.Append(LIBS=["m", "c"])
            break

# Allow user to override via pre:script
if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")

# Create builders for binary output formats

env.Append(
    BUILDERS=dict(
        ElfToBin=Builder(
            action=env.VerboseAction(" ".join([
                "$OBJCOPY",
                "-O", "binary",
                "$SOURCES",
                "$TARGET"
            ]), "Building binary $TARGET"),
            suffix=".bin"
        ),
        ElfToHex=Builder(
            action=env.VerboseAction(" ".join([
                "$OBJCOPY",
                "-O", "ihex",
                "$SOURCES",
                "$TARGET"
            ]), "Building hex $TARGET"),
            suffix=".hex"
        )
    )
)

#
# Target: Build executable and linkable firmware
#

target_elf = None
if "nobuild" in COMMAND_LINE_TARGETS:
    target_elf = join("$BUILD_DIR", "${PROGNAME}.elf")
    target_bin = join("$BUILD_DIR", "${PROGNAME}.bin")
else:
    target_elf = env.BuildProgram()
    target_bin = env.ElfToBin(join("$BUILD_DIR", "${PROGNAME}"), target_elf)
    env.Depends(target_bin, "checkprogsize")

AlwaysBuild(env.Alias("nobuild", target_bin))
target_buildprog = env.Alias("buildprog", target_bin, target_bin)

#
# Target: Print binary size
#

target_size = env.Alias(
    "size", target_elf,
    env.VerboseAction("$SIZEPRINTCMD", "Calculating size $SOURCE")
)
AlwaysBuild(target_size)

#
# Default targets
#

Default([target_buildprog, target_size])
