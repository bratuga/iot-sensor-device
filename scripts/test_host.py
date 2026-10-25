"""Run isolated sensor-core tests and the existing portable KNX regression suite."""
from pathlib import Path
import subprocess
import shutil
import sys
root = Path(__file__).resolve().parents[1]
compiler = shutil.which("g++") or shutil.which("clang++")
if not compiler:
    raise SystemExit("Install a C++14 compiler (g++ or clang++)")
build = root / "build"
build.mkdir(exist_ok=True)
for name in ("core", "heating", "heating_runtime", "heating_simulation", "thermostat"):
    exe = build / (name + "_tests" + (".exe" if sys.platform == "win32" else ""))
    subprocess.run([compiler, "-std=c++14", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined", "-fno-sanitize-recover=all", "-I", str(root / "include"), str(root / ("test/" + name + "_tests.cpp")), "-o", str(exe)], check=True)
    subprocess.run([str(exe)] + ([str(root / "test/fixtures/knx_v4_feedback.txt")] if name == "heating_simulation" else []), check=True)
bme_exe = build / ("bme280_tests" + (".exe" if sys.platform == "win32" else ""))
subprocess.run([compiler, "-std=c++14", "-Wall", "-Wextra", "-Werror", "-fsanitize=undefined", "-fno-sanitize-recover=all", "-I", str(root / "test/bme_fakes"), "-I", str(root / "include"), str(root / "test/bme280_tests.cpp"), "-o", str(bme_exe)], check=True)
subprocess.run([str(bme_exe)], check=True)
subprocess.run([sys.executable, str(root / "lib/knx-tunneling/scripts/run_tests.py"), "--compiler", compiler, "--sanitize"], check=True)
