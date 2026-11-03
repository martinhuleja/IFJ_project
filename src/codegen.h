/*
 * IFJ + IAL projekt 2026/2027
 * Adam havier (xhaviea00)
*/

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Writes string@ followed by the encoded bytes to stdout.
 * Does not append a newline. Text must point to length valid bytes.
*/
void codegen_string(const unsigned char *text, size_t length);
/** Writes an integer constant without a trailing newline. */
void codegen_int(long long value);
/** Writes a folating-point constant without a trailing newline. */
void codegen_float(double value);
/** Writes a Boolean constant without a trailing newline. */
void codegen_bool(bool value);

#endif