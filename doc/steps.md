## Synergy Studio: Project Setup & CI Configuration Summary

This document summarizes the key configuration steps and decisions made for structuring the Synergy Studio project, setting up the build system, integrating testing, and configuring the CI pipeline using GitHub Actions.

### 1. Project Structure and Git Strategy

*   **Decision:** A **Monorepo** structure was chosen, containing all components within a single Git repository.
*   **Directory Layout:**
    ```
    synergy-studio/
    ├── client/         # Client-specific code and tests
    ├── common/         # Shared code (protocol, utilities) and its tests
    ├── server/         # Server-specific code and tests
    ├── .github/workflows/ # CI workflow files
    ├── CMakeLists.txt  # Top-level CMake file
    └── ...             # Other files (LICENSE, README.md)
    ```
*   **Rationale:** Simplifies dependency management (especially for the `common` library), allows atomic commits across components, simplifies build setup, and better reflects the coupled nature of the client/server system.

### 2. CMake Build System Setup

*   **Approach:** A hierarchical CMake structure using `add_subdirectory()` was adopted.
*   **`synergy-studio/CMakeLists.txt` (Top-Level):**
    *   Sets project basics (name, C++ standard).
    *   Finds project-wide dependencies (**Qt6**, **Google Test**).
    *   Includes the `GoogleTest` CMake module.
    *   Enables testing via `enable_testing()`.
    *   Uses `add_subdirectory()` to include `common`, `server`, and `client`.
*   **`common/CMakeLists.txt`:**
    *   Defines the `synergy_protocol` library target (initially decided as `INTERFACE` for header-only, adaptable to `STATIC` if `.cpp` files are added).
    *   Specifies include directories and dependencies (`Qt6::Core`) required by users of the library, using the `INTERFACE` keyword if it's an `INTERFACE` library.
    *   Conditionally (`if(BUILD_TESTING)`) defines a `common_tests` executable target for testing the common library code.
    *   Links `common_tests` against `synergy_protocol`, `GTest::*`, and necessary Qt modules.
    *   Assigns the CTest label `"common_test"` using `gtest_discover_tests(common_tests LABELS "common_test")`.
*   **`server/CMakeLists.txt` & `client/CMakeLists.txt`:**
    *   Define the `SynergyStudioServer` and `SynergyStudioClient` executable targets, respectively.
    *   List target-specific source files.
    *   Link against required Qt modules (`Qt6::Core`, `Qt6::Network` for server; `Qt6::Core`, `Qt6::Network`, `Qt6::Gui`, `Qt6::Widgets` for client).
    *   **Crucially**, link against the `synergy_protocol` target to use the shared code.
    *   Conditionally define respective test executables (`server_tests`, `client_tests`).
    *   Link test executables against `synergy_protocol`, `GTest::*`, and needed Qt modules.
    *   Assign CTest labels `"server_test"` and `"client_test"` respectively.


#### 2.1 Windows/MinGW Linker Fix (`__imp___argc` Error)

*   **Problem Encountered:** When building the `SynergyStudioClient` target on Windows using the MinGW (GNU) compiler toolchain, linker errors occurred (e.g., `undefined reference to '__imp___argc'`). This did not occur for the `SynergyStudioServer` target or on Linux builds.
*   **Root Cause:** The issue stemmed from linking a `WIN32` GUI application (the client) with Qt libraries compiled for MinGW. Qt's entry point mechanism (`libQt6EntryPoint.a`), which bridges standard `main` with Windows `WinMain`, requires symbols from the MinGW C/C++ runtime startup code (`__imp___argc`, `__imp___argv`). These were not being correctly resolved automatically by the linker for the GUI client target.
*   **Solution:** Explicitly linked necessary MinGW runtime and Windows system libraries within the `client/CMakeLists.txt` file, guarded by a condition to only apply when building on Windows with the GNU compiler.
*   **Implementation (`client/CMakeLists.txt`):** Added the following block *after* the initial `target_link_libraries` call that links Qt modules and `synergy_protocol`:
    ```cmake
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
    ```
