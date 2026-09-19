find_package(SQLite3 REQUIRED)

add_library(drift_storage
  sqlite_bookmark_repo.cpp
  sqlite_history_repo.cpp
)

target_include_directories(drift_storage PUBLIC "${PROJECT_SOURCE_DIR}/core")
target_link_libraries(drift_storage PUBLIC drift_bookmarks drift_history SQLite3::SQLite3)
target_compile_features(drift_storage PUBLIC cxx_std_20)
set_target_properties(drift_storage PROPERTIES CXX_EXTENSIONS OFF)

if(APPLE)
  target_compile_options(drift_storage PRIVATE -mmacosx-version-min=12.0)
endif()

if(MSVC)
  target_compile_options(drift_storage PRIVATE /W4 /permissive-)
else()
  target_compile_options(drift_storage PRIVATE -Wall -Wextra -Wpedantic)
endif()
