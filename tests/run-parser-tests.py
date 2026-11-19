#!/usr/bin/env python3
"""Written with AI assistance (OpenAI Codex).

Relevant prompt: "You can start writing the test file for the parser."

Run from any directory: python3 /path/to/project/tests/run-parser-tests.py
Each case starts a fresh process to isolate the lexer's static state.
"""

from pathlib import Path
import subprocess
import tempfile


def main():
    project = Path(__file__).resolve().parent.parent
    with tempfile.TemporaryDirectory(prefix="ifj-parser-tests-") as directory:
        executable = Path(directory) / "parser-tests"
        sources = [
            project / "tests/parser-tests.c",
            project / "src/parser.c",
            project / "src/precedence-analysis.c",
            project / "src/abstract-syntactic-tree.c",
            project / "src/lexer.c",
            project / "src/error.c",
        ]
        compilation = subprocess.run([
            "gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic", "-g",
            "-I", str(project / "src"),
            *[str(source) for source in sources], "-o", str(executable),
        ], check=False)
        if compilation.returncode != 0:
            return 1

        listing = subprocess.run(
            [str(executable), "--list"], capture_output=True, text=True,
            check=True, timeout=5,
        )
        names = listing.stdout.splitlines()
        if not names:
            print("ERROR: no parser tests found")
            return 1

        failures = 0
        for index, name in enumerate(names, start=1):
            print(f"\n[{index}/{len(names)}] {name}")
            try:
                result = subprocess.run(
                    [str(executable), name], capture_output=True, text=True,
                    check=False, timeout=5,
                )
            except subprocess.TimeoutExpired:
                print(f"FAIL {name}: timed out after 5 seconds")
                failures += 1
                continue

            for line in result.stdout.splitlines():
                print(f"  {line}")
            for line in result.stderr.splitlines():
                print(f"  {line}")
            if result.returncode == 0:
                print(f"PASS {name}")
            else:
                failures += 1
                print(f"FAIL {name}: exit code {result.returncode}")

        print(f"\n{len(names) - failures} passed, {failures} failed")
        return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
