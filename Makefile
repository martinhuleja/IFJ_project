# IFJ + IAL projekt 2026/2027
# Makefile
# Martin Huleja (xhulejm00)
# Adam Havier (xhaviea00)
# Richard Komora (xkomorr00)
# Enzo Štětina (xstetie00)

CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Werror -pedantic
TARGET = translator
SRC = src/translator.c

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

