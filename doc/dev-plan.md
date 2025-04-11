**Development Plan: Synergy Studio MVP**

**Phase 0: Foundation & Setup (Estimate: 1-2 Weeks)**

*   **Goal:** Establish the basic project structure, development environment, and build system. Verify core tools are functional.
*   **Tasks:**
    1.  **Environment Setup:**
        *   Install/verify C++ compiler (GCC/Clang/MSVC).
        *   Install/verify CMake (v3.15+).
        *   Install Qt 6 SDK (Core, GUI, Widgets, Network, Test modules).
        *   Configure IDE (VS Code, Qt Creator, CLion, etc.) for C++/Qt/CMake development.
        *   Install/verify Git client.
        *   Install/verify Docker Desktop or Docker Engine for Linux.
    2.  **Project Initialization:**
        *   Create main project directory.
        *   Initialize Git repository (`git init`).
        *   Create initial directory structure (e.g., `client/`, `server/`, `common/`, `doc/`, `scripts/`).
    3.  **Build System Setup (CMake):**
        *   Create top-level `CMakeLists.txt`.
        *   Create basic `CMakeLists.txt` for `SynergyStudioServer` executable target.
        *   Create basic `CMakeLists.txt` for `SynergyStudioClient` executable target.
        *   Ensure CMake can find Qt6 packages.
        *   Configure basic Debug/Release build types.
        *   Successfully build empty "main" functions for both client and server.
    4.  **Initial Commit:** Commit the basic project structure and CMake files to Git.
*   **Deliverable:** A compilable (though non-functional) Client and Server project structure under Git version control. Development environment confirmed working.

**Phase 1: Secure Connection & Basic Structures (Estimate: 2-4 Weeks)**

*   **Goal:** Establish a secure TLS connection between Client and Server, handle basic session creation/joining (no state sync yet), and display the initial workspace structure.
*   **Tasks:**
    1.  **TLS Setup:**
        *   Generate self-signed TLS certificates (e.g., using OpenSSL) for server key/cert for testing (Ref: SRV-FUNC-NM-003). Document this process.
        *   Server: Implement `SslServer` logic using `QSslSocket` to listen and perform TLS handshake upon connection (Ref: SRV-FUNC-NM-001, SRV-FUNC-NM-002). Load cert/key from configurable paths (Ref: SRV-FUNC-CONF-002).
        *   Client: Implement `SslClient` logic using `QSslSocket` to connect and perform TLS handshake (Ref: CLI-FUNC-NM-003). Handle basic certificate validation (allowing self-signed for now) (Ref: CLI-FUNC-NM-004).
        *   Client: Implement basic UI for entering server address/port and a "Connect" button (Ref: CLI-FUNC-NM-001).
        *   Client/Server: Implement basic `clientHello`/`serverHello` message exchange upon successful TLS connection (Ref: Sec 5.4.1, 5.5.1).
    2.  **Message Framing/Parsing:**
        *   Client/Server: Implement logic to prepend 4-byte length prefix to outgoing JSON messages (Ref: PROT-TF-004).
        *   Client/Server: Implement logic to read incoming data, extract length prefix, read the full JSON payload based on length, and parse JSON (using `QJsonDocument`) (Ref: PROT-TF-005, SRV-FUNC-NM-009, CLI-FUNC-NM-009). Handle errors.
    3.  **Basic Session Management:**
        *   Server: Implement basic `SessionManager` and `Session` classes. Handle `createSessionRequest` -> generate unique ID, create Session object, respond with `sessionCreatedResponse` (Ref: SRV-FUNC-SM-001, 002, 004).
        *   Server: Handle `joinSessionRequest` -> lookup Session by ID, (skip password for now), add client connection to Session, respond `joinSessionResponse` (Ref: SRV-FUNC-SM-005, 007). Handle failure case (Ref: SRV-FUNC-SM-008).
        *   Server: Handle client disconnects – remove client from Session (Ref: SRV-FUNC-NM-007).
        *   Client: Implement UI for Create/Join Session actions (Ref: CLI-FUNC-SM-001, 002). Process responses (Ref: CLI-FUNC-SM-003, 004).
    4.  **Workspace Tree Display:**
        *   Server: Implement basic `WorkspaceManager` to associate a directory with a Session (Ref: SRV-FUNC-SM-003).
        *   Server: Handle `requestFileTree` -> scan directory (using `QDir` or `std::filesystem`), generate JSON tree structure, send `fileTreeUpdate` (Ref: SRV-FUNC-WM-003, 004).
        *   Client: Send `requestFileTree` upon successful join (Ref: CLI-FUNC-WM-001).
        *   Client: Implement `QTreeView` UI component (Ref: CLI-FUNC-WM-002).
        *   Client: Process `fileTreeUpdate`, parse JSON, populate the `QTreeView` model (Ref: CLI-FUNC-WM-003).
