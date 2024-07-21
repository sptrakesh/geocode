cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

# For INTERPROCEDURAL_OPTIMIZATION
if (POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
endif()

if (POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)
endif()

file(STRINGS "docker/env.sh" ver REGEX "VERSION=([0-9]*)")
string(REGEX REPLACE "VERSION='" "" project_version ${ver})
string(REGEX REPLACE "'" "" project_version ${project_version})
message("Project version: ${project_version}")

project(geocode LANGUAGES CXX VERSION ${project_version})

# Enable C++23 features
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# Find includes in the build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
  execute_process(
    COMMAND brew --prefix OpenSSL
    RESULT_VARIABLE BREW_OPENSSL
    OUTPUT_VARIABLE BREW_OPENSSL_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (BREW_OPENSSL EQUAL 0 AND EXISTS "${BREW_OPENSSL_PREFIX}")
    message(STATUS "Found OpenSSL keg installed by Homebrew at ${BREW_OPENSSL_PREFIX}")
    set(OPENSSL_ROOT_DIR "${BREW_OPENSSL_PREFIX}/")
    set(OPENSSL_INCLUDE_DIR "${BREW_OPENSSL_PREFIX}/include")
    set(OPENSSL_LIBRARIES "${BREW_OPENSSL_PREFIX}/lib")
    set(OPENSSL_CRYPTO_LIBRARY "${BREW_OPENSSL_PREFIX}/lib/libcrypto.dylib")
  endif()

  execute_process(
    COMMAND brew --prefix jemalloc
    RESULT_VARIABLE BREW_JEMALLOC
    OUTPUT_VARIABLE BREW_JEMALLOC_PREFIX
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if (BREW_JEMALLOC EQUAL 0 AND EXISTS "${BREW_JEMALLOC_PREFIX}")
    message(STATUS "Found OpenSSL keg installed by Homebrew at ${BREW_JEMALLOC_PREFIX}")
    set(JEMALLOC_ROOT_DIR "${BREW_JEMALLOC_PREFIX}/")
    set(JEMALLOC_LIBRARIES "${BREW_JEMALLOC_PREFIX}/lib")
    set(JEMALLOC_LIBRARY "${BREW_JEMALLOC_PREFIX}/lib/libjemalloc.dylib")
  endif()
endif()

find_package(OpenSSL REQUIRED)
if (OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  set(OPENSSL_USE_STATIC_LIBS TRUE)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror")
endif()

if (UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
endif(UNIX AND NOT APPLE)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_MULTITHREADED ON)

find_package(Boost REQUIRED COMPONENTS json)
find_package(cpr REQUIRED)

if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

if (APPLE)
  set(CMAKE_MACOSX_RPATH 1)
  include_directories(/usr/local/cpr/include)
endif()
if (UNIX AND NOT APPLE)
  include_directories(
    /opt/local/include
  )
  link_directories(
    /opt/local/lib
  )
endif()

IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX /opt/spt)
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

add_subdirectory(src/log)
add_subdirectory(src/lib)
#add_subdirectory(src/api)
#add_subdirectory(src/cli)
#add_subdirectory(src/shell)
#add_subdirectory(src/service)

if (BUILD_TESTING)
  enable_testing()
  add_subdirectory(test/unit)
  #add_subdirectory(test/integration)
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "GeocodeConfigVersion.cmake"
  VERSION ${geocode_VERSION}
  COMPATIBILITY AnyNewerVersion)

install(FILES "GeocodeConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/GeocodeConfigVersion.cmake" DESTINATION lib/cmake/Geocode)

