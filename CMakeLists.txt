# Minimum required version of CMake
cmake_minimum_required(VERSION 3.16)

# Project name and version
project(SynergyStudio VERSION 1.0 LANGUAGES CXX)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF) # Prefer explicit standard features

# We need to find Qt
find_package(Qt6 REQUIRED COMPONENTS 
    Core
    Gui  # Needed by client
    Widgets # Needed by client
    Network  # needed by client and server
    Test # needed for unit tests (if with Qt Test -> TBD)
)

find_package(OpenSSL REQUIRED)

# --- Qt Automatic Features ---
# Enable AUTOMOC, AUTOUIC, AUTORCC for convenience
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Adding GoogleTest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
  DOWNLOAD_EXTRACT_TIMESTAMP true
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

include(CTest)
include(GoogleTest)

# --- Compiler Warnings (Optional but recommended) ---
if(MSVC)
  # Add warning level for MSVC
  add_compile_options(/W4)
else()
  # Add common warnings for GCC/Clang
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# --- Add Subdirectories ---
# Add common first as client and server depend on it
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)

# This needs to be enabled at the top level if tests are defined in subdirs
enable_testing()