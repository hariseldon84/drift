add_library(drift_cef_engine cef_engine_adapter.cpp)

target_include_directories(drift_cef_engine
  PUBLIC  "${PROJECT_SOURCE_DIR}"
  PRIVATE "${CEF_ROOT}"
)

target_compile_features(drift_cef_engine PUBLIC cxx_std_20)
set_target_properties(drift_cef_engine PROPERTIES CXX_EXTENSIONS OFF)

if(APPLE)
  target_compile_options(drift_cef_engine PRIVATE -mmacosx-version-min=12.0)
endif()

if(MSVC)
  target_compile_options(drift_cef_engine PRIVATE /W4 /permissive-)
else()
  target_compile_options(drift_cef_engine PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_link_libraries(drift_cef_engine PRIVATE libcef_dll_wrapper ${CEF_STANDARD_LIBS})
