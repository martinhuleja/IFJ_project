# IFJ + IAL projekt 2026/2027
# Makefile
# Martin Huleja (xhulejm00)
# Adam Havier (xhaviea00)
# Richard Komora (xkomorr00)
# Enzo Štětina (xstetie00)

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic
TARGET = translator
SRC = src/translator.c src/symtable.c

# Clang-format configuration
FORMAT = clang-format
FORMAT_FLAGS = --dry-run -Werror

.PHONY: all build check build-check format clean

all: build

# Compiles the target binary
build: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $^ -o $@

# Runs check
check:
	$(FORMAT) $(FORMAT_FLAGS) $(SRC)
	# tests/...

# Runs check first then builds the binary
build-check: check build

# Formats source files
format:
	$(FORMAT) -i $(SRC)

clean:
	rm -f $(TARGET)

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
