# AI-generated code (ChatGPT).
# Prompt: Write the initial part of an IFJ26 E2E test runner that
# locates addition.scala relative to the script, reads addition.json
# and prints the test name and expected compiler exit code.

import json
from pathlib import Path
import subprocess
import tempfile
import sys

# AI-generated code (ChatGPT).
# Prompt: Extract interpreter execution into a function accepting its path,
# generated code bytes and program input bytes. Use a temporary code file,
# capture stdout and stderr, apply a five-second timeout and return the result.
def run_interpreter(interpreter_path, code, program_input):
    with tempfile.TemporaryDirectory() as temp_dir:
        code_path = Path(temp_dir) / "program.code"
        code_path.write_bytes(code)

        return subprocess.run(
            [str(interpreter_path), str(code_path)],
            input=program_input,
            capture_output=True,
            timeout=5,
        )

# AI-generated code (ChatGPT).
# Prompt: Define a function that runs a compiler executable, passes source
# bytes to stdin, captures stdout and stderr, applies a five-second timeout
# and returns the subprocess result.
def run_compiler(compiler_path, source):
    return subprocess.run(
        [str(compiler_path)],
        input=source,
        capture_output=True,
        timeout=5,
    )

# AI-generated code (ChatGPT).
# Prompt: Extract the existing single-test logic into run_test, accepting
# the source path, compiler path and interpreter path as arguments.
# Preserve its exit-code and exact-output comparisons.
def run_test(source_path, compiler_path, interpreter_path):
    config = json.loads(
        source_path.with_suffix(".json").read_text(encoding="utf-8")
    )

    compilation = run_compiler(compiler_path, source_path.read_bytes())

    if compilation.returncode != config["compiler_exit"]:
        print(f"FAIL {source_path.stem}: návratový kód prekladača")
        print(f"  Očakávaný: {config['compiler_exit']}")
        print(f"  Skutočný:  {compilation.returncode}")
        print(f"  Stderr: {compilation.stderr!r}")
        return 1

    if config["compiler_exit"] != 0:
        print(f"PASS {source_path.stem}: očakávaná chyba prekladu")
        return 0

    if not interpreter_path.is_file():
        print(f"Interpret neexistuje: {interpreter_path}")
        return 2

    execution = run_interpreter(
        interpreter_path,
        compilation.stdout,
        source_path.with_suffix(".in").read_bytes(),
    )

    expected_output = source_path.with_suffix(".out").read_bytes()
    exit_ok = execution.returncode == config["interpreter_exit"]
    output_ok = execution.stdout == expected_output

    if exit_ok and output_ok:
        print(f"PASS {source_path.stem}")
        return 0

    print(f"FAIL {source_path.stem}")

    if not exit_ok:
        print(f"  Očakávaný kód interpretu: {config['interpreter_exit']}")
        print(f"  Skutočný kód interpretu:  {execution.returncode}")
        print(f"  Stderr: {execution.stderr!r}")

    if not output_ok:
        print(f"  Očakávaný výstup: {expected_output!r}")
        print(f"  Skutočný výstup:  {execution.stdout!r}")

    return 1

# AI-generated code (ChatGPT).
# Prompt: Discover all .scala cases recursively in the cases directory.
# Run each using run_test, catch timeouts and configuration or OS errors
# per case, continue after failures, and print PASS/FAIL/ERROR counts.
# Return 2 if any ERROR occurred, otherwise 1 for failures or 0 for success.
# Treat an empty suite as an error.
def main():
    if len(sys.argv) not in (2, 3):
        print("Použitie: python3 tests/e2e/run_tests.py CESTA_K_PREKLADACU [CESTA_K_INTERPRETU]")
        return 2

    compiler_path = Path(sys.argv[1]).resolve()
    project_dir = Path(__file__).resolve().parents[2]
    interpreter_path = (
        Path(sys.argv[2]).expanduser().resolve()
        if len(sys.argv) == 3
        else project_dir / "ic26int"
    )
    cases_dir = Path(__file__).resolve().parent / "cases"

    if not compiler_path.is_file():
        print(f"Prekladač neexistuje: {compiler_path}")
        return 2

    source_paths = sorted(cases_dir.rglob("*.scala"))

    if not source_paths:
        print(f"ERROR: nenašli sa žiadne testy v {cases_dir}")
        return 2

    counts = [0, 0, 0]

    for source_path in source_paths:
        name = source_path.relative_to(cases_dir)
        print(f"\nTest: {name}")

        try:
            result = run_test(
                source_path, compiler_path, interpreter_path
            )
        except subprocess.TimeoutExpired as error:
            print(f"FAIL {name}: prekročený limit {error.timeout} s")
            print(f"  Príkaz: {error.cmd}")
            result = 1
        except OSError as error:
            print(f"ERROR {name}: {error}")
            result = 2
        except (ValueError, KeyError, TypeError) as error:
            print(f"ERROR {name}: neplatná konfigurácia: {error}")
            result = 2

        counts[result] += 1

    print(
        f"\nSpolu: {len(source_paths)}, "
        f"PASS: {counts[0]}, FAIL: {counts[1]}, ERROR: {counts[2]}"
    )

    if counts[2] > 0:
        return 2
    if counts[1] > 0:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
