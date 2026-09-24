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
    os.path.join(os.path.expanduser("~"), "powerpc-eabivle", "bin"),
    "/opt/powerpc-eabivle/bin",
    "/usr/local/powerpc-eabivle/bin",
]

SYSTEM_TOOLCHAIN_PATHS.extend(standard_paths)

# Get toolchain from PlatformIO package system
# PlatformIO automatically installs packages listed in platform.json from tools/<package-name>/package.json
# This works seamlessly like official toolchains (e.g., toolchain-armeabigcc)
TOOLCHAIN_DIR = None

def find_toolchain_in_dir(pkg_dir):
    """Find toolchain compiler in package directory, handling nested structures."""
    if not pkg_dir or not exists(pkg_dir):
        return None
    
    # Check root level first
    gcc_path = join(pkg_dir, "bin", TOOLCHAIN_PREFIX + "gcc")
    if exists(gcc_path):
        return pkg_dir
    
    # Check nested subdirectories (e.g., powerpc-eabivle-4_9/bin)
    try:
        for item in os.listdir(pkg_dir):
            subdir = join(pkg_dir, item)
            if os.path.isdir(subdir):
                subdir_gcc = join(subdir, "bin", TOOLCHAIN_PREFIX + "gcc")
                if exists(subdir_gcc):
                    return subdir
    except OSError:
        pass
    
    return None

try:
    TOOLCHAIN_DIR = platform.get_package_dir("toolchain-powerpc-eabivle")
    if TOOLCHAIN_DIR:
        # Verify toolchain is actually present
        actual_dir = find_toolchain_in_dir(TOOLCHAIN_DIR)
        if actual_dir:
            TOOLCHAIN_DIR = actual_dir
            TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
        else:
            TOOLCHAIN_DIR = None
except Exception:
    TOOLCHAIN_DIR = None

# Fallback: Auto-install from tools/package.json if PlatformIO dependency resolution hasn't run yet
# This ensures seamless installation like official toolchains, even for git-installed platforms
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
                
                # Check if package is already installed (may have been installed by PlatformIO after our check)
                try:
                    existing_pkg_dir = platform.get_package_dir(pkg_name)
                    if existing_pkg_dir:
                        found_dir = find_toolchain_in_dir(existing_pkg_dir)
                        if found_dir:
                            TOOLCHAIN_DIR = found_dir
                            TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
                except Exception:
                    pass
                
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
                    # Check if already installed in packages directory
                    if not find_toolchain_in_dir(pkg_install_dir):
                        print("Downloading toolchain from GitHub releases...")
                        print("  URL: %s" % package_url)
                        print("  Installing to: %s" % pkg_install_dir)
                        
                        # Download and extract the toolchain
                        import urllib.request
                        import zipfile
                        import tempfile
                        
                        try:
                            with tempfile.NamedTemporaryFile(delete=False, suffix=".zip") as tmp_zip:
                                zip_path = tmp_zip.name
                            
                            print("  Downloading archive...")
                            urllib.request.urlretrieve(package_url, zip_path)
                            
                            print("  Extracting archive...")
                            # Extract to packages directory
                            os.makedirs(pkg_install_dir, exist_ok=True)
                            with zipfile.ZipFile(zip_path, 'r') as zip_ref:
                                zip_ref.extractall(pkg_install_dir)
                            
                            # Remove temp file
                            os.unlink(zip_path)
                            
                            print("Toolchain installed successfully.")
                        except Exception as download_error:
                            print("ERROR: Failed to download/install toolchain: %s" % str(download_error))
                            import traceback
                            traceback.print_exc()
                            raise
                    
                    # Find toolchain after installation
                    found_dir = find_toolchain_in_dir(pkg_install_dir)
                    if found_dir:
                        TOOLCHAIN_DIR = found_dir
                        TOOLCHAIN_PREFIX = join(TOOLCHAIN_DIR, "bin", TOOLCHAIN_PREFIX)
                        print("Using PlatformIO toolchain package: %s" % TOOLCHAIN_DIR)
    except Exception as install_error:
        # Package installation failed, continue to system toolchain check
        import traceback
        print("Error during toolchain download/installation: %s" % str(install_error))
        print("Attempting automatic toolchain installation from platform tools directory...")
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
cpu = board.get("build.cpu", "e200z4")
linker_type = board.get("build.linker_type", "flash")
SPECS = os.environ.get("SPECS", "ewl_c9x_noio.specs")


