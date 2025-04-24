#!/usr/bin/env python3

import argparse
import subprocess
import os
import sys

def run_command(command):
    """Helper function to run a shell command."""
    print(f"Running: {' '.join(command)}")
    result = subprocess.run(command)

def main():
    parser = argparse.ArgumentParser(description="Compile and build project using CMake.")
    
    # Changed from default=None to action="append" to collect multiple definitions
    parser.add_argument("-d", "--define", 
                        help="Set compile definitions for Project (can be used multiple times)", 
                        action="append", dest="definitions")
    parser.add_argument("-dbg", "--debug", 
                        help="Enable debug mode with debug flags", 
                        action="store_true")
    parser.add_argument("-gt", "--googletest",
                        help="Start googleTest tests. Must name executable",
                        action="store_true")
    
    args = parser.parse_args()
    build_dir = "build"
    os.makedirs(build_dir, exist_ok=True)
    

    qt_prefix_path = "/home/manojlo/Qt/6.9.0/gcc_64/" # Define your path clearly
    build_env = os.environ.copy()
    build_env['CMAKE_PREFIX_PATH'] = qt_prefix_path
    cmake_command = ["cmake", "-B", build_dir, "-S", ".", "-G", "Ninja"]
    
    # Handle multiple definitions by adding them to CMAKE_CXX_FLAGS
    if args.definitions:
        # Join all definitions with proper formatting
        definitions_str = " ".join([f"-D{definition}" for definition in args.definitions])
        cmake_command.append(f"-DCMAKE_CXX_FLAGS={definitions_str}")
    
    if args.debug:
        cmake_command.append("-DCMAKE_BUILD_TYPE=Debug")
        cmake_command.append("-DCMAKE_RULE_MESSAGES:BOOL=OFF")
        cmake_command.append("-DCMAKE_VERBOSE_MAKEFILE:BOOL=ON")

    if args.googletest:
        cmake_command.append("-DBUILD_TESTING=ON")

    
    print(f"Running CMake Command: {' '.join(cmake_command)}")
    print(f"With Environment CMAKE_PREFIX_PATH={build_env['CMAKE_PREFIX_PATH']}") # Verify env var


    try:
        # Use check=True to raise an exception if CMake fails
        # Pass the modified environment using 'env='
        # Run without shell=True for better argument handling
        print(f"Running: {' '.join(cmake_command)}")
        configure_process = subprocess.run(cmake_command, check=True, env=build_env)
        print("CMake configuration successful!")

        # --- Run the Build ---
        build_command = ['cmake', '--build', build_dir]
        print(f"Running: {' '.join(build_command)}")
        print(f"Running Build Command: {' '.join(build_command)}")
        build_process = subprocess.run(build_command, check=True)
        print("Build successful!")

    except subprocess.CalledProcessError as e:
        print(f"Error during CMake process: {e}", file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print("Error: 'cmake' command not found. Is CMake installed and in PATH?", file=sys.stderr)
        sys.exit(1)

    
    print("Build complete!")

if __name__ == "__main__":
    main()
