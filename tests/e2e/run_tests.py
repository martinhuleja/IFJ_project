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
# Prompt: Implement main for one IFJ26 E2E test. Accept a compiler path
# as the sole command-line argument, load addition fixtures, check the
# compiler exit code, skip interpretation for expected compilation errors,
# otherwise interpret compiler stdout and compare exit code and output.
# Return 0 for PASS, 1 for FAIL and 2 for missing configuration or tools.
def main():
    if len(sys.argv) != 2:
        print("Použitie: python3 tests/e2e/run_tests.py CESTA_K_PREKLADACU")
        return 2

    compiler_path = Path(sys.argv[1]).resolve()
    project_dir = Path(__file__).resolve().parents[2]
    interpreter_path = project_dir / "ic26int"

    if not compiler_path.is_file():
        print(f"Prekladač neexistuje: {compiler_path}")
        return 2

    cases_dir = Path(__file__).resolve().parent / "cases"
    source_path = cases_dir / "addition.scala"
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


if __name__ == "__main__":
    try:
        exit_code = main()
    except subprocess.TimeoutExpired as error:
        print(f"FAIL: prekročený časový limit {error.timeout} s")
        print(f"  Príkaz: {error.cmd}")
        exit_code = 1
    except OSError as error:
        print(f"ERROR: chyba súboru alebo spustenia programu: {error}")
        exit_code = 2
    except (ValueError, KeyError, TypeError) as error:
        print(f"ERROR: neplatná konfigurácia testu: {error}")
        exit_code = 2

    raise SystemExit(exit_code)