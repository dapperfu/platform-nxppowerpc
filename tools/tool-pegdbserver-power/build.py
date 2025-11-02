#!/usr/bin/env python3
"""
Build PlatformIO pegdbserver package from S32DS installer.

This script replaces the Makefile functionality with pure Python
to align with PlatformIO conventions.
"""

import sys
from pathlib import Path

# Add parent tools directory to path for imports
TOOLS_DIR = Path(__file__).parent.parent
sys.path.insert(0, str(TOOLS_DIR))

from extract_pegdbserver import PegdbServerExtractor
import json


def main() -> int:
    """Main entry point."""
    import argparse

    parser = argparse.ArgumentParser(
        description="Build PlatformIO pegdbserver package from S32DS installer"
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
        extractor = PegdbServerExtractor(args.installer_root, args.output)
        zip_path = extractor.extract()

        # Update package.json if requested
        if args.update_package_json:
            package_json = Path(__file__).parent / "package.json"
            if package_json.exists():
                # Load metadata
                metadata_path = args.output / "tool-pegdbserver-power.metadata.json"
                if metadata_path.exists():
                    with open(metadata_path, "r") as f:
                        metadata = json.load(f)

                    # Update package.json
                    with open(package_json, "r") as f:
                        pkg_data = json.load(f)

                    if "sha256" not in pkg_data:
                        pkg_data["sha256"] = {}
                    pkg_data["sha256"]["linux_x86_64"] = metadata["sha256"]
                    pkg_data["version"] = metadata["version"]
                    if not pkg_data["urls"].get("linux_x86_64", "").startswith("http"):
                        # Only update file:// URL if it's not already an HTTP URL
                        pkg_data["urls"]["linux_x86_64"] = f"file://{zip_path}"

                    with open(package_json, "w") as f:
                        json.dump(pkg_data, f, indent=2)
                        f.write("\n")

                    print(f"Updated package.json: {package_json}")
                else:
                    print(f"Warning: Metadata file not found: {metadata_path}")

        print(f"\n✓ Package built successfully: {zip_path}")
        return 0

    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())

