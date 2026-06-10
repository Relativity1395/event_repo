cmake_minimum_required(VERSION 3.16)
project(dvxplore_stream C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall -Wextra")

# Disable unnecessary libcaer build targets
set(LIBCAER_BUILD_PYTHON   OFF CACHE BOOL "" FORCE)
set(LIBCAER_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LIBCAER_BUILD_DOCS     OFF CACHE BOOL "" FORCE)

# Build libcaer from submodule
add_subdirectory(external/libcaer)

# Your application
add_executable(dvxplore_simple
    src/dvxplore_simple.c
)

target_include_directories(dvxplore_simple
    PRIVATE
        src/
        external/libcaer/include
)

target_link_libraries(dvxplore_simple
    PRIVATE
        caer
        pthread
        rt
)