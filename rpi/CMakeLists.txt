cmake_minimum_required(VERSION 3.6)

project(libcamera_project
    DESCRIPTION "Minimal example for capturing frames with libcamera"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unused-parameter -Wno-missing-field-initializers")

# Only enable libcamera if running on Raspberry Pi
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    file(READ "/proc/cpuinfo" CPUINFO)
    if(CPUINFO MATCHES "Raspberry Pi")
        set(WITH_LIBCAMERA ON)
    else()
        set(WITH_LIBCAMERA OFF)
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
else()
    message(STATUS "Not running on Raspberry Pi, libcamera will not be used.")
endif()

add_executable(camera_test camera/test.cpp camera/camera.cpp)
add_executable(main main.cpp camera/camera.cpp)

# Link to libcamera if available
if(WITH_LIBCAMERA)
    target_compile_definitions(camera_test PRIVATE USE_LIBCAMERA)
    target_link_libraries(camera_test PkgConfig::LIBCAMERA)
    target_compile_definitions(main PRIVATE USE_LIBCAMERA)
    target_link_libraries(main PkgConfig::LIBCAMERA)
endif()
