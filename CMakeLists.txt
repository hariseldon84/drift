cmake_minimum_required(VERSION 3.25)
project(Drift VERSION 0.1.0 LANGUAGES CXX OBJCXX)

# CEF 152 requires C++20.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# CEF's cef_variables.cmake overrides CMAKE_OSX_DEPLOYMENT_TARGET — no need to set it here.

# CEF is a pinned external build dependency, not repository content. The
# bootstrap script downloads the matching binary distribution into .deps/.
set(DRIFT_CEF_VERSION "152.0.6+g708dc14+chromium-152.0.7977.83")
if(NOT DEFINED CEF_ROOT OR CEF_ROOT STREQUAL "")
  if(NOT APPLE)
    message(FATAL_ERROR
      "The current Drift shell builds on macOS only. A Windows platform shell "
      "and Windows CEF bootstrap are tracked separately.")
  endif()

  if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
    set(DRIFT_CEF_PLATFORM "macosarm64")
  elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
    set(DRIFT_CEF_PLATFORM "macosx64")
  else()
    message(FATAL_ERROR
      "Unsupported macOS architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
  endif()

  set(CEF_ROOT
    "${CMAKE_SOURCE_DIR}/.deps/cef_binary_${DRIFT_CEF_VERSION}_${DRIFT_CEF_PLATFORM}_minimal"
    CACHE PATH "Path to the extracted CEF binary distribution")
endif()

if(NOT EXISTS "${CEF_ROOT}/cmake/FindCEF.cmake")
  message(FATAL_ERROR
    "CEF was not found at:\n  ${CEF_ROOT}\n"
    "Run ./scripts/fetch-cef.sh from the repository root, then configure again.")
endif()

list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
find_package(CEF REQUIRED)

# The C++ wrapper ships as source inside the binary distribution and must be
# compiled against your local toolchain — it is NOT a prebuilt static lib.
add_subdirectory("${CEF_LIBCEF_DLL_WRAPPER_PATH}" libcef_dll_wrapper)

add_subdirectory(src/browser)
