#!/usr/bin/env python3
"""Written with AI assistance (OpenAI Codex).

Relevant prompt: "Generate me some unit tests for lexer part. Start small.
Maybe one file lexer-tests.c and one script to run all the tests.
Make sure to test every state's return token for start."
Follow-up prompt: "maybe on the write out have it more detailed"

Compile and run every case in lexer-tests.c using a fresh process per case.

Run from any directory: python3 /path/to/project/tests/run-lexer-tests.py
Uses GCC, Python's standard library, and POSIX stdin redirection in the C tests.
"""

from pathlib import Path
import subprocess
import tempfile


def main():
    project = Path(__file__).resolve().parent.parent
    with tempfile.TemporaryDirectory(prefix="ifj-lexer-tests-") as directory:
        executable = Path(directory) / "lexer-tests"
        compilation = subprocess.run([
            "gcc", "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic", "-g",
            "-I", str(project / "src"),
            str(project / "tests" / "lexer-tests.c"),
            str(project / "src" / "lexer.c"),
            "-o", str(executable),
        ], check=False)
        if compilation.returncode != 0:
            return 1

        listing = subprocess.run(
            [str(executable), "--list"], capture_output=True, text=True,
            check=True, timeout=5,
        )
        names = listing.stdout.splitlines()
        if not names:
            print("ERROR: no lexer tests found")
            return 1

        failures = 0
        print("Source positions marked * are not checked by that case.")
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

            if result.stdout:
                for line in result.stdout.splitlines():
                    print(f"  {line}")
            if result.stderr:
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
