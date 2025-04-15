# Define the shared library for protocol and common utilities
# add_library(synergy_protocol STATIC) 
add_library(synergy_protocol INTERFACE) # Or INTERFACE if header-only

# List public headers (adjust path if necessary)
# target_include_directories(synergy_protocol PUBLIC
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#     $<INSTALL_INTERFACE:include> # For potential installation
# )
target_include_directories(synergy_protocol INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include> # For potential installation
)

# List source files (if any)
target_sources(synergy_protocol INTERFACE 
    ./include/synergy_protocol/protocol.h)

# Link dependencies needed by the common library itself
# Example: If using QJsonObject for serialization
# target_link_libraries(synergy_protocol PUBLIC Qt6::Core)
target_link_libraries(synergy_protocol INTERFACE Qt6::Core)

# Ensure common library uses the correct C++ standard
target_compile_features(synergy_protocol INTERFACE cxx_std_20)

# An INTERFACE library in CMake is essentially a collection of usage requirements. It's used when you have a library that is entirely defined by its header files (a "header-only" library) or when you want to group usage requirements together without creating an actual compiled artifact (.a or .so file).

# --- Testing for the common library ---
if(BUILD_TESTING) # Check if testing is enabled (usually true by default)
    # Define the test executable for the common library
    add_executable(common_gtests
        test/gtest_common_main.cpp 
    )

    # Link the test executable against necessary libraries:
    target_link_libraries(common_gtests PRIVATE
        # synergy_protocol  # <-- Link against the library being tested (needed if STATIC)
        GTest::gtest      # Google Test framework
        GTest::gmock      # Google Mock framework
        GTest::gtest_main # Google Test main
        Qt6::Core         # Any Qt modules used by common code OR the tests themselves
    )

    # Discover tests and assign a label
    gtest_discover_tests(common_gtests LABELS "common_test") # Assign the label
endif()