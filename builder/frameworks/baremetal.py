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
Baremetal Framework for NXP PowerPC VLE

Minimal framework setup for baremetal development on PowerPC VLE microcontrollers.
Automatically includes startup code templates if user has not provided their own.
Supports user-provided startup code, linker scripts, and standard C library.
"""

from os.path import isdir, join, exists
import os

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

# No framework package - this is baremetal
# Startup code is automatically included if user hasn't provided their own

# Add common include paths if they exist
# Users can add custom include paths via build_flags in platformio.ini
potential_include_dirs = [
    join("$PROJECT_DIR", "include"),
    join("$PROJECT_DIR", "src"),
    join("$PROJECT_DIR", "lib"),
]

env.Append(
    CPPPATH=[
        d for d in potential_include_dirs if isdir(env.subst(d))
    ]
)

# Add library search paths
potential_lib_dirs = [
    join("$PROJECT_DIR", "lib"),
]

env.Append(
    LIBPATH=[
        d for d in potential_lib_dirs if isdir(env.subst(d))
    ]
)

# Auto-detect and include startup code if user hasn't provided their own
# Priority:
# 1. User-provided startup file in src/ (startup.S, startup.s, startup.c, or _start symbol)
# 2. Platform-provided startup template for the board
def find_startup_code():
    """Find appropriate startup code with fallback hierarchy."""
    project_src = env.subst("$PROJECT_DIR/src")
    
    # Check if user has provided startup code
    startup_patterns = ["startup.S", "startup.s", "startup.c"]
    if isdir(project_src):
        for pattern in startup_patterns:
            startup_file = join(project_src, pattern)
            if exists(startup_file):
                # User has provided startup code
                return None
    
    # Check for _start symbol in any .S file in src/
    if isdir(project_src):
        for root, dirs, files in os.walk(project_src):
            for file in files:
                if file.endswith((".S", ".s")):
                    file_path = join(root, file)
                    try:
                        with open(file_path, 'r') as f:
                            content = f.read()
                            if '_start' in content or '.globl\t_start' in content or '.globl _start' in content:
                                # User has provided startup code
                                return None
                    except (IOError, UnicodeDecodeError):
                        pass
    
    # No user startup code found - use platform template
    board_mcu = board.get("build.mcu", "").lower()
    platform_dir = platform.get_dir()
    startup_template = join(platform_dir, "startup", f"{board_mcu}_startup.S")
    
    if exists(startup_template):
        return startup_template
    
    return None

# Auto-include startup code if found
startup_code = find_startup_code()
if startup_code:
    # Build startup code from platform template
    env.BuildSources(
        join("$BUILD_DIR", "PlatformStartup"),
        join(platform.get_dir(), "startup"),
        src_filter=[f"+<{os.path.basename(startup_code)}>", "-<*>"]
    )
    print("Baremetal framework initialized for NXP PowerPC VLE")
    print(f"  - Using platform startup template: {os.path.basename(startup_code)}")
    print("  - Startup code automatically included")
    print("  - Linker script will be auto-detected from platform or project")
else:
    print("Baremetal framework initialized for NXP PowerPC VLE")
    print("  - Using user-provided startup code")
    print("  - Linker script will be auto-detected from platform or project")

# Users can specify custom linker scripts via build_flags:
# build_flags = -T path/to/linker.ld

# Users can specify startup files via src/ directory
# PlatformIO will automatically compile .c, .cpp, .S files in src/

