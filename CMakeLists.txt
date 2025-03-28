cmake_minimum_required(VERSION 3.12)
project(QueryCAD VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCASCADE package
find_package(OpenCASCADE REQUIRED)

# Collect all source files automatically
file(GLOB_RECURSE QUERYCAD_SOURCES src/*.cpp)
file(GLOB_RECURSE QUERYCAD_HEADERS src/*.hpp)

# Create library
add_library(QueryCAD ${QUERYCAD_SOURCES} ${QUERYCAD_HEADERS})

# Set include directories
target_include_directories(QueryCAD
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
        ${OpenCASCADE_INCLUDE_DIR}
)

# Link OpenCASCADE libraries
target_link_libraries(QueryCAD
    PUBLIC
        ${OpenCASCADE_LIBRARIES}
        TKernel
        TKMath
        TKBRep
        TKG2d
        TKG3d
        TKGeomBase
        TKTopAlgo
        TKDESTEP 
        TKDEIGES
)
target_link_directories(QueryCAD
    PUBLIC
        ${OpenCASCADE_LIBRARY_DIR}
)

include(CTest)
include(GNUInstallDirs)

add_subdirectory(tests)