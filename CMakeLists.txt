cmake_minimum_required(VERSION 3.25)
project(Drift VERSION 0.1.0 LANGUAGES CXX)

# CEF 152 requires C++20.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(DRIFT_BUILD_BROWSER_APP "Build the current macOS CEF browser" ON)

# Pin a sane deployment target before CEF's cef_variables.cmake overrides it,
# so core libraries don't inherit the host OS version (e.g. macOS 26 on dev machines).
if(APPLE AND NOT DEFINED CMAKE_OSX_DEPLOYMENT_TARGET)
  set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

include(CTest)
add_subdirectory(core/browser-state)
add_subdirectory(core/bookmarks)
add_subdirectory(core/history)
add_subdirectory(core/storage)
add_subdirectory(core/intelligence)
if(BUILD_TESTING)
  add_subdirectory(tests/core)
endif()

if(DRIFT_BUILD_BROWSER_APP)
  # CEF is a pinned external build dependency, not repository content.
  # macOS: run ./scripts/fetch-cef.sh
  # Windows: run .\scripts\fetch-cef-windows.ps1
  set(DRIFT_CEF_VERSION "152.0.6+g708dc14+chromium-152.0.7977.83")

  if(NOT DEFINED CEF_ROOT OR CEF_ROOT STREQUAL "")
    if(APPLE)
      if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64)$")
        set(DRIFT_CEF_PLATFORM "macosarm64")
      elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64)$")
        set(DRIFT_CEF_PLATFORM "macosx64")
      else()
        message(FATAL_ERROR "Unsupported macOS architecture: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
      endif()
    elseif(WIN32)
      set(DRIFT_CEF_PLATFORM "windows64")
    else()
      message(FATAL_ERROR "DRIFT_BUILD_BROWSER_APP is only supported on macOS and Windows.")
    endif()

    set(CEF_ROOT
      "${CMAKE_SOURCE_DIR}/.deps/cef_binary_${DRIFT_CEF_VERSION}_${DRIFT_CEF_PLATFORM}_minimal"
      CACHE PATH "Path to the extracted CEF binary distribution")
  endif()

  if(NOT EXISTS "${CEF_ROOT}/cmake/FindCEF.cmake")
    if(APPLE)
      set(_FETCH_HINT "Run ./scripts/fetch-cef.sh from the repository root, then configure again.")
    else()
      set(_FETCH_HINT "Run .\\scripts\\fetch-cef-windows.ps1 from the repository root, then configure again.")
    endif()
    message(FATAL_ERROR "CEF was not found at:\n  ${CEF_ROOT}\n${_FETCH_HINT}")
  endif()

  # CEF's cef_variables.cmake sets OBJCXX flags on macOS; enable the language
  # before find_package(CEF) so those variables resolve correctly.
  if(APPLE)
    enable_language(OBJCXX)
  endif()

  list(APPEND CMAKE_MODULE_PATH "${CEF_ROOT}/cmake")
  find_package(CEF REQUIRED)

  # The C++ wrapper ships as source inside the binary distribution and must be
  # compiled against the local toolchain; it is not a prebuilt static library.
  add_subdirectory("${CEF_LIBCEF_DLL_WRAPPER_PATH}" libcef_dll_wrapper)
  add_subdirectory(engine/cef)

  if(APPLE)
    add_subdirectory(src/browser)
  elseif(WIN32)
    add_subdirectory(apps/windows)
  endif()
endif()
