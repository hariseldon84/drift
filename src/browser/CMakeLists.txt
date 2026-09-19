include("${CEF_ROOT}/cmake/cef_macros.cmake")

set(DRIFT_ICON "${PROJECT_SOURCE_DIR}/assets/drift.icns")
set_source_files_properties("${DRIFT_ICON}" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

set(DRIFT_SRCS
  main.mm
  drift_app.h
  drift_app.cpp
  drift_client.h
  drift_client.cpp
  browser_window.h
  browser_window.mm
  "${DRIFT_ICON}"
)

# ── Helper apps ──────────────────────────────────────────────────────────────
# CEF spawns separate helper bundles for each subprocess type on macOS.
# CEF_HELPER_APP_SUFFIXES format: "<name suffix>:<target suffix>:<plist suffix>"
foreach(SUFFIX_ENTRY ${CEF_HELPER_APP_SUFFIXES})
  string(REPLACE ":" ";" SUFFIX_LIST "${SUFFIX_ENTRY}")
  list(GET SUFFIX_LIST 0 NAME_SUFFIX)
  list(GET SUFFIX_LIST 1 TARGET_SUFFIX)

  set(HELPER_TARGET "DriftHelper${TARGET_SUFFIX}")
  set(HELPER_NAME   "Drift Helper${NAME_SUFFIX}")

  add_executable(${HELPER_TARGET} MACOSX_BUNDLE ${DRIFT_SRCS})
  set_target_properties(${HELPER_TARGET} PROPERTIES
    OUTPUT_NAME "${HELPER_NAME}"
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/helper-Info.plist.in"
  )
  SET_EXECUTABLE_TARGET_PROPERTIES(${HELPER_TARGET})
  target_include_directories(${HELPER_TARGET} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
  # Do NOT link the framework directly — CEF 152 macOS uses dylib mode.
  # CefScopedLibraryLoader dlopen()s the framework at runtime from the correct path.
  target_link_libraries(${HELPER_TARGET} PRIVATE
    libcef_dll_wrapper
    ${CEF_STANDARD_LIBS}
    drift_cef_engine
    drift_bookmarks
    drift_history
    drift_browser_state
    drift_storage
    drift_intelligence
  )
endforeach()

# ── Main browser app ─────────────────────────────────────────────────────────
add_executable(Drift MACOSX_BUNDLE ${DRIFT_SRCS})
set_target_properties(Drift PROPERTIES
  OUTPUT_NAME "Drift"
  MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
)
SET_EXECUTABLE_TARGET_PROPERTIES(Drift)
target_include_directories(Drift PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(Drift PRIVATE
  libcef_dll_wrapper
  ${CEF_STANDARD_LIBS}
  drift_cef_engine
  drift_bookmarks
  drift_history
  drift_browser_state
  drift_storage
  drift_intelligence
)

# Copy CEF framework into Drift.app/Contents/Frameworks/ after build
COPY_MAC_FRAMEWORK(Drift "${CEF_BINARY_DIR_RELEASE}" "$<TARGET_BUNDLE_DIR:Drift>")

# Copy each helper bundle into Drift.app/Contents/Frameworks/
# Then ad-hoc sign each helper with JIT entitlements (required for V8 on Apple Silicon).
# Replace "-" with a real Developer ID before any beta/release build.
set(DEV_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/dev-entitlements.plist")

foreach(SUFFIX_ENTRY ${CEF_HELPER_APP_SUFFIXES})
  string(REPLACE ":" ";" SUFFIX_LIST "${SUFFIX_ENTRY}")
  list(GET SUFFIX_LIST 0 NAME_SUFFIX)
  list(GET SUFFIX_LIST 1 TARGET_SUFFIX)

  set(HELPER_TARGET "DriftHelper${TARGET_SUFFIX}")
  set(HELPER_NAME   "Drift Helper${NAME_SUFFIX}")

  add_custom_command(TARGET Drift POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "$<TARGET_BUNDLE_DIR:${HELPER_TARGET}>"
            "$<TARGET_BUNDLE_DIR:Drift>/Contents/Frameworks/${HELPER_NAME}.app"
    COMMAND codesign --force --sign "-" --entitlements "${DEV_ENTITLEMENTS}"
            "$<TARGET_BUNDLE_DIR:Drift>/Contents/Frameworks/${HELPER_NAME}.app"
    VERBATIM
  )
endforeach()

# Sign the main app last (must come after all helpers are in place)
add_custom_command(TARGET Drift POST_BUILD
  COMMAND codesign --force --sign "-" --entitlements "${DEV_ENTITLEMENTS}"
          "$<TARGET_BUNDLE_DIR:Drift>"
  VERBATIM
)
