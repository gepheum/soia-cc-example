cmake_minimum_required(VERSION 3.20)
project(cc-example CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTING "Build tests" ON)

# Include FetchContent for dependencies
include(FetchContent)

# ============================================================================
# Dependencies
# ============================================================================

# Abseil
FetchContent_Declare(
  absl
  GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
  GIT_TAG        20250814.0
)
set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL OFF)

# cpp-httplib
FetchContent_Declare(
  httplib
  GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
  GIT_TAG        v0.22.0
)
set(HTTPLIB_REQUIRE_OPENSSL OFF)

# GoogleTest (if building tests)
if(BUILD_TESTING)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
  )
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

# skir-client
FetchContent_Declare(
  skir-client
  GIT_REPOSITORY https://github.com/gepheum/skir-cc-gen.git
  GIT_TAG        44070ac519ae13bb85ae8f68498a2b633b48fac5
  SOURCE_SUBDIR  client
)

# Make all content available
FetchContent_MakeAvailable(absl httplib skir-client)
if(BUILD_TESTING)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  include(GoogleTest)
endif()

# ============================================================================
# Library: skirout
# ============================================================================

file(GLOB SKIROUT_SOURCES "skirout/*.cc")
file(GLOB SKIROUT_HEADERS "skirout/*.h")
list(FILTER SKIROUT_HEADERS EXCLUDE REGEX ".*\\.testing\\.h$")

add_library(skirout ${SKIROUT_SOURCES})
target_include_directories(skirout PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(skirout PUBLIC skir)

# ============================================================================
# Library: skirout.testing (test-only library)
# ============================================================================

if(BUILD_TESTING)
  file(GLOB SKIROUT_TESTING_HEADERS "skirout/*.testing.h")
  
  add_library(skirout_testing INTERFACE)
  target_include_directories(skirout_testing INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(skirout_testing INTERFACE skir_testing)
endif()

# ============================================================================
# Library: string_capitalizer
# ============================================================================

add_library(string_capitalizer INTERFACE)
target_include_directories(string_capitalizer INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(string_capitalizer INTERFACE
  absl::strings
  absl::optional
  skir
)

# ============================================================================
# Executable: example
# ============================================================================

add_executable(example example.cc)
target_link_libraries(example PRIVATE
  skirout
  string_capitalizer
  absl::flat_hash_set
  absl::statusor
  absl::strings
  absl::time
  absl::optional
  skir
)

# ============================================================================
# Executable: service_start
# ============================================================================

add_executable(service_start service_start.cc)
target_link_libraries(service_start PRIVATE
  skirout
  absl::flat_hash_map
  absl::status
  absl::statusor
  httplib::httplib
  skir
)

# ============================================================================
# Executable: service_client
# ============================================================================

add_executable(service_client service_client.cc)
target_link_libraries(service_client PRIVATE
  skirout
  absl::statusor
  absl::optional
  httplib::httplib
  skir
)

# ============================================================================
# Tests
# ============================================================================

if(BUILD_TESTING)
  add_executable(example_test example.test.cc)
  target_link_libraries(example_test PRIVATE
    skirout
    skirout_testing
    absl::time
    skir
    skir_testing
    GTest::gtest
    GTest::gtest_main
  )
  
  gtest_discover_tests(example_test)
endif()
