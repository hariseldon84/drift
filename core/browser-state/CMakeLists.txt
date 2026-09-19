add_library(drift_browser_state
  browser_state.cpp
  browser_state_codec.cpp
)

target_include_directories(drift_browser_state
  PUBLIC
    "${PROJECT_SOURCE_DIR}/core"
)

target_compile_features(drift_browser_state PUBLIC cxx_std_20)
set_target_properties(drift_browser_state PROPERTIES CXX_EXTENSIONS OFF)
if(APPLE)
  target_compile_options(drift_browser_state PRIVATE -mmacosx-version-min=12.0)
endif()

if(MSVC)
  target_compile_options(drift_browser_state PRIVATE /W4 /permissive-)
else()
  target_compile_options(drift_browser_state PRIVATE -Wall -Wextra -Wpedantic)
endif()
