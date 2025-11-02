# Makefile for PlatformIO Toolchain Package Builder
# Builds the toolchain package from pre-built binaries (golden source)

# Variables
TOOLCHAIN_NAME := toolchain-powerpc-eabivle
TOOLCHAIN_VERSION := 4.9.4.2724867
GOLDEN_SOURCE_URL := https://github.com/dapperfu/platform-nxppowerpc/releases/download/v.0.0.1/gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip
GOLDEN_SOURCE_FILE := gcc-4.9.4-Ee200-eabivle-x86_64-linux-g2724867.zip

# S32DS Installer path (optional, for extracting from installer)
S32DS_INSTALLER_ROOT := /home/jed/Downloads/extract_S32DS_Power_Linux/installer

SCRIPT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
TOOLS_DIR := ${SCRIPT_DIR}/..
EXTRACT_SCRIPT := ${TOOLS_DIR}/extract_toolchain.py
BUILD_DIR := ${SCRIPT_DIR}/build
DOWNLOAD_DIR := ${BUILD_DIR}/download
EXTRACT_DIR := ${BUILD_DIR}/extract
PACKAGE_DIR := ${BUILD_DIR}/${TOOLCHAIN_NAME}
PACKAGE_JSON := ${SCRIPT_DIR}/package.json
OUTPUT_ZIP := ${BUILD_DIR}/${TOOLCHAIN_NAME}.zip
OUTPUT_SHA256 := ${OUTPUT_ZIP}.sha256

PYTHON := python3
WGET := wget
UNZIP := unzip
ZIP := zip

.PHONY: help clean download extract package verify update-sha256 rebuild test all from-installer

help:
	@echo "PlatformIO Toolchain Package Builder"
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build complete package (download, extract, package, verify)"
	@echo "  from-installer - Extract and package from S32DS installer (use S32DS_INSTALLER_ROOT)"
	@echo "  download    - Download golden source binary"
	@echo "  extract     - Extract downloaded toolchain"
	@echo "  package     - Create PlatformIO package zip"
	@echo "  update-sha256 - Calculate and update SHA256 in package.json"
	@echo "  verify      - Verify package contents"
	@echo "  clean       - Remove build artifacts"
	@echo "  rebuild     - Clean and rebuild everything"
	@echo "  test        - Test the built package"
	@echo ""
	@echo "Variables:"
	@echo "  TOOLCHAIN_NAME=${TOOLCHAIN_NAME}"
	@echo "  TOOLCHAIN_VERSION=${TOOLCHAIN_VERSION}"
	@echo "  GOLDEN_SOURCE_URL=${GOLDEN_SOURCE_URL}"
	@echo "  S32DS_INSTALLER_ROOT=${S32DS_INSTALLER_ROOT}"
	@echo "  BUILD_DIR=${BUILD_DIR}"

all: clean download extract package update-sha256 verify
	@echo ""
	@echo "✓ Package build complete!"
	@echo "  Package: ${OUTPUT_ZIP}"
	@echo "  SHA256:  ${OUTPUT_SHA256}"

from-installer: clean
	@echo "Extracting toolchain from S32DS installer..."
	@if [ ! -d "${S32DS_INSTALLER_ROOT}" ]; then \
		echo "✗ S32DS installer directory not found: ${S32DS_INSTALLER_ROOT}"; \
		echo "  Set S32DS_INSTALLER_ROOT variable to point to extracted installer"; \
		exit 1; \
	fi
	@mkdir -p ${BUILD_DIR}
	@${PYTHON} ${EXTRACT_SCRIPT} "${S32DS_INSTALLER_ROOT}" "${BUILD_DIR}" ${TOOLCHAIN_NAME}
	@echo ""
	@echo "Updating package.json with SHA256..."
	@if [ -f "${BUILD_DIR}/${TOOLCHAIN_NAME}.metadata.json" ]; then \
		SHA256=$$(${PYTHON} -c "import json; f=open('${BUILD_DIR}/${TOOLCHAIN_NAME}.metadata.json'); d=json.load(f); print(d['sha256'])"); \
		${PYTHON} -c " \
			import json, sys; \
			f = open('${PACKAGE_JSON}', 'r'); \
			data = json.load(f); \
			f.close(); \
			data['sha256']['linux_x86_64'] = '$${SHA256}'; \
			data['version'] = '$$(${PYTHON} -c \"import json; f=open('${BUILD_DIR}/${TOOLCHAIN_NAME}.metadata.json'); d=json.load(f); print(d['version'])\")'; \
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
	@echo "  Package: ${BUILD_DIR}/${TOOLCHAIN_NAME}.zip"

