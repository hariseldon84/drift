add_library(drift_history history_state.cpp)

target_include_directories(drift_history PUBLIC "${PROJECT_SOURCE_DIR}/core")
target_compile_features(drift_history PUBLIC cxx_std_20)
set_target_properties(drift_history PROPERTIES CXX_EXTENSIONS OFF)
if(APPLE)
  target_compile_options(drift_history PRIVATE -mmacosx-version-min=12.0)
endif()

if(MSVC)
  target_compile_options(drift_history PRIVATE /W4 /permissive-)
else()
  target_compile_options(drift_history PRIVATE -Wall -Wextra -Wpedantic)
endif()
