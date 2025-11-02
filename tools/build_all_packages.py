#!/usr/bin/env python3
"""
Master script to build all PlatformIO packages from S32DS installer.

This script coordinates the extraction and packaging of all toolchain and
tool packages from an extracted S32 Design Studio installer.
"""

from pathlib import Path
from typing import Dict, Optional
import sys
import argparse
import json
import subprocess

from analyze_s32ds_installer import S32DSInstallerAnalyzer
from extract_toolchain import ToolchainExtractor
from extract_pegdbserver import PegdbServerExtractor


class PackageBuilder:
    """Build all PlatformIO packages from S32DS installer."""

    def __init__(
        self,
        installer_root: Path,
        output_base: Path,
        platform_root: Optional[Path] = None,
    ) -> None:
        """
        Initialize package builder.

        Parameters
        ----------
        installer_root : Path
            Path to extracted S32DS installer root directory
        output_base : Path
            Base directory for package output
        platform_root : Optional[Path]
            Platform root directory (for updating package.json files)
        """
        self.installer_root = Path(installer_root)
        self.output_base = Path(output_base)
        self.platform_root = Path(platform_root) if platform_root else None

    def build_toolchain(self) -> Path:
        """
        Build toolchain package.

        Returns
        -------
        Path
            Path to created package zip file
        """
        print("=" * 70)
        print("Building Toolchain Package")
        print("=" * 70)

        output_dir = self.output_base / "toolchain"
        extractor = ToolchainExtractor(self.installer_root, output_dir)
        zip_path = extractor.extract()

        # Update package.json if platform_root is provided
        if self.platform_root:
            self._update_package_json(
                self.platform_root / "tools" / "toolchain-powerpc-eabivle" / "package.json",
                zip_path,
            )

        return zip_path

    def build_pegdbserver(self) -> Path:
        """
        Build pegdbserver package.

        Returns
        -------
        Path
            Path to created package zip file
        """
        print("\n" + "=" * 70)
        print("Building PEGDBServer Package")
        print("=" * 70)

        output_dir = self.output_base / "pegdbserver"
        extractor = PegdbServerExtractor(self.installer_root, output_dir)
        zip_path = extractor.extract()

        # Update package.json if platform_root is provided
        if self.platform_root:
            self._update_package_json(
                self.platform_root / "tools" / "tool-pegdbserver-power" / "package.json",
                zip_path,
            )

        return zip_path

    def build_ewl_library(self) -> Path:
        """
        Build EWL library package.

        Returns
        -------
        Path
            Path to created package zip file
        """
        print("\n" + "=" * 70)
        print("Building EWL Library Package")
        print("=" * 70)

        output_dir = self.output_base / "ewl_library"
        build_script = Path(__file__).parent / "library-ewl-powerpc-eabivle" / "build.py"
        
        if not build_script.exists():
            raise FileNotFoundError(
                f"EWL library build script not found: {build_script}"
            )

        # Run the build script
        result = subprocess.run(
            [
                sys.executable,
                str(build_script),
                str(self.installer_root),
                "--output",
                str(output_dir),
                "--update-package-json",
            ],
            capture_output=False,
            text=True,
        )

        if result.returncode != 0:
            raise RuntimeError(f"EWL library build failed with code {result.returncode}")

        # Find the created zip file
        zip_path = output_dir / "library-ewl-powerpc-eabivle.zip"
        if not zip_path.exists():
            raise FileNotFoundError(f"Expected zip file not found: {zip_path}")

        return zip_path

    def build_all(self) -> Dict[str, Optional[Path]]:
        """
        Build all packages.

        Returns
        -------
        Dict[str, Optional[Path]]
            Dictionary mapping package names to zip paths (None if failed)
        """
        print("=" * 70)
        print("Building All PlatformIO Packages from S32DS Installer")
        print("=" * 70)
        print(f"\nInstaller: {self.installer_root}")
        print(f"Output:   {self.output_base}")
        if self.platform_root:
            print(f"Platform: {self.platform_root}")

        results = {}

        # Analyze installer first
        print("\n" + "=" * 70)
        print("Analyzing Installer")
        print("=" * 70)
        analyzer = S32DSInstallerAnalyzer(self.installer_root)
        analyzer.print_report()

        # Build toolchain
        toolchain_zip: Optional[Path] = None
        try:
            toolchain_zip = self.build_toolchain()
            results["toolchain"] = toolchain_zip
        except Exception as e:
            print(f"ERROR: Failed to build toolchain: {e}", file=sys.stderr)
            import traceback
            traceback.print_exc()
            results["toolchain"] = None

        # Build pegdbserver
        pegdbserver_zip: Optional[Path] = None
        try:
            pegdbserver_zip = self.build_pegdbserver()
            results["pegdbserver"] = pegdbserver_zip
        except Exception as e:
            print(f"ERROR: Failed to build pegdbserver: {e}", file=sys.stderr)
            import traceback
            traceback.print_exc()
            results["pegdbserver"] = None

        # Build EWL library
        ewl_zip: Optional[Path] = None
        try:
            ewl_zip = self.build_ewl_library()
            results["ewl_library"] = ewl_zip
        except Exception as e:
            print(f"ERROR: Failed to build EWL library: {e}", file=sys.stderr)
            import traceback
            traceback.print_exc()
            results["ewl_library"] = None

        # Print summary
        print("\n" + "=" * 70)
        print("Build Summary")
        print("=" * 70)
        for package_name, zip_path in results.items():
            if zip_path:
                size_mb = zip_path.stat().st_size / (1024 * 1024)
                print(f"\n  {package_name}:")
                print(f"    ✓ Package: {zip_path}")
                print(f"    Size: {size_mb:.2f} MB")
            else:
                print(f"\n  {package_name}:")
                print(f"    ✗ Failed")

        return results

    def _update_package_json(self, package_json_path: Path, zip_path: Path) -> None:
        """
        Update package.json with local file URL and SHA256.

        Parameters
        ----------
        package_json_path : Path
            Path to package.json file
        zip_path : Path
            Path to package zip file
        """
        if not package_json_path.exists():
            print(f"  Warning: package.json not found: {package_json_path}")
            return

        # Load metadata if available
        metadata_path = zip_path.parent / f"{zip_path.stem}.metadata.json"
        sha256 = None
        version = None
        if metadata_path.exists():
            with open(metadata_path, "r") as f:
                metadata = json.load(f)
                sha256 = metadata.get("sha256")
                version = metadata.get("version")

        # Calculate SHA256 if not in metadata
        if not sha256:
            import hashlib
            sha256_hash = hashlib.sha256()
            with open(zip_path, "rb") as f:
                for byte_block in iter(lambda: f.read(4096), b""):
                    sha256_hash.update(byte_block)
            sha256 = sha256_hash.hexdigest()

        # Read existing package.json
        with open(package_json_path, "r") as f:
            package_json = json.load(f)

        # Update URLs to use file:// path
        file_url = f"file://{zip_path}"
        if "urls" not in package_json:
            package_json["urls"] = {}
        package_json["urls"]["linux_x86_64"] = file_url

        # Update SHA256
        if "sha256" not in package_json:
            package_json["sha256"] = {}
        package_json["sha256"]["linux_x86_64"] = sha256

        # Update version if available
        if version and "version" in package_json:
            package_json["version"] = version

        # Write updated package.json
        with open(package_json_path, "w") as f:
            json.dump(package_json, f, indent=2)
            f.write("\n")

        print(f"  Updated package.json: {package_json_path}")
        print(f"    URL: {file_url}")
        print(f"    SHA256: {sha256}")


def main() -> int:
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Build all PlatformIO packages from S32DS installer"
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
        default=Path("/tmp/pio-packages"),
        help="Output directory for packages (default: /tmp/pio-packages)",
    )
    parser.add_argument(
        "--platform-root",
        "-p",
        type=Path,
        help="Platform root directory (for updating package.json files)",
    )
    parser.add_argument(
        "--toolchain-only",
        action="store_true",
        help="Only build toolchain package",
    )
    parser.add_argument(
        "--pegdbserver-only",
        action="store_true",
        help="Only build pegdbserver package",
    )
    parser.add_argument(
        "--ewl-only",
        action="store_true",
        help="Only build EWL library package",
    )

    args = parser.parse_args()

    if not args.installer_root.exists():
        print(f"Error: Installer directory not found: {args.installer_root}")
        return 1

    builder = PackageBuilder(
        args.installer_root,
        args.output,
        args.platform_root,
    )

    try:
        if args.toolchain_only:
            builder.build_toolchain()
        elif args.pegdbserver_only:
            builder.build_pegdbserver()
        elif args.ewl_only:
            builder.build_ewl_library()
        else:
            builder.build_all()
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())

