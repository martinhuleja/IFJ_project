/*
 * IFJ + IAL projekt 2026/2027
 * Adam havier (xhaviea00)
*/

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stddef.h>
#include <stdbool.h>
#include "abstract-syntactic-tree.h"

/**
 * Writes string@ followed by the encoded bytes to stdout.
 * Does not append a newline. Text must point to length valid bytes.
*/
void codegen_string(const unsigned char *text, size_t length);
/** Writes an integer constant without a trailing newline. */
void codegen_int(long long value);
/** Writes a floating-point constant without a trailing newline. */
void codegen_float(double value);
/** Writes a Boolean constant without a trailing newline. */
void codegen_bool(bool value);

typedef enum {
    CODEGEN_GF,
    CODEGEN_LF,
    CODEGEN_TF
} CodegenFrame;

void codegen_variable(CodegenFrame frame, const char *name);
void codegen_defvar(CodegenFrame frame, const char *name);
bool codegen_var_declaration(CodegenFrame frame, const ASTNode *node);
bool codegen_assignment(CodegenFrame frame, const ASTNode *node);
bool codegen_block(CodegenFrame frame, const ASTNode *node);

void codegen_move_int(CodegenFrame frame, const char *name, long long value);
void codegen_move_var(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name);
bool codegen_move_literal(CodegenFrame frame, const char *name, const ASTNode *node);
void codegen_add_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value);

void codegen_write_var(CodegenFrame frame, const char *name);
void codegen_label(const char *name);
void codegen_jump(const char *name);
void codegen_jumpifeq_int(const char *label, CodegenFrame frame, const char *name, long long value);

void codegen_call(const char *label);
void codegen_return(void);
#endif