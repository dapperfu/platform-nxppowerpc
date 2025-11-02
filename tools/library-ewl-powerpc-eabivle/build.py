#!/usr/bin/env python3
"""
Build PlatformIO library package for EWL (Embedded Workbench Library) runtime libraries.

This script extracts EWL libraries from the S32DS installer and packages them
for PlatformIO distribution.
"""

import sys
import json
import hashlib
import zipfile
import shutil
from pathlib import Path
from typing import Optional

# Add parent tools directory to path for imports
TOOLS_DIR = Path(__file__).parent.parent
sys.path.insert(0, str(TOOLS_DIR))

from analyze_s32ds_installer import S32DSInstallerAnalyzer


PACKAGE_NAME = "library-ewl-powerpc-eabivle"
VERSION = "2.0.0"


def calculate_sha256(file_path: Path) -> str:
    """Calculate SHA256 hash of a file."""
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            sha256_hash.update(chunk)
    return sha256_hash.hexdigest()


def extract_and_package(installer_root: Path, output_dir: Path) -> Path:
    """Extract EWL libraries and create PlatformIO package."""
    print(f"Analyzing installer: {installer_root}")
    
    analyzer = S32DSInstallerAnalyzer(installer_root)
    ewl_info = analyzer.find_ewl_libraries()
    
    if not ewl_info:
        raise Exception(f"EWL libraries not found in installer: {installer_root}")
    
    print(f"Found EWL libraries: {ewl_info.path}")
    print(f"  Files: {ewl_info.files_count:,}")
    print(f"  Size: {ewl_info.total_size / (1024*1024):.2f} MB")
    
    # Create output directory
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Create package directory structure
    package_dir = output_dir / PACKAGE_NAME
    if package_dir.exists():
        print(f"Removing existing package directory: {package_dir}")
        shutil.rmtree(package_dir)
    
    package_dir.mkdir(parents=True)
    
    # Copy EWL libraries
    print(f"\nCopying EWL library files...")
    shutil.copytree(
        ewl_info.path,
        package_dir / "e200_ewl2",
        dirs_exist_ok=False,
    )
    
    # Create zip archive
    zip_path = output_dir / f"{PACKAGE_NAME}.zip"
    print(f"\nCreating package zip: {zip_path}")
    if zip_path.exists():
        zip_path.unlink()
    
    with zipfile.ZipFile(zip_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for file_path in package_dir.rglob('*'):
            if file_path.is_file():
                arcname = file_path.relative_to(package_dir.parent)
                zipf.write(file_path, arcname)
    
    # Calculate SHA256
    sha256 = calculate_sha256(zip_path)
    print(f"SHA256: {sha256}")
    
    # Save metadata
    metadata = {
        "version": VERSION,
        "sha256": sha256,
        "source": str(ewl_info.path),
        "files_count": ewl_info.files_count,
        "total_size": ewl_info.total_size,
    }
    
    metadata_path = output_dir / f"{PACKAGE_NAME}.metadata.json"
    with open(metadata_path, 'w') as f:
        json.dump(metadata, f, indent=2)
        f.write('\n')
    
    print(f"\n✓ Package created successfully:")
    print(f"  Zip: {zip_path}")
    print(f"  Size: {zip_path.stat().st_size / (1024*1024):.2f} MB")
    print(f"  Metadata: {metadata_path}")
    
    return zip_path


def main() -> int:
    """Main entry point."""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Build PlatformIO library package for EWL runtime libraries"
    )
    parser.add_argument(
        "installer_root",
        type=Path,
        help="Path to extracted S32DS installer root directory",
    )
    parser.add_argument(
        "--output",
        "-o",
        type=Path,
        default=Path(__file__).parent / "build",
        help="Output directory (default: build/)",
    )
    parser.add_argument(
        "--update-package-json",
        action="store_true",
        help="Update package.json with SHA256 and version",
    )
    
    args = parser.parse_args()
    
    if not args.installer_root.exists():
        print(f"Error: Installer directory not found: {args.installer_root}")
        return 1
    
    try:
        # Build package
        zip_path = extract_and_package(args.installer_root, args.output)
        
        # Update package.json if requested
        if args.update_package_json:
            package_json = Path(__file__).parent / "package.json"
            if package_json.exists():
                # Load metadata
                metadata_path = args.output / f"{PACKAGE_NAME}.metadata.json"
                if metadata_path.exists():
                    with open(metadata_path, "r") as f:
                        metadata = json.load(f)
                    
                    # Update package.json
                    with open(package_json, "r") as f:
                        pkg_data = json.load(f)
                    
                    pkg_data["sha256"]["linux_x86_64"] = metadata["sha256"]
                    pkg_data["version"] = VERSION
                    if not pkg_data["urls"].get("linux_x86_64", "").startswith("http"):
                        # Only update file:// URL if it's not already an HTTP URL
                        pkg_data["urls"]["linux_x86_64"] = f"file://{zip_path}"
                    
                    with open(package_json, "w") as f:
                        json.dump(pkg_data, f, indent=2)
                        f.write("\n")
                    
                    print(f"Updated package.json: {package_json}")
                else:
                    print(f"Warning: Metadata file not found: {metadata_path}")
        
        return 0
        
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())

