# Makefile for PlatformIO Package Automation

PYTHON := python3
SCRIPT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
S32DS_ROOT := $(abspath $(SCRIPT_DIR)/..)
OUTPUT_DIR := $(S32DS_ROOT)/platformio_package
PACKAGE_NAME := tool-pegdbserver-power

.PHONY: help clean build rebuild package verify test

help:
	@echo "PlatformIO Package Builder"
	@echo ""
	@echo "Targets:"
	@echo "  build      - Build the PlatformIO package"
	@echo "  package    - Build and create zip archive"
	@echo "  verify     - Verify package contents"
	@echo "  clean      - Remove build artifacts"
	@echo "  rebuild    - Clean and rebuild"
	@echo "  test       - Test the built package"
	@echo ""
	@echo "Variables:"
	@echo "  S32DS_ROOT=$(S32DS_ROOT)"
	@echo "  OUTPUT_DIR=$(OUTPUT_DIR)"

build:
	@echo "Building PlatformIO package..."
	${PYTHON} ${SCRIPT_DIR}/build_pio_package.py \
		--s32ds-root ${S32DS_ROOT} \
		--output-dir ${OUTPUT_DIR} \
		--no-archive

package: clean
	@echo "Building PlatformIO package with archive..."
	${PYTHON} ${SCRIPT_DIR}/build_pio_package.py \
		--s32ds-root ${S32DS_ROOT} \
		--output-dir ${OUTPUT_DIR}

verify:
	@echo "Verifying package..."
	@if [ -d "$(OUTPUT_DIR)/$(PACKAGE_NAME)" ]; then \
		echo "✓ Package directory exists"; \
		${PYTHON} ${SCRIPT_DIR}/build_pio_package.py \
			--s32ds-root ${S32DS_ROOT} \
			--output-dir ${OUTPUT_DIR} \
			--no-archive; \
	else \
		echo "✗ Package not found. Run 'make build' first."; \
		exit 1; \
	fi

clean:
	@echo "Cleaning build artifacts..."
	rm -rf ${OUTPUT_DIR}
	@echo "✓ Clean complete"

rebuild: clean package

test:
	@echo "Testing package..."
	@if [ ! -f "$(OUTPUT_DIR)/$(PACKAGE_NAME).zip" ]; then \
		echo "✗ Archive not found. Run 'make package' first."; \
		exit 1; \
	fi
	@echo "Testing binary..."
	@if unzip -l $(OUTPUT_DIR)/$(PACKAGE_NAME).zip | grep -q "pegdbserver_power_console"; then \
		echo "✓ Binary found in archive"; \
	else \
		echo "✗ Binary not found in archive"; \
		exit 1; \
	fi
	@echo "Testing device files..."
	@if unzip -l $(OUTPUT_DIR)/$(PACKAGE_NAME).zip | grep -q "\.add"; then \
		echo "✓ Device files found"; \
	else \
		echo "✗ Device files not found"; \
		exit 1; \
	fi
	@echo "Testing flash algorithms..."
	@if unzip -l $(OUTPUT_DIR)/$(PACKAGE_NAME).zip | grep -q "\.pcp"; then \
		echo "✓ Flash algorithms found"; \
	else \
		echo "✗ Flash algorithms not found"; \
		exit 1; \
	fi
	@echo "✓ All tests passed"

