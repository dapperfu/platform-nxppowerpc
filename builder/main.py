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

Toolchain Source (Golden Source):
https://github.com/dapperfu/platform-nxppowerpc/releases/download/v.0.0.1/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip
"""

from os.path import join, exists
import os

from SCons.Script import (COMMAND_LINE_TARGETS, AlwaysBuild, Builder, Default,
                          DefaultEnvironment)

env = DefaultEnvironment()
platform = env.PioPlatform()

# Toolchain golden source URL (for reference and manual download)
TOOLCHAIN_URL = "https://github.com/dapperfu/platform-nxppowerpc/releases/download/v.0.0.1/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip"

# Try to get toolchain package directory, fallback to system toolchain
TOOLCHAIN_DIR = None
TOOLCHAIN_PREFIX = "powerpc-eabivle-"

# System toolchain paths - check common installation locations
# Users can override via environment variable POWERPC_TOOLCHAIN_PATH
import os
SYSTEM_TOOLCHAIN_PATHS = []

# Check environment variable first
if os.environ.get("POWERPC_TOOLCHAIN_PATH"):
    custom_path = os.path.join(os.environ["POWERPC_TOOLCHAIN_PATH"], "bin")
    SYSTEM_TOOLCHAIN_PATHS.append(custom_path)

# Add standard system paths (relative to common install locations)
# These are standard locations where toolchains are typically installed
standard_paths = [
    # S32DS installation locations (relative to common install paths)
    os.path.join(os.path.expanduser("~"), "S32DS", "build_tools", "powerpc-eabivle-4_9", "powerpc-eabivle", "bin"),
    os.path.join("S32DS", "build_tools", "powerpc-eabivle-4_9", "powerpc-eabivle", "bin"),
    # Standard Unix installation paths
    os.path.join(os.path.expanduser("~"), "powerpc-eabivle", "bin"),
    "/opt/powerpc-eabivle/bin",  # Standard /opt location
    "/usr/local/powerpc-eabivle/bin",  # Standard /usr/local location
]

SYSTEM_TOOLCHAIN_PATHS.extend(standard_paths)

# Try to get toolchain from PlatformIO package first
# PlatformIO automatically downloads packages from tools/<package-name>/package.json
# when referenced in platform.json, using the URLs specified in package.json.
# The package definition in tools/toolchain-powerpc-eabivle/package.json contains
# the GitHub release URL (https://github.com/dapperfu/platform-nxppowerpc/releases/download/...).
#
# If the package isn't installed yet, we try to install it explicitly using the PackageManager.
# This ensures PlatformIO downloads it from GitHub releases even if dependency resolution
# didn't run or failed.
TOOLCHAIN_DIR = None
try:
    # PlatformIO should automatically install packages listed in platform.json
    # when the platform is used. Check if the toolchain package is already installed.
    TOOLCHAIN_DIR = platform.get_package_dir("toolchain-powerpc-eabivle")
    # Verify toolchain directory exists and has the compiler
    # Check both root/bin and nested subdirectories (e.g., powerpc-eabivle-4_9/bin)
    if TOOLCHAIN_DIR:
        # Check root level first
        gcc_path_root = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX + "gcc")
        if exists(gcc_path_root):
            TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
            print("Using PlatformIO toolchain package (from GitHub): %s" % TOOLCHAIN_DIR)
        else:
            # Check nested subdirectories
            found_toolchain = False
            if exists(TOOLCHAIN_DIR):
                for item in os.listdir(TOOLCHAIN_DIR):
                    subdir = join(TOOLCHAIN_DIR, item)
                    if os.path.isdir(subdir):
                        gcc_path_sub = join(subdir, "bin", TOOLCHAIN_PREFIX + "gcc")
                        if exists(gcc_path_sub):
                            TOOLCHAIN_DIR = subdir
                            TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
                            print("Using PlatformIO toolchain package (from GitHub): %s" % TOOLCHAIN_DIR)
                            found_toolchain = True
                            break
            if not found_toolchain:
                TOOLCHAIN_DIR = None
    else:
        TOOLCHAIN_DIR = None
except Exception:
    TOOLCHAIN_DIR = None

# Fallback: If package not found (shouldn't happen if platform.json is properly configured),
# manually download and install from GitHub releases. This handles edge cases like:
# - Platform installed before toolchain was added to platform.json
# - PlatformIO dependency resolution issues
# - Git-installed platforms that haven't been properly initialized
if TOOLCHAIN_DIR is None:
    try:
        tools_package_json = join(
            platform.get_dir(),
            "tools",
            "toolchain-powerpc-eabivle",
            "package.json"
        )
        if exists(tools_package_json):
            # Read package.json to get the GitHub release URL
            import json
            with open(tools_package_json, 'r') as f:
                pkg_manifest = json.load(f)
            
            # Get the download URL for this system
            import platform as py_platform
            system = "linux_x86_64" if py_platform.machine() == "x86_64" else "linux_x86"
            if system in pkg_manifest.get("urls", {}):
                package_url = pkg_manifest["urls"][system]
                pkg_name = pkg_manifest["name"]
                pkg_version = pkg_manifest["version"]
                
                # Get PlatformIO packages directory
                # PlatformIO stores packages in .platformio/packages/ relative to project
                # First check if package is already installed
                try:
                    existing_pkg_dir = platform.get_package_dir(pkg_name)
                    if existing_pkg_dir:
                        # Check root level first
                        gcc_path_root = join(existing_pkg_dir, "bin", TOOLCHAIN_PREFIX + "gcc")
                        if exists(gcc_path_root):
                            TOOLCHAIN_DIR = existing_pkg_dir
                            TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
                            print("Using PlatformIO toolchain package (from GitHub): %s" % TOOLCHAIN_DIR)
                            existing_pkg_dir = None  # Mark as found
                        else:
                            # Check nested subdirectories
                            found_toolchain = False
                            if exists(existing_pkg_dir):
                                for item in os.listdir(existing_pkg_dir):
                                    subdir = join(existing_pkg_dir, item)
                                    if os.path.isdir(subdir):
                                        gcc_path_sub = join(subdir, "bin", TOOLCHAIN_PREFIX + "gcc")
                                        if exists(gcc_path_sub):
                                            TOOLCHAIN_DIR = subdir
                                            TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
                                            print("Using PlatformIO toolchain package (from GitHub): %s" % TOOLCHAIN_DIR)
                                            found_toolchain = True
                                            break
                            if found_toolchain:
                                existing_pkg_dir = None  # Mark as found
                            else:
                                existing_pkg_dir = None  # Not found, will download
                    else:
                        existing_pkg_dir = None
                except Exception:
                    existing_pkg_dir = None
                
                # Calculate packages directory - PlatformIO stores in .platformio/packages/
                # Do this outside the if block so pkg_install_dir is always defined
                platform_dir = platform.get_dir()
                # Try multiple possible locations
                possible_packages_dirs = [
                    join(platform_dir, "..", "packages"),  # .platformio/packages/
                    join(platform_dir, "..", "..", "packages"),  # Alternative
                ]
                
                # Also try getting from environment or platform config
                import os as os_module
                pio_home = os_module.environ.get("PLATFORMIO_HOME_DIR") or os_module.environ.get("HOME")
                if pio_home:
                    possible_packages_dirs.insert(0, join(pio_home, ".platformio", "packages"))
                
                packages_dir = None
                for pd in possible_packages_dirs:
                    if exists(pd):
                        packages_dir = pd
                        break
                
                if not packages_dir:
                    # Default fallback - use platform directory structure
                    packages_dir = join(platform_dir, "..", "packages")
                    os.makedirs(packages_dir, exist_ok=True)
                
                pkg_install_dir = join(packages_dir, pkg_name)
                
                # If not already found, download and install
                if not TOOLCHAIN_DIR:
                    # Check if already installed
                    # First, try to find the compiler in case it's already there
                    found_compiler = False
                    if exists(pkg_install_dir):
                        # Check root level
                        if exists(join(pkg_install_dir, "bin", TOOLCHAIN_PREFIX + "gcc")):
                            found_compiler = True
                        else:
                            # Check subdirectories
                            for item in os.listdir(pkg_install_dir):
                                subdir = join(pkg_install_dir, item)
                                if os.path.isdir(subdir) and exists(join(subdir, "bin", TOOLCHAIN_PREFIX + "gcc")):
                                    found_compiler = True
                                    break
                    
                    if not found_compiler:
                        print("Downloading toolchain from GitHub: %s" % package_url)
                        
                        # Download and extract the toolchain
                        import urllib.request
                        import zipfile
                        import tempfile
                        
                        with tempfile.NamedTemporaryFile(delete=False, suffix=".zip") as tmp_zip:
                            zip_path = tmp_zip.name
                            urllib.request.urlretrieve(package_url, zip_path)
                            
                            # Extract to packages directory
                            os.makedirs(pkg_install_dir, exist_ok=True)
                            with zipfile.ZipFile(zip_path, 'r') as zip_ref:
                                zip_ref.extractall(pkg_install_dir)
                            
                            # Remove temp file
                            os.unlink(zip_path)
                        
                        print("Toolchain installed from GitHub: %s" % pkg_install_dir)
                    
                    # Find the actual toolchain root (may be in a subdirectory)
                    if exists(join(pkg_install_dir, "bin", TOOLCHAIN_PREFIX + "gcc")):
                        TOOLCHAIN_DIR = pkg_install_dir
                    else:
                        # Look for bin directory in subdirectories (e.g., powerpc-eabivle-4_9/bin)
                        if exists(pkg_install_dir):
                            for item in os.listdir(pkg_install_dir):
                                subdir = join(pkg_install_dir, item)
                                if os.path.isdir(subdir) and exists(join(subdir, "bin", TOOLCHAIN_PREFIX + "gcc")):
                                    TOOLCHAIN_DIR = subdir
                                    break
                
                # Final check - find toolchain if not already found
                if not TOOLCHAIN_DIR:
                    # Find the actual toolchain root (may be in a subdirectory)
                    # Check root level first
                    gcc_path_root = join(pkg_install_dir, "bin", TOOLCHAIN_PREFIX + "gcc")
                    if exists(gcc_path_root):
                        TOOLCHAIN_DIR = pkg_install_dir
                    else:
                        # Look for bin directory in subdirectories (e.g., powerpc-eabivle-4_9/bin)
                        if exists(pkg_install_dir):
                            for item in os.listdir(pkg_install_dir):
                                subdir = join(pkg_install_dir, item)
                                if os.path.isdir(subdir) and exists(join(subdir, "bin", TOOLCHAIN_PREFIX + "gcc")):
                                    TOOLCHAIN_DIR = subdir
                                    break
                
                # Verify toolchain was found and set prefix correctly
                if TOOLCHAIN_DIR:
                    gcc_path_final = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX + "gcc")
                    if exists(gcc_path_final):
                        TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
                        print("Using PlatformIO toolchain package (downloaded from GitHub): %s" % TOOLCHAIN_DIR)
                else:
                    # Toolchain downloaded but compiler not found - this shouldn't happen
                    print("Warning: Toolchain downloaded but compiler not found at expected path")
    except Exception as install_error:
        # Package installation failed, continue to system toolchain check
        import traceback
        print("Error during toolchain download/installation: %s" % str(install_error))
        traceback.print_exc()

# If PlatformIO package not found, try system toolchain
if TOOLCHAIN_DIR is None:
    for sys_path in SYSTEM_TOOLCHAIN_PATHS:
        gcc_path = join(sys_path, TOOLCHAIN_PREFIX + "gcc")
        if exists(gcc_path):
            TOOLCHAIN_DIR = sys_path
            # Use full paths for system toolchain
            TOOLCHAIN_PREFIX = join(sys_path, TOOLCHAIN_PREFIX)
            print("Using system toolchain: %s" % TOOLCHAIN_DIR)
            break
    
    if TOOLCHAIN_DIR is None:
        # Still not found, try PATH
        import shutil
        which_gcc = shutil.which(TOOLCHAIN_PREFIX + "gcc")
        if which_gcc:
            TOOLCHAIN_PREFIX = TOOLCHAIN_PREFIX
            print("Using system toolchain from PATH: %s" % which_gcc)
        else:
            raise Exception(
                "PowerPC EABI VLE toolchain not found.\n\n"
                "Toolchain Golden Source (v0.0.1):\n"
                "%s\n\n"
                "Manual Installation:\n"
                "1. Download the toolchain from the URL above\n"
                "2. Extract to a system location (e.g., ~/powerpc-eabivle/ or /opt/powerpc-eabivle/)\n"
                "3. Set POWERPC_TOOLCHAIN_PATH environment variable to the toolchain directory\n"
                "   OR ensure bin/powerpc-eabivle-gcc is in your PATH\n\n"
                "Or install as a PlatformIO package if available in the registry."
                % TOOLCHAIN_URL
            )

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
# Note: Assembly files (.S) will be preprocessed, (.s) will not
# For .S files, compile through GCC (not direct assembler) to handle @ha/@l relocations
env.Append(
    ASFLAGS=machine_flags + [
        "-Wa,-mvle",  # Enable VLE mode for assembler
        "-Wa,-mrelocatable",  # Enable relocatable code generation for @ha/@l relocations
    ],
    # Preprocessed assembly (.S files) - compile through GCC to handle PowerPC relocations
    # Note: Assembly files may need .vle directive or proper VLE section directives
    # The errors suggest assembler confusion with register indirect addressing
    ASPPFLAGS=machine_flags + [
        "-x", "assembler-with-cpp",
        "-Wa,-mvle",  # Enable VLE mode for assembler
        "-Wa,-memb",  # Enable embedded ABI mode (may help with VLE instructions)
        # Note: Some assembly files may need manual fixes for register syntax
        # The linker will handle relocations during final link
    ],
    # Override for .s files (non-preprocessed) - use direct assembler with VLE
    SFLAGS=machine_flags + [
        "-Wa,-mvle",
        "-Wa,-mrelocatable",
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
    cpu_variant = board.get('build.cpu', 'e200z4')
    # Try multiple possible library base paths
    # Libraries might be at package root or in subdirectory
    toolchain_package_root = TOOLCHAIN_DIR
    # If TOOLCHAIN_DIR is a subdirectory (e.g., powerpc-eabivle-4_9), go up to package root
    if os.path.basename(TOOLCHAIN_DIR).startswith("powerpc-eabivle"):
        toolchain_package_root = join(TOOLCHAIN_DIR, "..")
    
    # Try library paths at package root level
    toolchain_lib_base_1 = join(toolchain_package_root, "e200_ewl2", "lib")
    # Also try within the subdirectory if TOOLCHAIN_DIR is a subdirectory
    toolchain_lib_base_2 = join(TOOLCHAIN_DIR, "e200_ewl2", "lib") if TOOLCHAIN_DIR != toolchain_package_root else None
    
    # Try to find library path for this CPU variant
    potential_lib_paths = []
    for lib_base in [toolchain_lib_base_1, toolchain_lib_base_2]:
        if lib_base:
            potential_lib_paths.extend([
                join(lib_base, cpu_variant),
                join(lib_base, cpu_variant, "spe"),
                join(lib_base, "e200z6"),  # Fallback
            ])
    
    for lib_path in potential_lib_paths:
        expanded_lib_path = env.subst(lib_path)
        if exists(expanded_lib_path):
            env.Append(LIBPATH=[lib_path])
            env.Append(LIBS=["m", "c"])
            break

# Allow user to override via pre:script
if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")

# Auto-detect and configure linker script
# Priority:
# 1. User-specified linker script in build_flags (-T path/to/linker.ld)
# 2. Board-specific linker script from board.json (board.build.linker_script)
# 3. Project-level linker.ld in PROJECT_DIR
# 4. Board-specific default linker script from platform/linker/
# 5. No linker script (user must provide)

def find_linker_script():
    """Find appropriate linker script with fallback hierarchy."""
    board_mcu = board.get("build.mcu", "").lower()
    
    # Check if user already specified a linker script in build_flags
    # Look in both BUILD_FLAGS and LINKFLAGS
    for flag_list_name in ["BUILD_FLAGS", "LINKFLAGS"]:
        flags = env.get(flag_list_name, [])
        for flag in flags:
            flag_str = str(flag) if not isinstance(flag, str) else flag
            if "-T" in flag_str:
                # User has specified linker script
                return None
    
    # Check board configuration for linker script
    # Handle missing option gracefully (some boards don't have this field)
    try:
        board_linker = board.get("build.linker_script")
    except (KeyError, AttributeError):
        board_linker = None
    
    if board_linker:
        # Can be relative to platform or absolute
        if exists(env.subst(board_linker)):
            return board_linker
        # Try relative to platform
        platform_linker = join(platform.get_dir(), "linker", board_linker)
        if exists(env.subst(platform_linker)):
            return platform_linker
    
    # Check for project-level linker.ld
    project_linker = join("$PROJECT_DIR", "linker.ld")
    if exists(env.subst(project_linker)):
        return project_linker
    
    # Check for board-specific linker script in platform
    platform_dir = platform.get_dir()
    
    # Get linker type from board config (default: flash)
    linker_type = board.get("build.linker_type", "flash")  # flash or ram
    
    # Try board-specific variants first
    linker_variants = [
        f"{board_mcu}_{linker_type}.ld",  # e.g., mpc5748g_flash.ld
        f"{board_mcu}.ld",                # e.g., mpc5748g.ld
    ]
    
    # Add series-based fallbacks
    if "574" in board_mcu:
        linker_variants.extend([
            f"57xx_{linker_type}.ld",     # e.g., 57xx_flash.ld
            "57xx_flash.ld",              # Always try flash as fallback
        ])
    elif "564" in board_mcu:
        linker_variants.extend([
            f"56xx_{linker_type}.ld",     # e.g., 56xx_flash.ld
            "56xx_flash.ld",              # Always try flash as fallback
        ])
    elif "577" in board_mcu:
        linker_variants.extend([
            f"57xx_{linker_type}.ld",
            "57xx_flash.ld",
        ])
    
    for variant in linker_variants:
        platform_linker = join(platform_dir, "linker", variant)
        if exists(env.subst(platform_linker)):
            return platform_linker
    
    return None

# Auto-configure linker script if found
linker_script = find_linker_script()
if linker_script:
    # Use proper SCons substitution and avoid extra spaces in path
    linker_flag = f"-T{env.subst(linker_script)}"
    env.Append(LINKFLAGS=[linker_flag])
    print(f"Using linker script: {env.subst(linker_script)}")

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
