# Define the server executable
add_executable(SynergyStudioServer)

# List server-specific source files
target_sources(SynergyStudioServer PRIVATE
    src/main.cpp
    src/SslServer.cpp
    include/SslServer.h
    # src/ClientConnection.cpp
    # src/ClientConnection.h
    # src/Session.cpp
    # src/Session.h
    # src/SessionManager.cpp
    # src/SessionManager.h
    # src/WorkspaceManager.cpp
    # src/WorkspaceManager.h
    # src/DockerExecutor.cpp
    # src/DockerExecutor.h
    # ... add all other server source/header files
)

# Link necessary libraries
target_link_libraries(SynergyStudioServer PRIVATE
    Qt6::Core
    Qt6::Network
    synergy_protocol # Link against the common library
    OpenSSL::SSL
    OpenSSL::Crypto
)

if(BUILD_TESTING) # Standard CMake variable check
    add_executable(server_gtests test/gtest_server_main.cpp)
    target_link_libraries(server_gtests PRIVATE
        # Link SUT (if server code is in a library) or specific components
        synergy_protocol # Definitely need common
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        Qt6::Core 
    )
    # Discover tests AND assign a label
    gtest_discover_tests(server_gtests LABELS "server_test")
endif()


# --- Testing (Example using Qt Test) ---
# find_package(GTest REQUIRED) # If using Google Test

# Example: Define a test executable
# qt_add_executable(server_tests src/tests/main_test.cpp src/tests/test_session.cpp)
# target_link_libraries(server_tests PRIVATE SynergyStudioServer synergy_protocol Qt6::Core Qt6::Test) # Link SUT, common, Qt Test
# qt_add_test(server_tests server_tests) # Register with CTest

# Example: Using CTest directly without qt_add_test
# add_executable(server_gtests src/tests/gtest_main.cpp src/tests/gtest_session.cpp)
# target_link_libraries(server_gtests PRIVATE SynergyStudioServer synergy_protocol GTest::gtest GTest::gmock GTest::gtest_main)
# include(GoogleTest)
# gtest_discover_tests(server_gtests)