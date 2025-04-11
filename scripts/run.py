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
    parser = argparse.ArgumentParser(description="Run the compiled project or a quick spike.")
    parser.add_argument("-t", "--target", help="Specify the target executable to run", default="main")
    
    args = parser.parse_args()

    build_dir = ""
    executable = ""

    if not args.target:
        print("No target specified")
        sys.exit(-1)

    if args.target == "server":
        build_dir = "build/server"
        executable = os.path.join(build_dir, "SynergyStudioServer")
    elif args.target == "client":
        build_dir = "build/client"
        executable = os.path.join(build_dir, "SynergyStudioClient")
    elif args.target == "common":
        build_dir = "build/common"
        executable = os.path.join(build_dir, "SynergyStudioCommon")
    else:
        print("Wrong target specified")
        sys.exit(-1)
    
    if not os.path.exists(executable):
        print(f"Error: Executable {executable} not found. Did you compile it?")
        exit(1)
    
    # Run the executable
    run_command([executable])
    
if __name__ == "__main__":
    main()
