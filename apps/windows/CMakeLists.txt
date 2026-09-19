include("${CEF_ROOT}/cmake/cef_macros.cmake")

set(DRIFT_WIN_SRCS
  main.cpp
  drift_win_app.h
  drift_win_client.h
  drift_win_client.cpp
  drift_win_window.h
  drift_win_window.cpp
)

add_executable(Drift WIN32 ${DRIFT_WIN_SRCS})
SET_EXECUTABLE_TARGET_PROPERTIES(Drift)

target_include_directories(Drift PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${PROJECT_SOURCE_DIR}"
  "${CEF_ROOT}"
)
target_compile_features(Drift PRIVATE cxx_std_20)
set_target_properties(Drift PROPERTIES CXX_EXTENSIONS OFF)
target_compile_options(Drift PRIVATE /W4 /permissive-)

target_link_libraries(Drift PRIVATE
  libcef_dll_wrapper
  ${CEF_STANDARD_LIBS}
  drift_cef_engine
  drift_browser_state
  drift_storage
)

# Copy CEF runtime binaries and resources next to Drift.exe
set(CEF_TARGET_OUT_DIR "$<TARGET_FILE_DIR:Drift>")
COPY_FILES(Drift "${CEF_BINARY_FILES}" "${CEF_BINARY_DIR}" "${CEF_TARGET_OUT_DIR}")
COPY_FILES(Drift "${CEF_RESOURCE_FILES}" "${CEF_RESOURCE_DIR}" "${CEF_TARGET_OUT_DIR}")
