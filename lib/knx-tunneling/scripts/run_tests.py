"""Compile and execute the portable C++ tests. No network or attached device."""
import argparse
from pathlib import Path
import shutil
import subprocess
import sys
import os

root = Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser()
parser.add_argument("--compiler", help="Path to clang++, g++, or zig executable")
parser.add_argument("--sanitize", action="store_true", help="Enable undefined-behavior checks")
args = parser.parse_args()
compiler = args.compiler or shutil.which("clang++") or shutil.which("g++")
if not compiler:
    parser.error("Provide --compiler with a C++14 compiler (clang++, g++, or zig)")
build = root / "build"
build.mkdir(exist_ok=True)
exe = build / ("tunnel_tests.exe" if sys.platform == "win32" else "tunnel_tests")
command = [compiler]
if Path(compiler).stem.lower() == "zig":
    command += ["c++"]
command += ["-std=c++14", "-Wall", "-Wextra", "-Werror", "-O1", "-g",
            "-I", str(root / "include"), str(root / "src/TunnelClient.cpp"),
            str(root / "tests/tunnel_tests.cpp"), "-o", str(exe)]
if args.sanitize:
    command += ["-fsanitize=undefined", "-fno-sanitize-recover=all"]
environment = os.environ.copy()
environment["ZIG_GLOBAL_CACHE_DIR"] = str(build / "zig-global-cache")
environment["ZIG_LOCAL_CACHE_DIR"] = str(build / "zig-local-cache")
subprocess.run(command, check=True, env=environment)
subprocess.run([str(exe)], check=True)
