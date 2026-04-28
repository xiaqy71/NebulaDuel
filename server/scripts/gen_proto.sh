#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PROTO_DIR="${ROOT_DIR}/proto"
OUT_DIR="${ROOT_DIR}/generated"

mkdir -p "${OUT_DIR}"

protoc \
	-I "${PROTO_DIR}" \
	--cpp_out="${OUT_DIR}" \
	"${PROTO_DIR}/common.proto" \
	"${PROTO_DIR}/login.proto"
