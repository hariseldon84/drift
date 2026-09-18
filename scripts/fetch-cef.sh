#!/usr/bin/env bash

set -euo pipefail

readonly CEF_VERSION="152.0.6+g708dc14+chromium-152.0.7977.83"
readonly SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly REPO_ROOT="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
readonly DEPS_DIR="${REPO_ROOT}/.deps"

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "Drift's current browser shell builds on macOS only."
  echo "The future Windows shell will use its own CEF distribution and build path."
  exit 1
fi

case "$(uname -m)" in
  arm64)
    readonly CEF_PLATFORM="macosarm64"
    readonly CEF_SHA1="426836139b0ea7b7278aa0915cfae90eb460551f"
    ;;
  x86_64)
    readonly CEF_PLATFORM="macosx64"
    readonly CEF_SHA1="fed74cac2af95dec716000e9a35ebed8eb4f9a25"
    ;;
  *)
    echo "Unsupported Mac architecture: $(uname -m)"
    exit 1
    ;;
esac

readonly DISTRIBUTION="cef_binary_${CEF_VERSION}_${CEF_PLATFORM}_minimal"
readonly ARCHIVE="${DISTRIBUTION}.tar.bz2"
readonly ARCHIVE_PATH="${DEPS_DIR}/${ARCHIVE}"
readonly CEF_ROOT="${DEPS_DIR}/${DISTRIBUTION}"
readonly ENCODED_VERSION="${CEF_VERSION//+/%2B}"
readonly DOWNLOAD_URL="https://cef-builds.spotifycdn.com/cef_binary_${ENCODED_VERSION}_${CEF_PLATFORM}_minimal.tar.bz2"

if [[ -f "${CEF_ROOT}/cmake/FindCEF.cmake" && -d "${CEF_ROOT}/include" ]]; then
  echo "CEF is ready at ${CEF_ROOT}"
  exit 0
fi

command -v curl >/dev/null || { echo "curl is required."; exit 1; }
command -v shasum >/dev/null || { echo "shasum is required."; exit 1; }
command -v tar >/dev/null || { echo "tar is required."; exit 1; }

mkdir -p "${DEPS_DIR}"

if [[ ! -f "${ARCHIVE_PATH}" ]]; then
  echo "Downloading pinned CEF ${CEF_VERSION} for ${CEF_PLATFORM}..."
  curl --fail --location --retry 3 --continue-at - \
    --output "${ARCHIVE_PATH}.partial" \
    "${DOWNLOAD_URL}"
  mv "${ARCHIVE_PATH}.partial" "${ARCHIVE_PATH}"
fi

readonly ACTUAL_SHA1="$(shasum -a 1 "${ARCHIVE_PATH}" | awk '{print $1}')"
if [[ "${ACTUAL_SHA1}" != "${CEF_SHA1}" ]]; then
  echo "CEF archive checksum mismatch."
  echo "Expected: ${CEF_SHA1}"
  echo "Actual:   ${ACTUAL_SHA1}"
  echo "Delete ${ARCHIVE_PATH} and run this script again."
  exit 1
fi

echo "Extracting ${ARCHIVE}..."
tar -xjf "${ARCHIVE_PATH}" -C "${DEPS_DIR}"

if [[ ! -f "${CEF_ROOT}/cmake/FindCEF.cmake" ]]; then
  echo "CEF extraction did not create the expected directory: ${CEF_ROOT}"
  exit 1
fi

echo "CEF is ready at ${CEF_ROOT}"