def _is_ewl_dir(path):
    if not path:
        return False
    return exists(join(path, "EWL_C", "include")) or exists(join(path, "lib"))


def toolchain_package_root():
    """Root of the downloaded toolchain tarball/zip after PlatformIO extracts it."""
    try:
        pkg = platform.get_package_dir("toolchain-powerpc-eabivle")
        if pkg:
            return pkg
    except Exception:
        pass
    if TOOLCHAIN_DIR:
        name = os.path.basename(TOOLCHAIN_DIR)
        if name.startswith("powerpc-eabivle"):
            return os.path.dirname(TOOLCHAIN_DIR)
        return TOOLCHAIN_DIR
    return None


def find_ewl_dir():
    """EWL lives inside the toolchain package (e200_ewl2 next to powerpc-eabivle-4_9).

    Override with board_build.ewl_dir in platformio.ini if needed. Never use a
    host S32DS install; an expanded tarball on disk is reference only.
    """
    candidates = []
    try:
        raw = board.get("build.ewl_dir")
    except (KeyError, AttributeError):
        raw = None
    if raw:
        candidates.append(os.path.realpath(env.subst(str(raw))))

    pkg_root = toolchain_package_root()
    if pkg_root:
        candidates.append(os.path.realpath(join(pkg_root, "e200_ewl2")))
        candidates.append(os.path.realpath(join(pkg_root, "powerpc-eabivle-4_9", "e200_ewl2")))
    if TOOLCHAIN_DIR:
        candidates.append(os.path.realpath(join(TOOLCHAIN_DIR, "e200_ewl2")))

    seen = set()
    unique = []
    for candidate in candidates:
        if not candidate or candidate in seen:
            continue
        seen.add(candidate)
        unique.append(candidate)
        if _is_ewl_dir(candidate):
            return candidate
    candidates = unique

    raise Exception(
        "e200_ewl2 was not found inside the toolchain package. "
        "It must ship in the downloaded tarball as e200_ewl2/ next to "
        "powerpc-eabivle-4_9/. Override with board_build.ewl_dir only if "
        "you extracted that same package tree elsewhere. Tried: %s" %
        (", ".join(candidates) if candidates else "(no toolchain package)")
    )


def find_assembler_bin_dir():
    if not TOOLCHAIN_DIR:
        return None
    nested = join(TOOLCHAIN_DIR, "powerpc-eabivle", "bin")
    if exists(nested):
        return nested
    sibling = join(TOOLCHAIN_DIR, "bin")
    if exists(sibling):
        return sibling
    return None


EWL_DIR = find_ewl_dir()

SPECS_PATH = SPECS
if not exists(SPECS_PATH):
    for spec_candidate in (join(EWL_DIR, "lib", SPECS), join(EWL_DIR, SPECS)):
        if exists(spec_candidate):
            SPECS_PATH = spec_candidate
            break
if not exists(SPECS_PATH):
    raise Exception("EWL specs file %s not found under %s" % (SPECS, EWL_DIR))
print("Using EWL sysroot: %s" % EWL_DIR)
print("Using EWL specs: %s" % SPECS_PATH)