*   **Deliverable:** Client can securely connect to Server. Client can create/join a session. Client displays the file structure of a predefined server-side directory associated with the session. Basic message framing works.

**Phase 2: Active File & Text Sync (MVP) (Estimate: 3-5 Weeks)**

*   **Goal:** Implement the "Workspace Lite" active file concept and the simplified real-time text synchronization.
*   **Tasks:**
    1.  **Active File Selection:**
        *   Client: Handle user selection in `QTreeView` -> send `requestOpenFile` with relative path (Ref: CLI-FUNC-WM-004, 005).
        *   Server: Handle `requestOpenFile` -> validate path (CRITICAL - Ref: SRV-FUNC-WM-006), read file content (Ref: SRV-FUNC-WM-007), set as Active File in Session state (Ref: SRV-FUNC-WM-008), send `setActiveFile` to all clients in session (Ref: SRV-FUNC-WM-009).
        *   Client: Implement `QTextEdit` UI component (Ref: CLI-FUNC-WM-006).
        *   Client: Handle `setActiveFile` -> display content in `QTextEdit`, update status bar (Ref: CLI-FUNC-WM-007). Disable editor if no file active (Ref: CLI-FUNC-WM-012).
    2.  **Text Edit Transmission:**
        *   Client: Detect local changes in `QTextEdit` (e.g., `textChanged` signal) (Ref: CLI-FUNC-WM-009).
        *   Client: Send `updateTextEdit` (C->S) containing full content and active file path to server (Ref: CLI-FUNC-WM-010).
    3.  **Text Edit Broadcasting & Display:**
        *   Server: Handle `updateTextEdit` (C->S) -> validate it's for the active file, update internal buffer (Ref: SRV-FUNC-WM-011).
        *   Server: Persist change to file (e.g., immediate save or on file close - Ref: SRV-FUNC-WM-012).
        *   Server: Broadcast `updateTextEdit` (S->C) with full content to all *other* clients (Ref: SRV-FUNC-SYNC-002). Include `originatorUserId`.
        *   Client: Handle `updateTextEdit` (S->C) -> check if it's for the current active file and not self-originated, replace content in `QTextEdit` (Ref: CLI-FUNC-WM-011). Handle cursor position potentially being lost.
*   **Deliverable:** Multiple clients can join the same session, select the same file, and see edits made by other users appear in near real-time (with "last writer wins" behavior).

**Phase 3: Docker Execution Integration (Estimate: 3-4 Weeks)**

*   **Goal:** Implement the core feature of executing code within Docker containers via the server.
*   **Tasks:**
    1.  **Docker Worker Image:**
        *   Create a simple `Dockerfile` (e.g., based on `ubuntu:latest` or `debian:stable-slim`).
        *   Install necessary tools (e.g., `build-essential`, `python3`).
        *   Build this image locally and tag it (e.g., `synergy-worker:latest`). Document image requirements.
    2.  **Server Docker API Client:**
        *   Server: Implement `DockerExecutor` class.
        *   Server: Choose & integrate HTTP client (`QNetworkAccessManager` or `cpr`) and JSON library (`QJsonDocument` or `nlohmann/json`).
        *   Server: Implement asynchronous functions (using thread pool recommended) to perform the Docker API sequence (Create, Start, Archive Upload [needs TAR logic], Exec Create, Exec Start+Stream, Exec Inspect, Cleanup) (Ref: SRV-FUNC-DOCKER-004). Ensure robust error handling for each API call (Ref: SRV-FUNC-DOCKER-007).
    3.  **Execution Flow:**
        *   Client: Add "Run" UI element (Ref: CLI-FUNC-EXEC-001).
        *   Client: On "Run" click, send `requestRunCode` with appropriate command/args (MVP can hardcode `make` or `python3 active_file.py`) (Ref: CLI-FUNC-EXEC-002).
        *   Server: Handle `requestRunCode` (Ref: SRV-FUNC-DOCKER-001). Trigger `DockerExecutor` logic.
        *   Server: Aggregate stdout, stderr, exit code (Ref: SRV-FUNC-DOCKER-005).
        *   Server: Send `runOutputResult` to all clients in session (Ref: SRV-FUNC-DOCKER-006).
    4.  **Client Output Display:**
        *   Client: Implement Output View UI component (e.g., `QTextBrowser`) (Ref: CLI-FUNC-EXEC-004).
        *   Client: Handle `runOutputResult` -> display stdout/stderr (distinguished visually) and exit code (Ref: CLI-FUNC-EXEC-005, 006, 007). Clear previous output (Ref: CLI-FUNC-EXEC-008).
