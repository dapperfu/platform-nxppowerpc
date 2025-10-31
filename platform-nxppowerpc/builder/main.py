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

from os.path import join

from SCons.Script import (COMMAND_LINE_TARGETS, AlwaysBuild, Builder, Default,
                          DefaultEnvironment)

env = DefaultEnvironment()
platform = env.PioPlatform()

# Get toolchain package directory
TOOLCHAIN_DIR = platform.get_package_dir("toolchain-powerpc-eabivle")
TOOLCHAIN_PREFIX = "powerpc-eabivle-"

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
        "-nostdlib"
    ],
    
    LIBS=["m", "c"],
)

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
