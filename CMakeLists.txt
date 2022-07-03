cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 20)

# Shimakaze Project Configuration
project(shimakaze)
file(GLOB_RECURSE SOURCE_FILES src/*.cc)
add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# Ensure /MT is used for the build.
set_property(TARGET ${PROJECT_NAME}
    PROPERTY "MultiThreaded$<$<CONFIG:Release>:Release>")

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT")

# Statically set the size of a void pointer to be the same for a 32-bit compilation.
set(CMAKE_SIZEOF_VOID_P 4)

# Ensure 32-bit on clang
if (${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -target i386-pc-windows-msvc")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -target i386-pc-windows-msvc")
  add_definitions("--target=i386-pc-windows-msvc")
endif()


### LIBRARY CONFIGURATIONS
# Create include directories for every library.
target_include_directories(${PROJECT_NAME} PRIVATE
    incl/v8
    incl/minhook/include
    incl/gd.h/include
    incl/gd.h
)

# Handle the v8 library.
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/v8.cmake)

# Add minhook & cocos-headers as a sub directory.
add_subdirectory(incl/minhook)
add_subdirectory(incl/cocos-headers)

# Link every relevant library.
target_link_libraries(${PROJECT_NAME}
    minhook
    cocos2d
    v8

    # Windows libraries required by v8
    winmm
    dbghelp
)