#!/usr/bin/env python3
"""
Analyze extracted S32 Design Studio installer structure.

This script parses the extracted S32DS installer directory and identifies
all components needed for PlatformIO toolchains and tools.
"""

from pathlib import Path
from typing import Dict, Optional, Tuple
import json
import sys
from dataclasses import dataclass
from enum import Enum


class ComponentType(Enum):
    """Component types in the S32DS installer."""
    TOOLCHAIN = "toolchain"
    PEGDBSERVER = "pegdbserver"
    EWL_LIBRARIES = "ewl_libraries"
    DRIVERS = "drivers"
    UNKNOWN = "unknown"


@dataclass
class ComponentInfo:
    """Information about a component found in the installer."""
    component_type: ComponentType
    path: Path
    version: Optional[str] = None
    description: Optional[str] = None
    files_count: int = 0
    total_size: int = 0


class S32DSInstallerAnalyzer:
    """Analyzer for extracted S32 Design Studio installer."""

    def __init__(self, installer_root: Path) -> None:
        """
        Initialize analyzer.

        Parameters
        ----------
        installer_root : Path
            Path to extracted S32DS installer root directory
        """
        self.installer_root = Path(installer_root)
        if not self.installer_root.exists():
            raise FileNotFoundError(
                f"Installer directory not found: {installer_root}"
            )
        self.layout_dir = (
            self.installer_root / "C_" / "MakingInstalers" / "Layout"
        )
        if not self.layout_dir.exists():
            raise FileNotFoundError(
                f"Layout directory not found: {self.layout_dir}"
            )

    def find_toolchain(self) -> Optional[ComponentInfo]:
        """
        Find the GCC toolchain in the installer.

        Returns
        -------
        Optional[ComponentInfo]
            Toolchain component information, or None if not found
        """
        toolchain_dir = (
            self.layout_dir / "Cross_Tools_zg_ia_sf" / "powerpc-eabivle-4_9"
        )
        if not toolchain_dir.exists():
            return None

        # Extract version from directory name or releasenotes
        version = "4.9.4"
        version_file = toolchain_dir / "releasenotes.pdf"
        if version_file.exists():
            # Could parse PDF but for now use directory name
            version = "4.9.4"

        files_count, total_size = self._count_files_and_size(toolchain_dir)

        return ComponentInfo(
            component_type=ComponentType.TOOLCHAIN,
            path=toolchain_dir,
            version=version,
            description="GCC PowerPC EABI VLE toolchain",
            files_count=files_count,
            total_size=total_size,
        )

    def find_pegdbserver(self) -> Optional[ComponentInfo]:
        """
        Find pegdbserver in the installer.

        Returns
        -------
        Optional[ComponentInfo]
            PEGDBServer component information, or None if not found
        """
        # Search for pegdbserver plugin in eclipse plugins
        plugins_dir = self.layout_dir / "eclipse_zg_ia_sf" / "plugins"
        if not plugins_dir.exists():
            return None

        # Search for pemicro debug plugin
        pemicro_plugins = list(
            plugins_dir.glob("com.pemicro.debug.gdbjtag.ppc_*")
        )
        if not pemicro_plugins:
            return None

        # Use the first found plugin (or latest version)
        plugin_dir = sorted(pemicro_plugins)[-1]

        # Extract version from directory name
        version = None
        if "_" in plugin_dir.name:
            parts = plugin_dir.name.split("_")
            if len(parts) >= 2:
                version = parts[-1]

        # Linux version is what we need
        lin_dir = plugin_dir / "lin"
        if not lin_dir.exists():
            return None

        files_count, total_size = self._count_files_and_size(lin_dir)

        return ComponentInfo(
            component_type=ComponentType.PEGDBSERVER,
            path=lin_dir,
            version=version,
            description="P&E Micro GDB Server for Power Architecture",
            files_count=files_count,
            total_size=total_size,
        )

    def find_ewl_libraries(self) -> Optional[ComponentInfo]:
        """
        Find EWL runtime libraries in the installer.

        Returns
        -------
        Optional[ComponentInfo]
            EWL libraries component information, or None if not found
        """
        ewl_dir = (
            self.layout_dir / "S32DS_zg_ia_sf" / "e200_ewl2"
        )
        if not ewl_dir.exists():
            return None

        files_count, total_size = self._count_files_and_size(ewl_dir)

        return ComponentInfo(
            component_type=ComponentType.EWL_LIBRARIES,
            path=ewl_dir,
            version=None,
            description="EWL Runtime Libraries for e200 cores",
            files_count=files_count,
            total_size=total_size,
        )

    def find_drivers(self) -> Optional[ComponentInfo]:
        """
        Find USB drivers in the installer.

        Returns
        -------
        Optional[ComponentInfo]
            Drivers component information, or None if not found
        """
        drivers_dir = (
            self.layout_dir / "Drivers_zg_ia_sf" / "libusb_64_32"
        )
        if not drivers_dir.exists():
            return None

        files_count, total_size = self._count_files_and_size(drivers_dir)

        return ComponentInfo(
            component_type=ComponentType.DRIVERS,
            path=drivers_dir,
            version=None,
            description="USB drivers for P&E Micro hardware",
            files_count=files_count,
            total_size=total_size,
        )

    def analyze(self) -> Dict[str, ComponentInfo]:
        """
        Analyze the entire installer and return all components.

        Returns
        -------
        Dict[str, ComponentInfo]
            Dictionary mapping component types to component info
        """
        components: Dict[str, ComponentInfo] = {}

        toolchain = self.find_toolchain()
        if toolchain:
            components["toolchain"] = toolchain

        pegdbserver = self.find_pegdbserver()
        if pegdbserver:
            components["pegdbserver"] = pegdbserver

        ewl = self.find_ewl_libraries()
        if ewl:
            components["ewl_libraries"] = ewl

        drivers = self.find_drivers()
        if drivers:
            components["drivers"] = drivers

        return components

    def _count_files_and_size(
        self, directory: Path
    ) -> Tuple[int, int]:
        """
        Count files and calculate total size in a directory tree.

        Parameters
        ----------
        directory : Path
            Directory to analyze

        Returns
        -------
        Tuple[int, int]
            Tuple of (file_count, total_size_bytes)
        """
        file_count = 0
        total_size = 0

        try:
            for path in directory.rglob("*"):
                if path.is_file():
                    file_count += 1
                    try:
                        total_size += path.stat().st_size
                    except OSError:
                        pass
        except OSError:
            pass

        return file_count, total_size

    def print_report(self) -> None:
        """Print a human-readable analysis report."""
        components = self.analyze()

        print("=" * 70)
        print("S32 Design Studio Installer Analysis")
        print("=" * 70)
        print(f"\nInstaller root: {self.installer_root}")
        print(f"Layout directory: {self.layout_dir}")
        print(f"\nFound {len(components)} component(s):\n")

        for name, component in components.items():
            size_mb = component.total_size / (1024 * 1024)
            print(f"  {name.upper()}")
            print(f"    Type:       {component.component_type.value}")
            print(f"    Path:       {component.path}")
            if component.version:
                print(f"    Version:    {component.version}")
            if component.description:
                print(f"    Description: {component.description}")
            print(f"    Files:      {component.files_count:,}")
            print(f"    Size:       {size_mb:.2f} MB")
            print()

    def export_json(self, output_path: Path) -> None:
        """
        Export analysis to JSON file.

        Parameters
        ----------
        output_path : Path
            Path to output JSON file
        """
        components = self.analyze()

        # Convert to JSON-serializable format
        json_data = {
            "installer_root": str(self.installer_root),
            "layout_dir": str(self.layout_dir),
            "components": {
                name: {
                    "component_type": comp.component_type.value,
                    "path": str(comp.path),
                    "version": comp.version,
                    "description": comp.description,
                    "files_count": comp.files_count,
                    "total_size": comp.total_size,
                }
                for name, comp in components.items()
            },
        }

        with open(output_path, "w") as f:
            json.dump(json_data, f, indent=2)

        print(f"Analysis exported to: {output_path}")


def main() -> int:
    """Main entry point."""
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <installer_root> [--json <output.json>]")
        return 1

    installer_root = Path(sys.argv[1])
    if not installer_root.exists():
        print(f"Error: Installer directory not found: {installer_root}")
        return 1

    analyzer = S32DSInstallerAnalyzer(installer_root)

    if "--json" in sys.argv:
        json_idx = sys.argv.index("--json")
        if json_idx + 1 < len(sys.argv):
            output_json = Path(sys.argv[json_idx + 1])
            analyzer.export_json(output_json)
        else:
            print("Error: --json requires output file path")
            return 1
    else:
        analyzer.print_report()

    return 0


if __name__ == "__main__":
    sys.exit(main())

