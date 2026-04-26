#!/usr/bin/env bash

set -euo pipefail


SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SERVER_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
PROJECT_DIR="$(cd "${SERVER_DIR}/.." && pwd)"
BUILD_DIR="${PROJECT_DIR}/build"

"${BUILD_DIR}/server/nebula_server"