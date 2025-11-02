# Makefile for PlatformIO PEGDBServer Package Builder
# Builds the pegdbserver package from S32DS installer

# Variables
PEGDBSERVER_NAME := tool-pegdbserver-power
PEGDBSERVER_VERSION := 1.7.2.201709281658

# S32DS Installer path (optional, for extracting from installer)
S32DS_INSTALLER_ROOT := /home/jed/Downloads/extract_S32DS_Power_Linux/installer

SCRIPT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
TOOLS_DIR := ${SCRIPT_DIR}/..
EXTRACT_SCRIPT := ${TOOLS_DIR}/extract_pegdbserver.py
BUILD_DIR := ${SCRIPT_DIR}/build
PACKAGE_JSON := ${SCRIPT_DIR}/package.json
OUTPUT_ZIP := ${BUILD_DIR}/${PEGDBSERVER_NAME}.zip

PYTHON := python3
ZIP := zip

.PHONY: help clean from-installer verify rebuild

help:
	@echo "PlatformIO PEGDBServer Package Builder"
	@echo ""
	@echo "Targets:"
	@echo "  from-installer - Extract and package from S32DS installer (use S32DS_INSTALLER_ROOT)"
	@echo "  verify      - Verify package contents"
	@echo "  clean       - Remove build artifacts"
	@echo "  rebuild     - Clean and rebuild everything"
	@echo ""
	@echo "Variables:"
	@echo "  PEGDBSERVER_NAME=${PEGDBSERVER_NAME}"
	@echo "  PEGDBSERVER_VERSION=${PEGDBSERVER_VERSION}"
	@echo "  S32DS_INSTALLER_ROOT=${S32DS_INSTALLER_ROOT}"
	@echo "  BUILD_DIR=${BUILD_DIR}"

from-installer: clean
	@echo "Extracting pegdbserver from S32DS installer..."
	@if [ ! -d "${S32DS_INSTALLER_ROOT}" ]; then \
		echo "✗ S32DS installer directory not found: ${S32DS_INSTALLER_ROOT}"; \
		echo "  Set S32DS_INSTALLER_ROOT variable to point to extracted installer"; \
		exit 1; \
	fi
	@mkdir -p ${BUILD_DIR}
	@${PYTHON} ${EXTRACT_SCRIPT} "${S32DS_INSTALLER_ROOT}" "${BUILD_DIR}" ${PEGDBSERVER_NAME}
	@echo ""
	@echo "Updating package.json with SHA256..."
	@if [ -f "${BUILD_DIR}/${PEGDBSERVER_NAME}.metadata.json" ]; then \
		SHA256=$$(${PYTHON} -c "import json; f=open('${BUILD_DIR}/${PEGDBSERVER_NAME}.metadata.json'); d=json.load(f); print(d['sha256'])"); \
		${PYTHON} -c " \
			import json, sys; \
			f = open('${PACKAGE_JSON}', 'r'); \
			data = json.load(f); \
			f.close(); \
			if 'sha256' not in data: \
				data['sha256'] = {}; \
			data['sha256']['linux_x86_64'] = '$${SHA256}'; \
			data['version'] = '$$(${PYTHON} -c \"import json; f=open('${BUILD_DIR}/${PEGDBSERVER_NAME}.metadata.json'); d=json.load(f); print(d['version'])\")'; \
			f = open('${PACKAGE_JSON}', 'w'); \
			json.dump(data, f, indent=2); \
			f.write('\n'); \
			f.close(); \
		"; \
		echo "✓ SHA256 and version updated in package.json"; \
	else \
		echo "⚠ Metadata file not found, run update-sha256 manually"; \
	fi
	@echo ""
	@echo "✓ Package build complete from installer!"
	@echo "  Package: ${OUTPUT_ZIP}"

verify: ${OUTPUT_ZIP}
	@echo "Verifying package..."
	@if [ ! -f "${OUTPUT_ZIP}" ]; then \
		echo "✗ Package zip not found"; \
		exit 1; \
	fi
	@echo "  Checking zip integrity..."
	@unzip -t -q "${OUTPUT_ZIP}" || (echo "✗ Zip file is corrupted" && exit 1)
	@echo "  Checking for required files..."
	@if unzip -l "${OUTPUT_ZIP}" | grep -q "pegdbserver_power_console"; then \
		echo "✓ pegdbserver binary found"; \
	else \
		echo "✗ pegdbserver binary not found"; \
		exit 1; \
	fi
	@if unzip -l "${OUTPUT_ZIP}" | grep -q "gdi/P&E/"; then \
		echo "✓ GDI directory found"; \
	else \
		echo "✗ GDI directory not found"; \
		exit 1; \
	fi
	@echo "✓ Package verification passed"

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf ${BUILD_DIR}
	@echo "✓ Clean complete"

rebuild: clean from-installer verify

