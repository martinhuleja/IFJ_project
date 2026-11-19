# IFJ + IAL projekt 2026/2027
# Makefile
# Martin Huleja (xhulejm00)
# Adam Havier (xhaviea00)
# Richard Komora (xkomorr00)
# Enzo Štětina (xstetie00)

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic -g
TARGET = translator
SRC = src/translator.c src/symtable.c src/error.c src/semantic_check.c src/parser.c src/precedence-analysis.c src/abstract-syntactic-tree.c src/lexer.c src/codegen.c
# Clang-format configuration
FORMAT = clang-format
FORMAT_FLAGS = --dry-run -Werror

.PHONY: all build check build-check format clean doc clean-doc clean-all test-symtable

all: build

# Compiles the target binary
build: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

# Runs check
check:
	$(FORMAT) $(FORMAT_FLAGS) src/*
	# tests/...

# Runs check first then builds the binary
build-check: check build

# Formats source files
format:
	$(FORMAT) -i src/*

clean:
	rm -f $(TARGET) $(SYMTABLE_TEST) $(SEMANTIC_TEST)

# Builds and runs the symbol table unit tests
SYMTABLE_TEST = tests/symtable_tests
$(SYMTABLE_TEST): tests/symtable_tests.c src/symtable.c src/symtable.h src/abstract-syntactic-tree.h src/error.h
	$(CC) $(CFLAGS) -Isrc tests/symtable_tests.c src/symtable.c -o $@

	

test-symtable: $(SYMTABLE_TEST)
	./$(SYMTABLE_TEST)

# Builds and runs the semantic check unit tests
SEMANTIC_TEST = tests/semantic_check_tests
$(SEMANTIC_TEST): tests/semantic_check_tests.c src/semantic_check.c src/semantic_check.h src/symtable.c src/symtable.h src/token.h
	$(CC) $(CFLAGS) -Isrc tests/semantic_check_tests.c src/semantic_check.c src/symtable.c -o $@

test-semantic: $(SEMANTIC_TEST)
	./$(SEMANTIC_TEST)

# Runs all unit tests
test: test-symtable test-semantic

# === DOCUMENTATION ===

SRC_DOC=dokumentace
SRC_DOC_PATH=doc/$(SRC_DOC)

doc: $(SRC_DOC_PATH).pdf

$(SRC_DOC_PATH).pdf: $(SRC_DOC_PATH).tex
	latex $(SRC_DOC_PATH).tex
	latex $(SRC_DOC_PATH).tex
	dvips -t a4 $(SRC_DOC).dvi
	ps2pdf -sPAPERSIZE=a4 $(SRC_DOC).ps
	mv $(SRC_DOC).* doc/

clean-doc:
	rm -f $(SRC_DOC_PATH).aux $(SRC_DOC_PATH).dvi $(SRC_DOC_PATH).log $(SRC_DOC_PATH).out $(SRC_DOC_PATH).ps

# === CLEAN ===
clean-all: clean clean-doc
	rm -f $(SRC_DOC_PATH).pdf
