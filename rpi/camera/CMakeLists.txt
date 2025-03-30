cmake_minimum_required(VERSION 3.6)

project(libcamera_project
    DESCRIPTION "Minimal example for capturing frames with libcamera"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unused-parameter -Wno-missing-field-initializers")

# Find pkg-config and libcamera
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED IMPORTED_TARGET libcamera)

message(STATUS "Libcamera library found:")
message(STATUS "    version: ${LIBCAMERA_VERSION}")
message(STATUS "    libraries: ${LIBCAMERA_LINK_LIBRARIES}")
message(STATUS "    include path: ${LIBCAMERA_INCLUDE_DIRS}")

# Include paths
include_directories(${LIBCAMERA_INCLUDE_DIRS})

# Add the executable
add_executable(camera_capture test.cpp camera.cpp)

# Link to libcamera
target_link_libraries(camera_capture PkgConfig::LIBCAMERA)

