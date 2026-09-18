add_executable(drift_browser_state_tests
  browser_state_test.cpp
)

target_link_libraries(drift_browser_state_tests PRIVATE drift_browser_state)
target_compile_features(drift_browser_state_tests PRIVATE cxx_std_20)
set_target_properties(drift_browser_state_tests PROPERTIES CXX_EXTENSIONS OFF)

if(MSVC)
  target_compile_options(drift_browser_state_tests PRIVATE /W4 /permissive-)
else()
  target_compile_options(drift_browser_state_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME drift.browser_state COMMAND drift_browser_state_tests)
