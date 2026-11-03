#!/bin/sh
# AI-generated code (ChatGPT).
# Prompt: Compile the codegen constants test driver in a temporary
# directory, compare its output with the expected file using diff,
# stop on failure and automatically remove temporary files.

set -eu

project_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
temp_dir=$(mktemp -d)
trap 'rm -rf "$temp_dir"' EXIT
trap 'exit 1' HUP INT TERM

cd "$project_dir"

gcc -std=c11 -Wall -Wextra -Werror -pedantic -Isrc \
    tests/codegen_string_test.c src/codegen.c \
    -o "$temp_dir/codegen_test"

"$temp_dir/codegen_test" > "$temp_dir/actual.code"

diff -u tests/codegen_constants.expected "$temp_dir/actual.code"

printf 'PASS: codegen constants\n'