# Define the client executable
# Use WIN32 for Windows GUI applications to avoid console window
# if(WIN32)
#     add_executable(SynergyStudioClient WIN32)
# else()
#     add_executable(SynergyStudioClient)
# endif()

add_executable(SynergyStudioClient)

# List client-specific source files, UI forms, resource files
target_sources(SynergyStudioClient PRIVATE
    src/main.cpp
    # src/MainWindow.cpp
    # src/MainWindow.h
    # src/MainWindow.ui
    src/SslClient.cpp
    include/SslClient.h
    # src/FileTreeView.cpp
    # src/FileTreeView.h
    # src/EditorView.cpp
    # src/EditorView.h
    # src/CanvasWidget.cpp
    # src/CanvasWidget.h
    # src/OutputView.cpp
    # src/OutputView.h
    # resources/client.qrc # Example resource file
    # ... add all other client source/header/ui/qrc files
)

# Link necessary libraries
target_link_libraries(SynergyStudioClient PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    synergy_protocol # Link against the common library
    OpenSSL::SSL
    OpenSSL::Crypto
)

# --- Add MinGW specific libraries ONLY on Windows for GUI ---
if(WIN32 AND CMAKE_CXX_COMPILER_ID MATCHES GNU)
    # Explicitly link the mingw runtime library FIRST
    # Also add user32 and gdi32 often needed by GUI
    target_link_libraries(SynergyStudioClient PRIVATE mingw32 user32 gdi32)

    # Additionally, try linking against Qt6::EntryPoint if it exists
    # This handles the main/WinMain wrapping. CMake might do this via
    # Qt6::Widgets, but being explicit might help resolve link order issues.
    if(TARGET Qt6::EntryPoint)
         target_link_libraries(SynergyStudioClient PRIVATE Qt6::EntryPoint)
    endif()
endif()

# --- Testing (Example using Google Test) ---
if(BUILD_TESTING)
    add_executable(client_gtests test/gtest_client_main.cpp)
    target_link_libraries(client_gtests PRIVATE
        # Link SUT or specific components
        synergy_protocol
        GTest::gtest
        GTest::gmock
        GTest::gtest_main
        Qt6::Core # Or other needed Qt modules
    )
    # Discover tests AND assign a label
    gtest_discover_tests(client_gtests LABELS "client_test")
endif()

# qt_add_executable(client_tests ...)
# target_link_libraries(client_tests PRIVATE SynergyStudioClient synergy_protocol Qt6::Core Qt6::Test)
# qt_add_test(client_tests client_tests)