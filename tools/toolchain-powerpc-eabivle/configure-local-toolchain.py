#!/usr/bin/env python3
"""
Configure PlatformIO toolchain package to use a local zip file.

This script searches for a toolchain zip file in common locations and updates
package.json to use a file:// URL if found.
"""

from pathlib import Path
import json
import sys
from typing import Optional


def find_toolchain_zip() -> Optional[Path]:
    """Find toolchain zip file in common locations."""
    import os
    
    # Get current working directory and home directory
    cwd = Path.cwd()
    home = Path.home()
    
    # Get PROJECT_DIR from environment if available (PlatformIO sets this)
    project_dir = os.environ.get("PLATFORMIO_WORKSPACE_DIR") or os.environ.get("PIO_WORKSPACE_DIR")
    if project_dir:
        project_dir = Path(project_dir)
    else:
        project_dir = cwd
    
    search_paths = [
        # In the toolchain package directory
        Path(__file__).parent / "toolchain" / "gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip",
        Path(__file__).parent.parent.parent.parent / "toolchain" / "gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip",
        # In platform root
        Path(__file__).parent.parent.parent.parent / "gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip",
        # In current working directory or project directory
        cwd / "gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip",
        project_dir / "gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip",
        # In home directory
        home / "gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip",
        # Generic search by pattern in current directory and project directory
        *list(cwd.glob("**/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip")),
        *list(project_dir.glob("**/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip")),
        *list(cwd.glob("**/*powerpc*.zip")),
        *list(cwd.glob("**/*eabivle*.zip")),
        *list(project_dir.glob("**/*powerpc*.zip")),
        *list(project_dir.glob("**/*eabivle*.zip")),
    ]
    
    for path in search_paths:
        if path.exists() and path.is_file():
            return path.resolve()
    
    return None


def update_package_json(zip_path: Path) -> bool:
    """Update package.json to use local file:// URL."""
    package_json = Path(__file__).parent / "package.json"
    
    if not package_json.exists():
        print(f"Error: package.json not found at {package_json}", file=sys.stderr)
        return False
    
    # Read current package.json
    with open(package_json, "r") as f:
        data = json.load(f)
    
    # Update URL to use local file
    file_url = zip_path.as_uri()  # Creates file:// URL
    data["urls"]["linux_x86_64"] = file_url
    
    # Write back
    with open(package_json, "w") as f:
        json.dump(data, f, indent=2)
    
    print(f"Updated package.json to use local toolchain:")
    print(f"  {file_url}")
    return True


def main() -> int:
    """Main entry point."""
    print("Searching for local toolchain zip file...")
    
    zip_path = find_toolchain_zip()
    if not zip_path:
        print("No local toolchain zip file found.")
        print("\nExpected filename: gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip")
        print("\nSearched locations:")
        print("  - platform-nxppowerpc/tools/toolchain-powerpc-eabivle/toolchain/")
        print("  - platform-nxppowerpc/ (platform root)")
        print("  - Current working directory and subdirectories")
        print("  - Project directory (PLATFORMIO_WORKSPACE_DIR) and subdirectories")
        print("  - Home directory (~)")
        print("  - Any subdirectory matching *powerpc* or *eabivle*")
        return 1
    
    print(f"Found toolchain zip: {zip_path}")
    
    if not update_package_json(zip_path):
        return 1
    
    print("\nConfiguration complete!")
    print("\nNext steps:")
    print("  1. Reinstall the toolchain package:")
    print("     pio pkg uninstall toolchain-powerpc-eabivle")
    print("     pio pkg install toolchain-powerpc-eabivle")
    print("\n  2. Or rebuild your project:")
    print("     pio run")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())