download: ${DOWNLOAD_DIR}/${GOLDEN_SOURCE_FILE}

${DOWNLOAD_DIR}/${GOLDEN_SOURCE_FILE}:
	@echo "Downloading golden source toolchain..."
	@mkdir -p ${DOWNLOAD_DIR}
	@if [ ! -f "${DOWNLOAD_DIR}/${GOLDEN_SOURCE_FILE}" ]; then \
		echo "  URL: ${GOLDEN_SOURCE_URL}"; \
		${WGET} -O "${DOWNLOAD_DIR}/${GOLDEN_SOURCE_FILE}" "${GOLDEN_SOURCE_URL}" || \
		curl -L -o "${DOWNLOAD_DIR}/${GOLDEN_SOURCE_FILE}" "${GOLDEN_SOURCE_URL}" || \
		(echo "✗ Download failed. Please ensure wget or curl is installed." && exit 1); \
		echo "✓ Downloaded: ${GOLDEN_SOURCE_FILE}"; \
	else \
		echo "✓ Already downloaded: ${GOLDEN_SOURCE_FILE}"; \
	fi

extract: download ${EXTRACT_DIR}/.extracted

${EXTRACT_DIR}/.extracted: ${DOWNLOAD_DIR}/${GOLDEN_SOURCE_FILE}
	@echo "Extracting toolchain..."
	@mkdir -p ${EXTRACT_DIR}
	@${UNZIP} -q -o "${DOWNLOAD_DIR}/${GOLDEN_SOURCE_FILE}" -d "${EXTRACT_DIR}" || \
		(echo "✗ Extraction failed. Ensure unzip is installed." && exit 1)
	@touch ${EXTRACT_DIR}/.extracted
	@echo "✓ Extracted to: ${EXTRACT_DIR}"

package: extract ${OUTPUT_ZIP}

