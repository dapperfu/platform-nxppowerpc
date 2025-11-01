# PlatformIO Package Builder for P&E GDB Server

Automated build system for creating PlatformIO-compatible packages from S32 Design Studio installation.

## Quick Start

```bash
# Build package with archive
make package

# Build without archive (faster iteration)
make build

# Verify package contents
make verify

# Clean build artifacts
make clean

# Rebuild everything
make rebuild

# Run tests
make test
```

## Requirements

- Python 3.8+
- Make (optional but recommended)
- Standard Unix tools (zip, unzip)

## Usage

### Using Make (Recommended)

```bash
# From the scripts directory
make package

# Custom source location
make package S32DS_ROOT=/path/to/s32ds
```

### Using Python Script Directly

```bash
python3 build_pio_package.py

# With options
python3 build_pio_package.py \
  --s32ds-root /path/to/s32ds \
  --output-dir /path/to/output \
  --verbose
```

## Output Structure

```
platformio_package/
├── tool-pegdbserver-power/
│   ├── package.json          # PlatformIO manifest
│   ├── README.md            # Documentation
│   └── tools/
│       └── pegdbserver/
│           ├── bin/
│           │   └── pegdbserver_power_console
│           └── gdi/
│               ├── unit_ngs_ppcnexus_internal.so
│               └── P&E/
│                   ├── *.add (65 device files)
│                   ├── *.pcp (202 flash algorithms)
│                   └── *.so, *.xml, *.mac
├── tool-pegdbserver-power.zip
└── tool-pegdbserver-power.zip.sha256
```

## CI/CD

### GitHub Actions

The `.github/workflows/build_package.yml` workflow automatically:
- Builds on tag pushes
- Creates GitHub releases
- Uploads artifacts

### Local Testing

```bash
make test        # Test package integrity
make verify      # Verify all files present
```

## Integration

The generated package can be integrated into PlatformIO board packages or installed manually:

```bash
# Manual installation
unzip tool-pegdbserver-power.zip -d ~/.platformio/packages/
```

## Development

```bash
# Make changes to build script
vim build_pio_package.py

# Test locally
make rebuild

# Commit changes
git add .
git commit -m "Update package builder"
```