def find_ewl_lib_dir():
    """Directory that holds EWL libc99.a / libm.a / librt.a for this CPU."""
    candidates = [
        join(EWL_DIR, "lib", cpu, "fp"),
        join(EWL_DIR, "lib", cpu),
        join(EWL_DIR, "lib"),
    ]
    for candidate in candidates:
        if exists(join(candidate, "libc99.a")):
            return candidate
    lib_root = join(EWL_DIR, "lib")
    if exists(lib_root):
        for root, _dirs, files in os.walk(lib_root):
            if "libc99.a" in files:
                return root
    return None


EWL_LIB_DIR = find_ewl_lib_dir()
if EWL_LIB_DIR is None:
    raise Exception(
        "EWL C archives (libc99.a, libm.a, librt.a) not found under %s. "
        "The toolchain tarball's e200_ewl2/lib tree must include them." % EWL_DIR
    )
print("Using EWL libraries: %s" % EWL_LIB_DIR)

ASSEMBLER_BIN_DIR = find_assembler_bin_dir()
B_DIRS = [ASSEMBLER_BIN_DIR] if ASSEMBLER_BIN_DIR else []
if ASSEMBLER_BIN_DIR:
    env.PrependENVPath("PATH", ASSEMBLER_BIN_DIR)
if TOOLCHAIN_DIR:
    env.PrependENVPath("PATH", join(TOOLCHAIN_DIR, "bin"))

# DEVKIT-Makefile MACH_OPTS
machine_flags = [
    "-mcpu=%s" % cpu,
    "-mbig",
    "-mvle",
    "-mregnames",
    "-mhard-float",
]

common_c_flags = machine_flags + [
    "-std=gnu99",
    "-fmessage-length=0",
    "-fsigned-char",
    "-ffunction-sections",
    "-fdata-sections",
    "-Wall",
    "-g3",
    "--sysroot=%s" % EWL_DIR,
]
for b_dir in B_DIRS:
    common_c_flags.append("-B%s" % b_dir)
specs_flag = "-specs=%s" % SPECS_PATH

cppdefines = [
    "MPC574xP",
    ("F_CPU", board.get("build.f_cpu", "160000000L")),
]
if linker_type != "ram":
    cppdefines.append("START_FROM_FLASH")

env.Replace(
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
    SIZEPROGREGEXP=r"^(?:\.text|\.data|\.rodata|\.vectors|\.startup|\.rchw)\s+([0-9]+).*",
    SIZEDATAREGEXP=r"^(?:\.data|\.bss|\.noinit|\.sdata|\.sbss)\s+(\d+).*",
    SIZECHECKCMD="$SIZETOOL -A -d $SOURCES",
    SIZEPRINTCMD="$SIZETOOL -B -d $SOURCES",
    PROGSUFFIX=".elf",
)

env.Append(
    ASFLAGS=common_c_flags,
    ASPPFLAGS=common_c_flags + ["-x", "assembler-with-cpp"],
    CCFLAGS=common_c_flags + [specs_flag],
    CXXFLAGS=["-fno-exceptions", "-fno-rtti", "-fno-threadsafe-statics"],
    CPPDEFINES=cppdefines,
    CPPPATH=[
        join(EWL_DIR, "EWL_C", "include"),
        join(EWL_DIR, "EWL_C", "include", "pa"),
    ],
    LINKFLAGS=machine_flags + [
        specs_flag,
        "--sysroot=%s" % EWL_DIR,
        "-fno-use-linker-plugin",
        "-Wl,--gc-sections",
        "-Wl,-Map,%s" % join("$BUILD_DIR", "${PROGNAME}.map"),
    ],
    LIBPATH=[EWL_LIB_DIR],
)
if TOOLCHAIN_DIR:
    gcc_lib = join(TOOLCHAIN_DIR, "lib", "gcc", "powerpc-eabivle", "4.9.4", cpu)
    if exists(join(gcc_lib, "libgcc.a")):
        env.Append(LIBPATH=[gcc_lib])
for b_dir in B_DIRS:
    env.Append(LINKFLAGS=["-B%s" % b_dir])