*   **Rationale:** This ensures that the linker finds the required C runtime symbols (`mingw32`) and common Windows GUI libraries (`user32`, `gdi32`) before or alongside the Qt libraries that depend on them. Explicitly linking `Qt6::EntryPoint` further helps resolve potential linker confusion with the `WIN32` executable type. This conditional block ensures these libraries are not incorrectly linked on Linux or when using MSVC.

### 3. Qt Integration

*   **Finding Qt:** `find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network Test)` is placed in the **top-level** `CMakeLists.txt`.
*   **Locating Qt Installation:** The most reliable method identified was passing the path to the Qt installation via `CMAKE_PREFIX_PATH` during the CMake **configure** step on the command line (e.g., `cmake -B build -S . -D CMAKE_PREFIX_PATH="/path/to/Qt/6.x.y/gcc_64"`). Setting it inside the CMakeLists.txt was discouraged.
*   **OpenGL Dependency (Linux):** Resolved the `WrapOpenGL could not be found` error by installing the necessary OpenGL development package (`libgl1-mesa-dev` on Debian/Ubuntu).

### 4. Google Test Integration

*   **Method:** **`FetchContent`** was chosen over `find_package`.
*   **Location:** The `FetchContent_Declare` and `FetchContent_MakeAvailable` commands for `googletest` are placed in the **top-level** `CMakeLists.txt` *before* `add_subdirectory` calls.
*   **Rationale:** Provides a self-contained build (no need for pre-installed GTest), ensures consistent GTest version via `GIT_TAG`, simplifies setup for other developers/CI.
*   **Usage:** Test targets in subdirectories link against `GTest::gtest`, `GTest::gmock`, `GTest::gtest_main`.

### 5. CTest Integration and Labels

*   **Enablement:** `enable_testing()` called in the top-level `CMakeLists.txt`.
*   **Test Registration:** `gtest_discover_tests()` (from `include(GoogleTest)` called after `FetchContent_MakeAvailable`) is used to find tests within test executables.
*   **Labeling:** The `LABELS` argument is added to `gtest_discover_tests` in each component's `CMakeLists.txt` (`common_test`, `server_test`, `client_test`) to categorize tests. This is essential for conditional execution in CI.

### 6. GitHub Actions CI Workflow (`.github/workflows/build_test.yml`)

*   **Goal:** Build on Linux/Windows and run tests conditionally based on the target branch.
*   **Triggers:** Configured to run on pushes to `main`, `develop`, `develop/server`, `develop/client`, `develop/common`, and on pull requests targeting `main` or `develop`.
*   **Dependencies:** Uses `apt-get` on Ubuntu and `aqtinstall` (via pip) on Windows to install Qt dependencies reliably.
*   **Build:** Builds *all* targets (client, server, tests) on every triggered run to ensure compilation integrity.
*   **Conditional Testing:**
    *   Uses `if:` conditions on steps, evaluating variables set based on `github.event_name` and `github.ref_name`.
    *   **Runs All Tests:** Executes `ctest` (no `-L` flag) on pushes to `main`, `develop`, `develop/common`, and on PRs to `main`/`develop`. This implicitly runs tests with *any* label or no label.
    *   **Runs Specific Tests:** Executes `ctest -L server_test` on pushes to `develop/server`.
    *   **Runs Specific Tests:** Executes `ctest -L client_test` on pushes to `develop/client`.
*   **Shell Handling:** Removed explicit `shell:` key from CMake configure step, relying on runner defaults (`bash` for Linux, `pwsh` for Windows) as conditional logic is handled *within* the `run:` script.

### 7. Branch Protection (GitHub Setting)

*   **Configuration:** Recommended setting up branch protection rules for the `main` branch directly in the GitHub repository settings (Settings > Branches > Add rule).
*   **Key Rules:**
    *   "Require a pull request before merging" (enforces PR workflow).
    *   "Require status checks to pass before merging" (links to the `build_and_test` job in the CI workflow, ensuring CI must succeed).
    *   Optional: "Require approvals".

