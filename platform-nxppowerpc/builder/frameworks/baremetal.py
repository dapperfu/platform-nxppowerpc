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
Supports user-provided startup code, linker scripts, and standard C library.
"""

from os.path import isdir, join

from SCons.Script import DefaultEnvironment

env = DefaultEnvironment()
platform = env.PioPlatform()
board = env.BoardConfig()

# No framework package - this is baremetal
# Users provide their own startup code and linker scripts

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

# Baremetal typically needs:
# - Startup code (provided by user in src/ or specified via build flags)
# - Linker script (provided by user or specified via build flags)
# - Standard C library (linked via -lc)

# Users can specify custom linker scripts via build_flags:
# build_flags = -T path/to/linker.ld

# Users can specify startup files via src/ directory
# PlatformIO will automatically compile .c, .cpp, .S files in src/

print("Baremetal framework initialized for NXP PowerPC VLE")
print("  - Provide your own startup code and linker script")
print("  - Use build_flags to specify linker script: -T path/to/script.ld")
print("  - Source files in src/ will be compiled automatically")

