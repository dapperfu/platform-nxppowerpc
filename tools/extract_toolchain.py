#!/usr/bin/env python3
"""
Extract and package GCC toolchain from S32DS installer.

This script extracts the PowerPC EABI VLE toolchain from the extracted
S32 Design Studio installer and packages it for PlatformIO.
"""

from pathlib import Path
from typing import Optional
import shutil
import sys
import zipfile
import hashlib
import json
from datetime import datetime

from analyze_s32ds_installer import S32DSInstallerAnalyzer


class ToolchainExtractor:
    """Extract and package toolchain from S32DS installer."""

    def __init__(
        self,
        installer_root: Path,
        output_dir: Path,
    ) -> None:
        """
        Initialize extractor.

        Parameters
        ----------
        installer_root : Path
            Path to extracted S32DS installer root directory
        output_dir : Path
            Directory where package will be created
        """
        self.installer_root = Path(installer_root)
        self.output_dir = Path(output_dir)
        self.analyzer = S32DSInstallerAnalyzer(installer_root)

    def extract(self, package_name: str = "toolchain-powerpc-eabivle") -> Path:
        """
        Extract toolchain and create PlatformIO package zip.

        Parameters
        ----------
        package_name : str
            Name of the package (default: toolchain-powerpc-eabivle)

        Returns
        -------
        Path
            Path to created package zip file
        """
        # Find toolchain component
        toolchain_info = self.analyzer.find_toolchain()
        if not toolchain_info:
            raise FileNotFoundError(
                f"Toolchain not found in installer: {self.installer_root}"
            )

        print(f"Found toolchain: {toolchain_info.path}")
        print(f"  Files: {toolchain_info.files_count:,}")
        print(f"  Size: {toolchain_info.total_size / (1024*1024):.2f} MB")

        # Create output directory
        self.output_dir.mkdir(parents=True, exist_ok=True)

        # Create package directory structure
        package_dir = self.output_dir / package_name
        if package_dir.exists():
            print(f"Removing existing package directory: {package_dir}")
            shutil.rmtree(package_dir)

        package_dir.mkdir(parents=True)

        print(f"\nCopying toolchain files...")
        # Copy entire toolchain directory
        shutil.copytree(
            toolchain_info.path,
            package_dir / "powerpc-eabivle-4_9",
            dirs_exist_ok=False,
        )

        # Determine version
        version = toolchain_info.version or "4.9.4"
        # Try to extract build number from gcc binary
        gcc_path = (
            package_dir / "powerpc-eabivle-4_9" / "bin" / "powerpc-eabivle-gcc"
        )
        build_number = self._extract_gcc_build_number(gcc_path)

        # Create zip archive
        zip_path = self.output_dir / f"{package_name}.zip"
        print(f"\nCreating package zip: {zip_path}")
        if zip_path.exists():
            zip_path.unlink()

        with zipfile.ZipFile(
            zip_path, "w", zipfile.ZIP_DEFLATED
        ) as zipf:
            for file_path in package_dir.rglob("*"):
                if file_path.is_file():
                    arcname = file_path.relative_to(package_dir.parent)
                    zipf.write(file_path, arcname)

        # Calculate SHA256
        sha256 = self._calculate_sha256(zip_path)
        print(f"SHA256: {sha256}")

        # Save package metadata
        metadata = {
            "package_name": package_name,
            "version": f"{version}.{build_number}" if build_number else version,
            "source": str(toolchain_info.path),
            "files_count": toolchain_info.files_count,
            "total_size": toolchain_info.total_size,
            "sha256": sha256,
            "created": datetime.now().isoformat(),
        }

        metadata_path = self.output_dir / f"{package_name}.metadata.json"
        with open(metadata_path, "w") as f:
            json.dump(metadata, f, indent=2)

        print(f"\nPackage created successfully:")
        print(f"  Zip: {zip_path}")
        print(f"  Size: {zip_path.stat().st_size / (1024*1024):.2f} MB")
        print(f"  Metadata: {metadata_path}")

        return zip_path

    def _extract_gcc_build_number(self, gcc_path: Path) -> Optional[str]:
        """
        Extract GCC build number from binary.

        Parameters
        ----------
        gcc_path : Path
            Path to gcc binary

        Returns
        -------
        Optional[str]
            Build number if found, None otherwise
        """
        if not gcc_path.exists():
            return None

        try:
            # Try to get version string from binary
            import subprocess
            result = subprocess.run(
                [str(gcc_path), "--version"],
                capture_output=True,
                text=True,
                timeout=5,
            )
            if result.returncode == 0:
                # Look for build number in version string
                # Format: "gcc (Freescale ...) 4.9.4 20150629 (release) [g2724867]"
                version_str = result.stdout
                if "g2724867" in version_str:
                    return "2724867"
                # Try to extract any pattern like g[0-9]+
                import re
                match = re.search(r"\[g(\d+)\]", version_str)
                if match:
                    return match.group(1)
        except Exception:
            pass

        return None

    def _calculate_sha256(self, file_path: Path) -> str:
        """
        Calculate SHA256 hash of file.

        Parameters
        ----------
        file_path : Path
            Path to file

        Returns
        -------
        str
            SHA256 hash in hexadecimal
        """
        sha256_hash = hashlib.sha256()
        with open(file_path, "rb") as f:
            for byte_block in iter(lambda: f.read(4096), b""):
                sha256_hash.update(byte_block)
        return sha256_hash.hexdigest()


def main() -> int:
    """Main entry point."""
    if len(sys.argv) < 3:
        print(
            f"Usage: {sys.argv[0]} <installer_root> <output_dir> "
            "[package_name]"
        )
        print("\nExample:")
        print(
            f"  {sys.argv[0]} "
            "/home/jed/Downloads/extract_S32DS_Power_Linux/installer "
            "/tmp/toolchain-package"
        )
        return 1

    installer_root = Path(sys.argv[1])
    output_dir = Path(sys.argv[2])
    package_name = sys.argv[3] if len(sys.argv) > 3 else "toolchain-powerpc-eabivle"

    if not installer_root.exists():
        print(f"Error: Installer directory not found: {installer_root}")
        return 1

    try:
        extractor = ToolchainExtractor(installer_root, output_dir)
        extractor.extract(package_name)
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())