Okay, let's add a section to the summary markdown file explaining how we configured VS Code using the CMake Tools extension for a seamless development experience.

### 8.VS Code Integration (CMake Tools Extension)

To optimize the development workflow within Visual Studio Code, the **CMake Tools** extension was configured to manage the build, debug, and IntelliSense processes, minimizing the need for manual `tasks.json` or `launch.json` files for common operations.

**Key Configuration Steps:**

1.  **Extension Installation:** Ensured the **CMake Tools** and **C/C++** extensions from Microsoft are installed in VS Code.
2.  **CMake Kit Selection:** Selected the appropriate compiler toolchain (Kit) via the Command Palette (`CMake: Select a Kit`) or the status bar prompt, matching the development environment (e.g., GCC, Clang, MSVC).
3.  **Qt Path Configuration (`CMAKE_PREFIX_PATH`):**
    *   To enable CMake to find the Qt6 installation, `CMAKE_PREFIX_PATH` was added to the VS Code workspace settings.
    *   This was done by editing the `.vscode/settings.json` file:
        ```json
        {
            // Remove or comment out cmake.configureArgs if it only contains CMAKE_PREFIX_PATH
            // "cmake.configureArgs": [
            //     "-D CMAKE_PREFIX_PATH=/home/manojlo/Qt/6.9.0/gcc_64"
            // ],

            // Add this section instead:
            "cmake.environment": {
                "CMAKE_PREFIX_PATH": "/home/manojlo/Qt/6.9.0/gcc_64"
            },

            "cmake.generator": "Ninja" // Keep other settings
        }
        ```
    *   Setting this in `settings.json` is preferred over modifying the root `CMakeLists.txt` or relying solely on environment variables for persistent VS Code configuration.
4.  **CMake Configuration:** Ensured the project configured successfully within VS Code using the CMake Tools extension (e.g., via Command Palette `CMake: Configure` or `CMake: Delete Cache and Reconfigure` if needed). Checked the "CMake" output panel for errors.
5.  **IntelliSense Configuration (C/C++ Extension):**
    *   Addressed issues where VS Code showed squiggles for headers (like Qt's) that CMake could successfully find during build.
    *   **Action 1:** Used the Command Palette (`Ctrl+Shift+P`) -> `C/C++: Select IntelliSense Configuration...` and explicitly chose the **"CMake Tools"** option as the configuration provider.
    *   **Action 2:** Used the Command Palette -> `C/C++: Reset IntelliSense Database` to force the C/C++ extension to re-parse the project using the configuration provided by CMake Tools.
    *   **Action 3:** Verified that no conflicting manual configuration existed in `.vscode/c_cpp_properties.json` (deleting or renaming this file if necessary).
6.  **Running & Debugging CMake Targets:**
    *   Utilized the VS Code **Status Bar** controls provided by the CMake Tools extension:
    *   **Select Launch Target:** Clicked the target name (e.g., `[SynergyStudioServer]`) in the status bar to select which executable (`SynergyStudioClient`, `SynergyStudioServer`, `common_tests`, etc.) to run or debug.
    *   **Run:** Clicked the **Play icon (triangle)** in the status bar to run the selected launch target without debugging.
    *   **Debug:** Clicked the **Bug icon** in the status bar to run the selected launch target with the debugger attached (uses the C/C++ extension's debugger).
7.  **Running Tests (CTest):**
    *   Utilized the VS Code **Status Bar** controls:
    *   Clicked the **Test icon (beaker)** in the status bar to run all tests registered with CTest (`common_tests`, `server_tests`, `client_tests`).
    *   Alternatively, used Command Palette (`Ctrl+Shift+P`) -> `CMake: Run CTest`.

**Outcome:** This setup allows developers to configure, build, run, debug, and test the CMake project targets directly from within the VS Code interface using the integrated controls provided by the CMake Tools extension, streamlining the development cycle.
