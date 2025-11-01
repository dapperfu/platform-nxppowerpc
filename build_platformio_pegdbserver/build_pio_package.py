#!/usr/bin/env python3
"""
PlatformIO Package Builder for P&E GDB Server

Automates the creation of a PlatformIO-compatible package from the
S32 Design Studio installation.
"""

import argparse
import os
import shutil
import subprocess
import sys
import json
import hashlib
from pathlib import Path
from typing import Dict, List, Optional
import logging

logging.basicConfig(level=logging.INFO, format='%(levelname)s: %(message)s')
logger = logging.getLogger(__name__)


class PIOPackageBuilder:
    """Build PlatformIO package from S32DS installation."""
    
    def __init__(self, s32ds_root: Path, output_dir: Path = None):
        """
        Initialize package builder.
        
        Parameters
        ----------
        s32ds_root : Path
            Root directory of S32 Design Studio installation
        output_dir : Path, optional
            Output directory for package (default: s32ds_root/platformio_package)
        """
        self.s32ds_root = Path(s32ds_root).resolve()
        self.output_dir = output_dir or (self.s32ds_root / "platformio_package")
        
        # Source paths
        self.server_plugin = self.s32ds_root / "plugins" / "com.pemicro.debug.gdbjtag.ppc_1.7.2.201709281658"
        self.server_binary = self.server_plugin / "lin" / "pegdbserver_power_console"
        self.gdi_dir = self.server_plugin / "lin" / "gdi"
        self.pemicro_dir = self.gdi_dir / "P&E"
        
        # Package structure
        self.package_name = "tool-pegdbserver-power"
        self.package_root = self.output_dir / self.package_name
        
    def validate_sources(self) -> bool:
        """Validate that all required source files exist."""
        logger.info("Validating source files...")
        
        missing = []
        if not self.server_binary.exists():
            missing.append(f"Server binary: {self.server_binary}")
        if not self.gdi_dir.exists():
            missing.append(f"GDI directory: {self.gdi_dir}")
        if not self.pemicro_dir.exists():
            missing.append(f"P&E directory: {self.pemicro_dir}")
            
        if missing:
            logger.error("Missing required files:")
            for item in missing:
                logger.error(f"  - {item}")
            return False
            
        logger.info("✓ All source files found")
        return True
    
    def create_package_structure(self):
        """Create package directory structure."""
        logger.info("Creating package structure...")
        
        dirs = [
            self.package_root / "tools" / "pegdbserver" / "bin",
            self.package_root / "tools" / "pegdbserver" / "gdi" / "P&E",
        ]
        
        for dir_path in dirs:
            dir_path.mkdir(parents=True, exist_ok=True)
            
        logger.info(f"✓ Package structure created: {self.package_root}")
    
    def copy_files(self):
        """Copy all required files to package."""
        logger.info("Copying files...")
        
        # Copy binary
        binary_dest = self.package_root / "tools" / "pegdbserver" / "bin" / "pegdbserver_power_console"
        shutil.copy2(self.server_binary, binary_dest)
        os.chmod(binary_dest, 0o755)
        logger.info(f"  ✓ Binary: {binary_dest.name}")
        
        # Copy GDI internal library
        gdi_lib = self.gdi_dir / "unit_ngs_ppcnexus_internal.so"
        if gdi_lib.exists():
            shutil.copy2(gdi_lib, self.package_root / "tools" / "pegdbserver" / "gdi")
            logger.info(f"  ✓ GDI library: {gdi_lib.name}")
        
        # Copy P&E directory contents
        pemicro_dest = self.package_root / "tools" / "pegdbserver" / "gdi" / "P&E"
        if self.pemicro_dir.exists():
            for item in self.pemicro_dir.iterdir():
                if item.is_file():
                    shutil.copy2(item, pemicro_dest)
                elif item.is_dir():
                    shutil.copytree(item, pemicro_dest / item.name, dirs_exist_ok=True)
            
            # Count files by type
            add_files = list(pemicro_dest.glob("*.add"))
            pcp_files = list(pemicro_dest.glob("*.pcp"))
            so_files = list(pemicro_dest.glob("*.so"))
            xml_files = list(self.package_root.rglob("*.xml"))
            mac_files = list(pemicro_dest.glob("*.mac"))
            
            logger.info(f"  ✓ Device files (.add): {len(add_files)}")
            logger.info(f"  ✓ Flash algorithms (.pcp): {len(pcp_files)}")
            logger.info(f"  ✓ Libraries (.so): {len(so_files)}")
            logger.info(f"  ✓ XML files: {len(xml_files)}")
            logger.info(f"  ✓ Macro files (.mac): {len(mac_files)}")
        
        # Copy XML files from gdi root
        for xml_file in self.gdi_dir.glob("*.xml"):
            shutil.copy2(xml_file, self.package_root / "tools" / "pegdbserver" / "gdi")
    
    def create_package_json(self):
        """Create package.json manifest."""
        logger.info("Creating package.json...")
        
        package_info = {
            "name": self.package_name,
            "version": "1.7.2.201709281658",
            "description": "P&E Microcomputer Systems GDB Server for Power Architecture microcontrollers",
            "keywords": [
                "platformio",
                "tools",
                "debugger",
                "opensda",
                "pemicro",
                "powerpc",
                "e200",
                "mpc57xx",
                "mpc56xx",
                "nxp"
            ],
            "homepage": "https://www.pemicro.com",
            "license": "Proprietary",
            "system": [
                "linux_x86_64"
            ]
        }
        
        json_path = self.package_root / "package.json"
        with open(json_path, 'w') as f:
            json.dump(package_info, f, indent=2)
            
        logger.info(f"  ✓ Created: {json_path.name}")
    
    def create_readme(self):
        """Create README.md documentation."""
        logger.info("Creating README.md...")
        
        readme_content = f"""# P&E GDB Server for PlatformIO

This package contains the P&E Microcomputer Systems GDB server (`pegdbserver_power_console`) 
and all required dependencies for use with PlatformIO board support packages.

## Contents

- `tools/pegdbserver/bin/pegdbserver_power_console` - Main GDB server executable
- `tools/pegdbserver/gdi/P&E/` - Device definitions and flash algorithms
  - `pedebug_ppcnexus_*.add` - Device definition files
  - `nxp_*.pcp` - Flash programming algorithm files
  - `*.so` - Required shared libraries
  - `*.xml` - Device target XML files
  - `*.mac` - Macro files for device initialization

## Usage in PlatformIO

To use this in a PlatformIO board support package, include this directory structure
in your board package.

### Example platformio.ini

```ini
[env:mpc5748g]
platform = nxps32
board = mpc5748g
upload_protocol = pemicro
debug_tool = pemicro
upload_command = $PROJECT_PACKAGES_DIR/tool-pegdbserver-power/bin/pegdbserver_power_console
```

### Command Line Arguments

- `-device=<name>` - Target device (e.g., MPC5748G)
- `-interface=OPENSDA` - Hardware interface type
- `-port=<string>` - Hardware port (e.g., USB1)
- `-speed=<kHz>` - JTAG speed (default: 5000)
- `-startserver` - Start GDB server
- `-serverport=<n>` - Server TCP port (default: 7224)
- `-gdbmiport=<n>` - GDB/MI TCP port (default: 6224)
- `-singlesession` - Allow only one session
- `-devicelist` - List supported devices
- `-getportlist` - List available hardware ports

## Version

- Version: 1.7.2.201709281658
- P&E DLL Version: v651
- Build Date: 170928
- Source: S32 Design Studio for Power Architecture 2017.R1
"""
        
        readme_path = self.package_root / "README.md"
        with open(readme_path, 'w') as f:
            f.write(readme_content)
            
        logger.info(f"  ✓ Created: {readme_path.name}")
    
    def create_archive(self, output_file: Path = None) -> Path:
        """Create zip archive of package."""
        logger.info("Creating archive...")
        
        if output_file is None:
            output_file = self.output_dir / f"{self.package_name}.zip"
        
        # Remove existing archive if present
        if output_file.exists():
            output_file.unlink()
        
        # Create archive
        shutil.make_archive(
            str(output_file.with_suffix('')),
            'zip',
            root_dir=self.package_root.parent,
            base_dir=self.package_name
        )
        
        # Calculate size and hash
        size_mb = output_file.stat().st_size / (1024 * 1024)
        with open(output_file, 'rb') as f:
            sha256 = hashlib.sha256(f.read()).hexdigest()
        
        logger.info(f"  ✓ Archive created: {output_file.name} ({size_mb:.1f} MB)")
        logger.info(f"  ✓ SHA256: {sha256}")
        
        # Create checksum file
        checksum_file = output_file.with_suffix('.sha256')
        with open(checksum_file, 'w') as f:
            f.write(f"{sha256}  {output_file.name}\n")
        logger.info(f"  ✓ Checksum file: {checksum_file.name}")
        
        return output_file
    
    def verify_package(self) -> bool:
        """Verify package contents."""
        logger.info("Verifying package...")
        
        checks = [
            (self.package_root / "tools" / "pegdbserver" / "bin" / "pegdbserver_power_console", "Binary"),
            (self.package_root / "tools" / "pegdbserver" / "gdi" / "P&E", "P&E directory"),
            (self.package_root / "package.json", "package.json"),
        ]
        
        all_ok = True
        for path, name in checks:
            if not path.exists():
                logger.error(f"  ✗ Missing: {name}")
                all_ok = False
            else:
                logger.info(f"  ✓ Found: {name}")
        
        # Check file counts
        add_files = len(list((self.package_root / "tools" / "pegdbserver" / "gdi" / "P&E").glob("*.add")))
        pcp_files = len(list((self.package_root / "tools" / "pegdbserver" / "gdi" / "P&E").glob("*.pcp")))
        
        if add_files == 0:
            logger.warning(f"  ⚠ No device files (.add) found")
        else:
            logger.info(f"  ✓ Device files: {add_files}")
            
        if pcp_files == 0:
            logger.warning(f"  ⚠ No flash algorithms (.pcp) found")
        else:
            logger.info(f"  ✓ Flash algorithms: {pcp_files}")
        
        return all_ok
    
    def build(self, create_archive: bool = True) -> Optional[Path]:
        """
        Build the complete package.
        
        Parameters
        ----------
        create_archive : bool
            Whether to create zip archive
            
        Returns
        -------
        Optional[Path]
            Path to archive if created, None otherwise
        """
        logger.info(f"Building PlatformIO package: {self.package_name}")
        logger.info(f"Source: {self.s32ds_root}")
        logger.info(f"Output: {self.package_root}")
        logger.info("")
        
        if not self.validate_sources():
            return None
        
        self.create_package_structure()
        self.copy_files()
        self.create_package_json()
        self.create_readme()
        
        if not self.verify_package():
            logger.warning("Package verification found issues, but continuing...")
        
        archive = None
        if create_archive:
            archive = self.create_archive()
        
        logger.info("")
        logger.info("✓ Package build complete!")
        
        return archive


def main():
    """Main entry point."""
    parser = argparse.ArgumentParser(
        description="Build PlatformIO package from S32DS installation"
    )
    parser.add_argument(
        '--s32ds-root',
        type=Path,
        default=Path.cwd(),
        help='Root directory of S32 Design Studio installation'
    )
    parser.add_argument(
        '--output-dir',
        type=Path,
        default=None,
        help='Output directory for package (default: s32ds_root/platformio_package)'
    )
    parser.add_argument(
        '--no-archive',
        action='store_true',
        help='Skip creating zip archive'
    )
    parser.add_argument(
        '--verbose',
        action='store_true',
        help='Enable verbose output'
    )
    
    args = parser.parse_args()
    
    if args.verbose:
        logging.getLogger().setLevel(logging.DEBUG)
    
    builder = PIOPackageBuilder(
        s32ds_root=args.s32ds_root,
        output_dir=args.output_dir
    )
    
    archive = builder.build(create_archive=not args.no_archive)
    
    if archive:
        print(f"\n✅ Package archive: {archive}")
        return 0
    else:
        print("\n✅ Package built (no archive created)")
        return 0


if __name__ == '__main__':
    sys.exit(main())

