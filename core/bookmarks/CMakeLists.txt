add_library(drift_bookmarks bookmark_state.cpp)

target_include_directories(drift_bookmarks PUBLIC "${PROJECT_SOURCE_DIR}/core")
target_compile_features(drift_bookmarks PUBLIC cxx_std_20)
set_target_properties(drift_bookmarks PROPERTIES CXX_EXTENSIONS OFF)
if(APPLE)
  target_compile_options(drift_bookmarks PRIVATE -mmacosx-version-min=12.0)
endif()

if(MSVC)
  target_compile_options(drift_bookmarks PRIVATE /W4 /permissive-)
else()
  target_compile_options(drift_bookmarks PRIVATE -Wall -Wextra -Wpedantic)
endif()
