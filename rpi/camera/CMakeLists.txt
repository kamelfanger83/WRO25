cmake_minimum_required(VERSION 3.6)

project(libcamera_project
    DESCRIPTION "Minimal example for capturing frames with libcamera"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unused-parameter -Wno-missing-field-initializers")

# Check platform
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if(DEFINED ENV{WSL_DISTRO_NAME})
        set(WITH_LIBCAMERA OFF)
    else()
        set(WITH_LIBCAMERA ON)
    endif()
else()
    set(WITH_LIBCAMERA OFF)
endif()

if(WITH_LIBCAMERA)
    # Find pkg-config and libcamera
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBCAMERA REQUIRED IMPORTED_TARGET libcamera)

    message(STATUS "Libcamera library found:")
    message(STATUS "    version: ${LIBCAMERA_VERSION}")
    message(STATUS "    libraries: ${LIBCAMERA_LINK_LIBRARIES}")
    message(STATUS "    include path: ${LIBCAMERA_INCLUDE_DIRS}")

    # Include paths
    include_directories(${LIBCAMERA_INCLUDE_DIRS})
endif()

# Add the executable
add_executable(camera_capture test.cpp camera.cpp)

# Link to libcamera if available
if(WITH_LIBCAMERA)
    target_link_libraries(camera_capture PkgConfig::LIBCAMERA)
else()
    target_compile_definitions(camera_capture PRIVATE NO_LIBCAMERA)
endif()