*   **Deliverable:** A user can click "Run" in the client, and the server will (1) copy the current workspace, (2) execute a predefined command (like `make`) inside a Docker container using that workspace, (3) capture the output, and (4) display the output back in all connected clients' UIs.

**Phase 4: Drawing Canvas Integration (Estimate: 2-3 Weeks)**

*   **Goal:** Implement the synchronized drawing canvas overlay.
*   **Tasks:**
    1.  **Canvas UI:**
        *   Client: Create transparent `CanvasWidget` overlaying the `EditorWidget` (Ref: CLI-FUNC-DRAW-001).
        *   Client: Implement basic drawing logic (`QPainter`) capturing mouse events to draw lines (Ref: CLI-FUNC-DRAW-003, 004, 005).
        *   Client: Add UI controls to toggle canvas visibility/activation and clear the canvas (Ref: CLI-FUNC-DRAW-002, 009).
    2.  **Drawing Sync:**
        *   Client: Translate local draw actions into `drawCommand` (C->S) messages and send to server (Ref: CLI-FUNC-DRAW-006).
        *   Server: Handle `drawCommand` (C->S), broadcast to other clients (Ref: SRV-FUNC-SYNC-003, 004).
        *   Client: Handle incoming `drawCommand` (S->C), render the action on the local canvas (Ref: CLI-FUNC-DRAW-007, 008).
    3.  **Canvas Clear Sync:**
        *   Client: Send `clearCanvasRequest` on user action (Ref: CLI-FUNC-DRAW-009).
        *   Server: Handle `clearCanvasRequest`, broadcast `clearCanvasBroadcast` (Ref: SRV-FUNC-SYNC-006).
        *   Client: Handle `clearCanvasBroadcast`, clear local canvas rendering (Ref: CLI-FUNC-DRAW-010).
*   **Deliverable:** Users can draw simple freehand lines on the canvas overlay, and these lines appear in near real-time for other users in the session. Canvas can be cleared.

**Phase 5: Testing, Polish & Documentation (Estimate: 2-4 Weeks)**

*   **Goal:** Improve stability, usability, add unit tests, and finalize documentation for the MVP.
*   **Tasks:**
    1.  **Integration Testing:** Thoroughly test all features working together across multiple clients. Identify and fix bugs related to concurrency, state consistency (within MVP limits), and error handling.
    2.  **Unit Testing:**
        *   Implement unit tests (Qt Test or Google Test) for critical non-GUI server logic: Protocol message parsing/serialization, Workspace path validation, Session state transitions (basic), Docker command generation/parsing logic snippets. (Ref: NFR-MAINT-003).
        *   Implement basic unit tests for client-side protocol handling or state management logic if feasible.
    3.  **Error Handling Refinement:** Improve error reporting and handling throughout the client and server based on testing feedback. Ensure graceful failure modes (Ref: NFR-REL-002, 004). Check resource management (RAII) (Ref: NFR-REL-005).
    4.  **GUI Polish:** Refine UI layout, add missing status indicators, improve visual feedback based on usability testing (Ref: NFR-USE). Add basic menus if not already fully implemented.
    5.  **Code Cleanup & Refactoring:** Review code for clarity, consistency (Ref: NFR-MAINT-002), modularity (Ref: NFR-MAINT-001), and adherence to C++/Qt best practices. Add comments where needed.
    6.  **README Documentation:** Write the comprehensive `README.md` file as specified (Ref: DOC-README-001 to 004). Include build, setup, configuration, and basic usage instructions.
    7.  **Final Build & Test:** Perform final builds on target platforms (Linux, Windows) and run core functionality tests.
    8.  **(Optional) Basic Packaging:** Explore using CPack or platform deployment tools (`windeployqt`, `linuxdeployqt`) to create simple distributable packages.
*   **Deliverable:** A stable, functional MVP release candidate meeting the specified requirements, with unit tests for key backend components, and a comprehensive README file. Codebase is cleaned and version tagged in Git.