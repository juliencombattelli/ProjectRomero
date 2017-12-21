# Specify the minimum version for CMake
cmake_minimum_required(VERSION 3.6)

# Project configuration
project(acm-gateway)
set(EXECUTABLE_NAME acm-gateway)


# Find pkgconfig
find_package(PkgConfig REQUIRED)

# Find Glib2
pkg_check_modules(GLIB2 REQUIRED glib-2.0)
include_directories(${GLIB2_INCLUDE_DIRS})
list(APPEND LIBRARIES_LIST ${GLIB2_LIBRARIES})

# Find Bluez5
pkg_check_modules(BLUEZ REQUIRED bluez)
include_directories(${BLUEZ_INCLUDE_DIRS})
link_directories(${BLUEZ_LIBRARY_DIRS})
list(APPEND LIBRARIES_LIST ${BLUEZ_LIBRARIES})

# Find Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
list(APPEND LIBRARIES_LIST Threads::Threads)


# Set source and header directories
set(PROJECT_SOURCE_DIR src)
set(PROJECT_HEADER_DIR src)

# Find all header and source files
file(GLOB_RECURSE PROJECT_SOURCE_LIST ${PROJECT_HEADER_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.c ${PROJECT_HEADER_DIR}/*.hpp ${PROJECT_SOURCE_DIR}/*.cpp)

# Include the following directories
include_directories(${PROJECT_HEADER_DIR})
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/ble)


# Add the acm-gateway executable
add_executable(${EXECUTABLE_NAME} ${PROJECT_SOURCE_LIST})

# Link all needed libraries
target_link_libraries(${EXECUTABLE_NAME} ${LIBRARIES_LIST})
