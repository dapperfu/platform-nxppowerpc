#!/bin/bash
#
# Docker wrapper script for PowerPC toolchain
# This script can be used to wrap toolchain commands for Docker execution
#
# Usage:
#   ./docker-wrapper.sh gcc -o output.o input.c
#   ./docker-wrapper.sh ar rc lib.a file1.o file2.o
#

set -e

DOCKER_IMAGE="s32ds-power-v1-2:latest"
TOOLCHAIN_PREFIX="powerpc-eabivle-"

# Get the tool name (first argument)
TOOL_NAME="$1"
shift

# Get project directory (assuming we're in the project root)
PROJECT_DIR="${PROJECT_DIR:-$(pwd)}"
PROJECT_DIR="$(cd "$PROJECT_DIR" && pwd)"

# Build Docker command
DOCKER_CMD=(
    docker run --rm
    -v "$PROJECT_DIR:$PROJECT_DIR"
    -w "$PROJECT_DIR"
    "$DOCKER_IMAGE"
    "${TOOLCHAIN_PREFIX}${TOOL_NAME}"
    "$@"
)

# Execute in Docker
exec "${DOCKER_CMD[@]}"

