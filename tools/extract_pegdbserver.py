#!/usr/bin/env python3
"""
Extract and package pegdbserver from S32DS installer.

This script extracts the P&E Micro GDB Server from the extracted
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


class PegdbServerExtractor:
    """Extract and package pegdbserver from S32DS installer."""

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

    def extract(self, package_name: str = "tool-pegdbserver-power") -> Path:
        """
        Extract pegdbserver and create PlatformIO package zip.

        Parameters
        ----------
        package_name : str
            Name of the package (default: tool-pegdbserver-power)

        Returns
        -------
        Path
            Path to created package zip file
        """
        # Find pegdbserver component
        pegdbserver_info = self.analyzer.find_pegdbserver()
        if not pegdbserver_info:
            raise FileNotFoundError(
                f"PEGDBServer not found in installer: {self.installer_root}"
            )

        print(f"Found pegdbserver: {pegdbserver_info.path}")
        print(f"  Version: {pegdbserver_info.version}")
        print(f"  Files: {pegdbserver_info.files_count:,}")
        print(f"  Size: {pegdbserver_info.total_size / (1024*1024):.2f} MB")

        # Create output directory
        self.output_dir.mkdir(parents=True, exist_ok=True)

        # Create package directory structure
        # PlatformIO expects: package_name/tools/pegdbserver/...
        package_root = self.output_dir / package_name
        if package_root.exists():
            print(f"Removing existing package directory: {package_root}")
            shutil.rmtree(package_root)

        package_root.mkdir(parents=True)
        tools_dir = package_root / "tools" / "pegdbserver"
        tools_dir.mkdir(parents=True)

        print(f"\nCopying pegdbserver files...")

        # Copy binary
        binary_src = pegdbserver_info.path / "pegdbserver_power_console"
        binary_dst = tools_dir / "bin" / "pegdbserver_power_console"
        binary_dst.parent.mkdir(parents=True, exist_ok=True)
        if binary_src.exists():
            shutil.copy2(binary_src, binary_dst)
            # Make executable
            binary_dst.chmod(0o755)
            print(f"  Copied binary: {binary_dst}")

        # Copy GDI directory (device definitions, flash algorithms, etc.)
        gdi_src = pegdbserver_info.path / "gdi"
        gdi_dst = tools_dir / "gdi"
        if gdi_src.exists():
            shutil.copytree(gdi_src, gdi_dst, dirs_exist_ok=False)
            print(f"  Copied GDI directory: {gdi_dst}")

        # Copy build_version.txt if it exists
        version_file_src = pegdbserver_info.path / "build_version.txt"
        version_file_dst = tools_dir / "build_version.txt"
        if version_file_src.exists():
            shutil.copy2(version_file_src, version_file_dst)
            print(f"  Copied version file: {version_file_dst}")

        # Extract version from path or file
        version = pegdbserver_info.version or self._extract_version_from_path(
            pegdbserver_info.path
        )

        # Create zip archive
        zip_path = self.output_dir / f"{package_name}.zip"
        print(f"\nCreating package zip: {zip_path}")
        if zip_path.exists():
            zip_path.unlink()

        with zipfile.ZipFile(
            zip_path, "w", zipfile.ZIP_DEFLATED
        ) as zipf:
            for file_path in package_root.rglob("*"):
                if file_path.is_file():
                    arcname = file_path.relative_to(package_root.parent)
                    zipf.write(file_path, arcname)

        # Calculate SHA256
        sha256 = self._calculate_sha256(zip_path)
        print(f"SHA256: {sha256}")

        # Save package metadata
        metadata = {
            "package_name": package_name,
            "version": version,
            "source": str(pegdbserver_info.path),
            "files_count": pegdbserver_info.files_count,
            "total_size": pegdbserver_info.total_size,
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

    def _extract_version_from_path(self, path: Path) -> str:
        """
        Extract version from plugin directory path.

        Parameters
        ----------
        path : Path
            Path to plugin directory

        Returns
        -------
        str
            Version string
        """
        # Path structure: .../com.pemicro.debug.gdbjtag.ppc_1.7.2.201709281658/lin
        # Try to find version in parent directory name
        parent = path.parent
        if "_" in parent.name:
            parts = parent.name.split("_")
            if len(parts) >= 2:
                return ".".join(parts[1:])

        return "1.7.2.201709281658"  # Default fallback

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
            "/tmp/pegdbserver-package"
        )
        return 1

    installer_root = Path(sys.argv[1])
    output_dir = Path(sys.argv[2])
    package_name = (
        sys.argv[3] if len(sys.argv) > 3 else "tool-pegdbserver-power"
    )

    if not installer_root.exists():
        print(f"Error: Installer directory not found: {installer_root}")
        return 1

    try:
        extractor = PegdbServerExtractor(installer_root, output_dir)
        extractor.extract(package_name)
        return 0
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())

