#!/usr/bin/env python3
"""
Build PlatformIO framework package for NXP FreeRTOS MPC57xx.

This script downloads the NXP FreeRTOS release from GitHub and packages it
for PlatformIO distribution.
"""

import sys
import json
import hashlib
import zipfile
import tempfile
from pathlib import Path
import urllib.request


NXP_FREERTOS_URL = "https://github.com/dapperfu/platform-nxppowerpc/releases/download/v.0.0.1/freertos-9.0.0_MPC57XXX_public_rel_1.zip"
PACKAGE_NAME = "framework-freertos-nxp-mpc57xx"
VERSION = "9.0.0.1"


def calculate_sha256(file_path: Path) -> str:
    """Calculate SHA256 hash of a file."""
    sha256_hash = hashlib.sha256()
    with open(file_path, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            sha256_hash.update(chunk)
    return sha256_hash.hexdigest()


def download_file(url: str, dest: Path) -> Path:
    """Download a file from URL."""
    print(f"Downloading {url}...")
    urllib.request.urlretrieve(url, dest)
    return dest


def extract_and_repackage(zip_path: Path, output_dir: Path) -> Path:
    """Extract FreeRTOS zip and repackage as PlatformIO framework package."""
    print(f"Extracting {zip_path}...")
    
    # Extract to temp directory
    with tempfile.TemporaryDirectory() as tmpdir:
        extract_dir = Path(tmpdir) / "extract"
        extract_dir.mkdir()
        
        # Extract original zip
        with zipfile.ZipFile(zip_path, 'r') as zip_ref:
            zip_ref.extractall(extract_dir)
        
        # The zip contains: freertos-9.0.0_MPC57XXX_public_rel_1/FreeRTOS/
        # For PlatformIO framework, we want: FreeRTOS/ directly
        freertos_dir = extract_dir / "freertos-9.0.0_MPC57XXX_public_rel_1" / "FreeRTOS"
        if not freertos_dir.exists():
            raise Exception(f"Expected FreeRTOS directory not found in zip: {freertos_dir}")
        
        # Create package directory structure
        package_dir = output_dir / PACKAGE_NAME
        package_dir.mkdir(parents=True, exist_ok=True)
        
        # Copy FreeRTOS directory to package
        import shutil
        shutil.copytree(freertos_dir, package_dir / "FreeRTOS")
        
        # Create final zip
        output_zip = output_dir / f"{PACKAGE_NAME}.zip"
        print(f"Creating package zip: {output_zip}")
        
        with zipfile.ZipFile(output_zip, 'w', zipfile.ZIP_DEFLATED) as zip_out:
            for file_path in package_dir.rglob('*'):
                if file_path.is_file():
                    arcname = file_path.relative_to(package_dir.parent)
                    zip_out.write(file_path, arcname)
        
        return output_zip


def main() -> int:
    """Main entry point."""
    import argparse
    
    parser = argparse.ArgumentParser(
        description="Build PlatformIO framework package for NXP FreeRTOS"
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
        help="Update package.json with SHA256",
    )
    parser.add_argument(
        "--skip-download",
        action="store_true",
        help="Skip download, use existing zip file",
    )
    
    args = parser.parse_args()
    
    output_dir = args.output
    output_dir.mkdir(parents=True, exist_ok=True)
    
    # Download or use existing zip
    zip_path = output_dir / "freertos-source.zip"
    if not args.skip_download or not zip_path.exists():
        download_file(NXP_FREERTOS_URL, zip_path)
    
    # Calculate SHA256 of source zip
    source_sha256 = calculate_sha256(zip_path)
    print(f"Source zip SHA256: {source_sha256}")
    
    # Extract and repackage
    package_zip = extract_and_repackage(zip_path, output_dir)
    
    # Calculate SHA256 of package zip
    package_sha256 = calculate_sha256(package_zip)
    print(f"Package zip SHA256: {package_sha256}")
    
    # Update package.json if requested
    if args.update_package_json:
        package_json = Path(__file__).parent / "package.json"
        if package_json.exists():
            with open(package_json, 'r') as f:
                pkg_data = json.load(f)
            
            # Update with source zip SHA256 (what PlatformIO will download)
            pkg_data["sha256"]["linux_x86_64"] = source_sha256
            pkg_data["version"] = VERSION
            
            with open(package_json, 'w') as f:
                json.dump(pkg_data, f, indent=2)
                f.write('\n')
            
            print(f"Updated package.json: {package_json}")
    
    # Save metadata
    metadata = {
        "version": VERSION,
        "source_sha256": source_sha256,
        "package_sha256": package_sha256,
        "source_url": NXP_FREERTOS_URL,
    }
    
    metadata_path = output_dir / f"{PACKAGE_NAME}.metadata.json"
    with open(metadata_path, 'w') as f:
        json.dump(metadata, f, indent=2)
        f.write('\n')
    
    print(f"\n✓ Package built successfully: {package_zip}")
    print(f"  Version: {VERSION}")
    print(f"  Source SHA256: {source_sha256}")
    print(f"  Package SHA256: {package_sha256}")
    
    return 0


if __name__ == "__main__":
    sys.exit(main())