if env.get("PROGNAME", "program") == "program":
    env.Replace(PROGNAME="firmware")


def user_specified_linker_script():
    for flag_list_name in ("BUILD_FLAGS", "LINKFLAGS"):
        for flag in env.get(flag_list_name, []):
            if "-T" in str(flag):
                return True
    return False


def resolve_platform_linker(name):
    if not name:
        return None
    if exists(env.subst(name)):
        return env.subst(name)
    platform_linker = join(platform.get_dir(), "linker", name)
    if exists(platform_linker):
        return platform_linker
    return None


def find_memory_linker_script():
    try:
        board_linker = board.get("build.linker_script")
    except (KeyError, AttributeError):
        board_linker = None
    found = resolve_platform_linker(board_linker)
    if found:
        return found
    project_linker = env.subst(join("$PROJECT_DIR", "linker.ld"))
    if exists(project_linker):
        return project_linker
    board_mcu = board.get("build.mcu", "mpc5744p").lower()
    for variant in (
        "%s_%s.ld" % (board_mcu, linker_type),
        "57xx_%s.ld" % linker_type,
        "57xx_flash.ld",
    ):
        found = resolve_platform_linker(variant)
        if found:
            return found
    return None


if not user_specified_linker_script():
    libs_ld = resolve_platform_linker("libs.ld")
    memory_ld = find_memory_linker_script()
    if libs_ld:
        env.Append(LINKFLAGS=["-T%s" % libs_ld])
        print("Using linker script: %s" % libs_ld)
    if memory_ld:
        env.Append(LINKFLAGS=["-T%s" % memory_ld])
        print("Using linker script: %s" % memory_ld)

env.Append(
    BUILDERS=dict(
        ElfToBin=Builder(
            action=env.VerboseAction(" ".join([
                "$OBJCOPY", "--strip-all", "--output-target", "binary",
                "$SOURCES", "$TARGET"
            ]), "Building binary $TARGET"),
            suffix=".bin"
        ),
        ElfToHex=Builder(
            action=env.VerboseAction(" ".join([
                "$OBJCOPY", "--strip-all", "--output-target", "ihex",
                "$SOURCES", "$TARGET"
            ]), "Building hex $TARGET"),
            suffix=".hex"
        ),
        ElfToS19=Builder(
            action=env.VerboseAction(" ".join([
                "$OBJCOPY", "--strip-all", "--output-target", "srec",
                "$SOURCES", "$TARGET"
            ]), "Building s19 $TARGET"),
            suffix=".s19"
        )
    )
)

if "nobuild" in COMMAND_LINE_TARGETS:
    target_elf = join("$BUILD_DIR", "${PROGNAME}.elf")
    target_hex = join("$BUILD_DIR", "${PROGNAME}.hex")
    target_bin = join("$BUILD_DIR", "${PROGNAME}.bin")
    target_s19 = join("$BUILD_DIR", "${PROGNAME}.s19")
else:
    target_elf = env.BuildProgram()
    target_hex = env.ElfToHex(join("$BUILD_DIR", "${PROGNAME}"), target_elf)
    target_bin = env.ElfToBin(join("$BUILD_DIR", "${PROGNAME}"), target_elf)
    target_s19 = env.ElfToS19(join("$BUILD_DIR", "${PROGNAME}"), target_elf)
    env.Depends(target_hex, "checkprogsize")
    env.Depends(target_bin, "checkprogsize")
    env.Depends(target_s19, "checkprogsize")

AlwaysBuild(env.Alias("nobuild", [target_hex, target_bin, target_s19]))
target_buildprog = env.Alias(
    "buildprog",
    [target_elf, target_hex, target_bin, target_s19],
)

target_size = env.Alias(
    "size", target_elf,
    env.VerboseAction("$SIZEPRINTCMD", "Calculating size $SOURCE")
)
AlwaysBuild(target_size)

Default([target_buildprog, target_size])
