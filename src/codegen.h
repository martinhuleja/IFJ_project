/*
 * IFJ + IAL projekt 2026/2027
 * Adam havier (xhaviea00)
*/

#ifndef CODEGEN_H
#define CODEGEN_H

#include <stddef.h>
#include <stdbool.h>
#include "abstract-syntactic-tree.h"

typedef enum {
    CODEGEN_GF,
    CODEGEN_LF,
    CODEGEN_TF
} CodegenFrame;

void codegen_string(const unsigned char *text, size_t length);
void codegen_int(long long value);
void codegen_float(double value);
void codegen_bool(bool value);

void codegen_variable(CodegenFrame frame, const char *name);
void codegen_defvar(CodegenFrame frame, const char *name);

void codegen_move_int(CodegenFrame frame, const char *name, long long value);
void codegen_move_var(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name);

void codegen_add_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value);
void codegen_sub_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value);
void codegen_mul_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value);
void codegen_idiv_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value);
void codegen_add_vars(CodegenFrame dest_frame, const char *dest_name, CodegenFrame lsource_frame, const char *lsource_name, CodegenFrame rsource_frame, const char *rsource_name);

void codegen_lt_int(CodegenFrame dest_frame, const char *dest_name, CodegenFrame source_frame, const char *source_name, long long value);

void codegen_write_var(CodegenFrame frame, const char *name);
void codegen_label(const char *name);
void codegen_jump(const char *name);
void codegen_jumpifeq_int(const char *label, CodegenFrame frame, const char *name, long long value);
void codegen_jumpifeq_bool(const char *label, CodegenFrame frame, const char *name, bool value);

void codegen_call(const char *label);
void codegen_return(void);

bool codegen_move_literal(CodegenFrame frame, const char *name, const ASTNode *node);
bool codegen_var_declaration(CodegenFrame frame, const ASTNode *node);
bool codegen_assignment(CodegenFrame frame, const ASTNode *node);
bool codegen_print(CodegenFrame frame, const ASTNode *node);
bool codegen_if(CodegenFrame frame, const ASTNode *node);
bool codegen_while(CodegenFrame frame, const ASTNode *node);
bool codegen_block(CodegenFrame frame, const ASTNode *node);
bool codegen_program(const ASTree *tree);

#endif