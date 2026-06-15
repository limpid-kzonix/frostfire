#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT_DIR}/.tmp-host-tests"
mkdir -p "${BUILD_DIR}"

run_test() {
  local source_file="$1"
  local binary_name
  binary_name="${BUILD_DIR}/$(basename "${source_file%.cpp}")"

  echo "Compiling ${source_file}"
  g++ -std=c++17 -I"${ROOT_DIR}/include" "${ROOT_DIR}/${source_file}" -o "${binary_name}"

  echo "Running ${binary_name}"
  "${binary_name}"
}

run_test "test/test_relay_controller/test_pulse_policy.cpp"
run_test "test/test_config/test_config_defaults.cpp"
run_test "test/test_auth/test_auth_policy.cpp"

echo "Host tests passed."
