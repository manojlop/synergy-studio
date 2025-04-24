#!/usr/bin/env python3
import argparse
import subprocess
import sys
import os

def main():
    parser = argparse.ArgumentParser(description="Quickly call other scripts")
    
    # Changed --define to use action="append" to handle multiple definitions
    parser.add_argument("-cl", "--clean", 
                        help="Run clean script",
                        action="store_true")
    parser.add_argument("-b", "--build", 
                        help="Build whole project (common, client, server)",
                        action="store_true")
    parser.add_argument("-r", "--run", 
                        help="Run tefined target after building. Needs target specified",
                        action="store_true")
    parser.add_argument("-a", "--all", 
                        help="Build and run target. Needs target specified",
                        action="store_true")
    parser.add_argument("-t", "--target", 
                        help="Target filename for compilation and execution. Must be server, client or common", 
                        default=None)
    parser.add_argument("-d", "--define", 
                        help="Set compile definitions for Project (can be used multiple times)",
                        action="append", dest="definitions")
    parser.add_argument("-dbg", "--debug", 
                        help="Enable debug mode with debug flags", 
                        action="store_true")
    parser.add_argument("-gt", "--googletest",
                        help="Start googleTest tests. Must name executable",
                        default=None)
    
    args = parser.parse_args()

    build_dir = "build/bin"

    script_dir = os.path.dirname(os.path.abspath(__file__))
    script_dir = os.path.join(script_dir, "scripts")

    if args.clean:
      clean_script = os.path.join(script_dir, "clean.py")
      clean_command = [sys.executable, clean_script]

      print(f"Running clean script: {' '.join(clean_command)}")
      build_result = subprocess.run(clean_command)
    

    if args.build or args.all:
      # Construct command for the build script
      build_script = os.path.join(script_dir, "build.py")
      build_command = [sys.executable, build_script]

      if args.definitions:
        for definition in args.definitions:
          build_command.extend(["-d", definition])

      if args.debug:
        build_command.append("-dbg")

      if args.googletest:
        build_command.append("-gt")

      # Run the build script
      print(f"Running build script: {' '.join(build_command)}")
      build_result = subprocess.run(build_command)
      
      # Check if build was successful
      if build_result.returncode != 0:
        print("Build failed. Aborting run.")
        sys.exit(build_result.returncode)
      
      if args.build:
         sys.exit(build_result.returncode)

    if args.run or args.all:
      if not args.target:
        print("No target specified")
        sys.exit(-1)
      
      if (args.all and (args.run or  args.googletest)) or (args.run and (args.all or args.googletest)) or (args.googletest and (args.all or args.run)):
        print("You can specify only one of run commands")
        sys.exit(-2)
      
      # Construct command for the run script
      run_script = os.path.join(script_dir, "run.py")
      run_command = [sys.executable, run_script]

      if args.target:
          run_command.extend(["-t", args.target])
      if args.googletest:
          run_command.extend(["-gt", args.googletest])

      # Run the run script
      print(f"Running run script: {' '.join(run_command)}")
      run_result = subprocess.run(run_command)
      
      # Return the exit code from the run script
      sys.exit(run_result.returncode)
          

    # Return the exit code from the run script
    sys.exit(0)

if __name__ == "__main__":
    main()