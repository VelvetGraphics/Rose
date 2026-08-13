import argparse
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--generator", default = "Visual Studio 18 2026")

args = parser.parse_args()

cmd = [
    "cmake",
    "-S",
    ".",
    "-B",
    "build-vs",
    "-G",
    args.generator
]

print("Running: ".join(cmd))
subprocess.run(cmd, check = True)