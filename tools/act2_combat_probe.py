#!/usr/bin/env python3
"""Run controlled Act 2 encounters using the existing test_runner objects."""

import shlex
import subprocess
import sys
import tempfile
from pathlib import Path


def main():
    root = Path(__file__).resolve().parents[1]
    build = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else root / "build"
    link_file = build / "CMakeFiles/test_runner.dir/link.txt"
    if not link_file.exists():
        sys.exit("Build test_runner with CMake's Unix Makefiles generator first.")
    command = shlex.split(link_file.read_text())
    with tempfile.TemporaryDirectory(prefix="conr-act2-probe-") as temporary:
        work = Path(temporary)
        obj = work / "probe.o"
        exe = work / "probe"
        subprocess.run([
            command[0], "-std=gnu99", "-I" + str(root / "src"),
            "-I" + str(root / "external"), "-c",
            str(root / "tools/tests/act2_combat_probe.c"), "-o", str(obj),
        ], check=True)
        command = [arg for arg in command if "/tests/" not in arg]
        command.insert(1, str(obj))
        command[command.index("-o") + 1] = str(exe)
        subprocess.run(command, cwd=build, check=True)
        (work / "assets").symlink_to(root / "assets", target_is_directory=True)
        subprocess.run([str(exe)], cwd=work, check=True)


if __name__ == "__main__":
    main()