${OUTPUT_ZIP}: ${EXTRACT_DIR}/.extracted
	@echo "Creating PlatformIO package..."
	@mkdir -p ${PACKAGE_DIR}
	@echo "  Copying toolchain files..."
	@# Find the actual toolchain root (may be nested)
	@if [ -d "${EXTRACT_DIR}/powerpc-eabivle-4_9" ]; then \
		TOOLCHAIN_ROOT="${EXTRACT_DIR}/powerpc-eabivle-4_9"; \
	elif [ -f "${EXTRACT_DIR}/bin/powerpc-eabivle-gcc" ]; then \
		TOOLCHAIN_ROOT="${EXTRACT_DIR}"; \
	else \
		TOOLCHAIN_ROOT=$$(find "${EXTRACT_DIR}" -name "powerpc-eabivle-gcc" -type f | head -1 | xargs dirname | xargs dirname); \
	fi; \
	if [ -z "$$TOOLCHAIN_ROOT" ] || [ ! -d "$$TOOLCHAIN_ROOT" ]; then \
		echo "✗ Could not find toolchain root directory"; \
		exit 1; \
	fi; \
	echo "  Toolchain root: $$TOOLCHAIN_ROOT"; \
	cp -r "$$TOOLCHAIN_ROOT"/* "${PACKAGE_DIR}/" || \
		(echo "✗ Copy failed" && exit 1); \
	echo "  Creating zip archive..."; \
	cd ${BUILD_DIR} && ${ZIP} -r -q "${TOOLCHAIN_NAME}.zip" "${TOOLCHAIN_NAME}" || \
		(echo "✗ Zip creation failed" && exit 1)
	@echo "✓ Package created: ${OUTPUT_ZIP}"

update-sha256: ${OUTPUT_ZIP}
	@echo "Calculating SHA256 checksum..."
	@if command -v sha256sum >/dev/null 2>&1; then \
		SHA256=$$(sha256sum "${OUTPUT_ZIP}" | cut -d' ' -f1); \
	elif command -v shasum >/dev/null 2>&1; then \
		SHA256=$$(shasum -a 256 "${OUTPUT_ZIP}" | cut -d' ' -f1); \
	else \
		echo "✗ No SHA256 calculator found (sha256sum or shasum)"; \
		exit 1; \
	fi; \
	echo "  SHA256: $${SHA256}"; \
	echo "$${SHA256}" > "${OUTPUT_SHA256}"; \
	echo "  Updating package.json..."; \
	${PYTHON} -c " \
		import json, sys; \
		f = open('${PACKAGE_JSON}', 'r'); \
		data = json.load(f); \
		f.close(); \
		data['sha256']['linux_x86_64'] = '$${SHA256}'; \
		f = open('${PACKAGE_JSON}', 'w'); \
		json.dump(data, f, indent=2); \
		f.write('\n'); \
		f.close(); \
	"
	@echo "✓ SHA256 updated in package.json"

verify: ${OUTPUT_ZIP}
	@echo "Verifying package..."
	@if [ ! -f "${OUTPUT_ZIP}" ]; then \
		echo "✗ Package zip not found"; \
		exit 1; \
	fi
	@echo "  Checking zip integrity..."
	@${UNZIP} -t -q "${OUTPUT_ZIP}" || (echo "✗ Zip file is corrupted" && exit 1)
	@echo "  Checking for required binaries..."
	@if ${UNZIP} -l "${OUTPUT_ZIP}" | grep -q "powerpc-eabivle-gcc"; then \
		echo "✓ GCC compiler found"; \
	else \
		echo "✗ GCC compiler not found"; \
		exit 1; \
	fi
	@if ${UNZIP} -l "${OUTPUT_ZIP}" | grep -q "powerpc-eabivle-g\+\+"; then \
		echo "✓ G++ compiler found"; \
	else \
		echo "✗ G++ compiler not found"; \
		exit 1; \
	fi
	@if ${UNZIP} -l "${OUTPUT_ZIP}" | grep -q "powerpc-eabivle-ar"; then \
		echo "✓ AR archiver found"; \
	else \
		echo "✗ AR archiver not found"; \
		exit 1; \
	fi
	@if ${UNZIP} -l "${OUTPUT_ZIP}" | grep -q "powerpc-eabivle-objcopy"; then \
		echo "✓ OBJCOPY found"; \
	else \
		echo "✗ OBJCOPY not found"; \
		exit 1; \
	fi
	@if [ -f "${OUTPUT_SHA256}" ]; then \
		echo "✓ SHA256 checksum file found"; \
	else \
		echo "⚠ SHA256 checksum file not found (run 'make update-sha256')"; \
	fi
	@echo "✓ Package verification passed"

clean:
	@echo "Cleaning build artifacts..."
	@rm -rf ${BUILD_DIR}
	@echo "✓ Clean complete"

rebuild: clean all

test: verify
	@echo "Testing package..."
	@echo "  Package size: $$(du -h ${OUTPUT_ZIP} | cut -f1)"
	@echo "  File count: $$(${UNZIP} -l ${OUTPUT_ZIP} | tail -1 | awk '{print $$2}')"
	@echo "✓ All tests passed"

