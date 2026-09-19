add_library(drift_intelligence STATIC product_extractor.cpp)

target_include_directories(drift_intelligence
  PUBLIC  "${PROJECT_SOURCE_DIR}/core"
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}"  # json.hpp lives here
)

target_compile_features(drift_intelligence PUBLIC cxx_std_20)
set_target_properties(drift_intelligence PROPERTIES CXX_EXTENSIONS OFF)

if(APPLE)
  target_compile_options(drift_intelligence PRIVATE -mmacosx-version-min=12.0)
endif()

if(MSVC)
  target_compile_options(drift_intelligence PRIVATE /W4 /permissive-)
else()
  target_compile_options(drift_intelligence PRIVATE -Wall -Wextra -Wpedantic)
endif()
