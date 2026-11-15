/*
 * IFJ + IAL projekt 2026/2027
 * Adam Havier (xhaviea00)
 */
#include "codegen.h"
#include <stdio.h>

int main(void)
{
    const CodegenFrame gf = CODEGEN_GF;
    const CodegenFrame lf = CODEGEN_LF;
    const CodegenFrame tf = CODEGEN_TF;
    const unsigned char text[] = "A B#\\\n";
    puts(".IFJcode26");

    printf("WRITE ");
    codegen_string(text, sizeof(text) - 1);
    putchar('\n');
    printf("WRITE ");
    codegen_int(1234567890123LL);
    puts("\nWRITE string@\\010");
    printf("WRITE ");
    codegen_float(2.5);
    puts("\nWRITE string@\\010");

    printf("WRITE ");
    codegen_bool(true);
    puts("\nWRITE string@\\010");
    printf("WRITE ");
    codegen_bool(false);
    puts("\nWRITE string@\\010");

    codegen_defvar(gf, "x");
    puts("CREATEFRAME");
    puts("PUSHFRAME");
    codegen_defvar(lf, "y");
    codegen_move_int(lf, "y", 42);
    codegen_move_var(gf, "x", lf, "y");
    codegen_add_int(lf, "y", gf, "x", 8);

    codegen_jumpifeq_int("vypis_vysledok", lf, "y", 50);
    puts("WRITE int@999");
    codegen_label("vypis_vysledok");
    codegen_write_var(lf, "y");
    puts("WRITE string@\\010");

    ASTNode zero = {
        .type = AST_INT_LITERAL,
        .int_value = 0
    };
    ASTNode *assignment_children[] = {&zero};
    char target[] = "y";
    const ASTNode assignment = {
        .type = AST_ASSIGN,
        .text = target,
        .children = assignment_children,
        .children_count = 1,
        .children_capacity = 1
    };
    if (!codegen_assignment(lf, &assignment)) {
        return 1;
    }
    codegen_label("zaciatok_cyklu");
    codegen_jumpifeq_int("koniec_cyklu", lf, "y", 3);
    codegen_add_int(lf, "y", lf, "y", 1);
    codegen_jump("zaciatok_cyklu");
    codegen_label("koniec_cyklu");
    codegen_write_var(lf, "y");
    puts("WRITE string@\\010");

    puts("CREATEFRAME");
    codegen_defvar(tf, "zdroj");
    codegen_move_int(tf, "zdroj", 42);
    ASTNode initializer = {
        .type = AST_INT_LITERAL,
        .int_value = 41
    };
    ASTNode *children[] = {&initializer};
    char name[] = "cislo";

    ASTNode declaration = {
        .type = AST_VAR_DECLARATION,
        .text = name,
        .children = children,
        .children_count = 1,
        .children_capacity = 1
    };
    char source[] = "zdroj";
    ASTNode value = {
        .type = AST_IDENTIFIER,
        .text = source
    };
    ASTNode *update_children[] = {&value};
    ASTNode update = {
        .type = AST_ASSIGN,
        .text = name,
        .children = update_children,
        .children_count = 1,
        .children_capacity = 1
    };
    ASTNode left = {
        .type = AST_IDENTIFIER,
        .text = name
    };
    ASTNode right = {
        .type = AST_INT_LITERAL,
        .int_value = 1
    };
    ASTNode *operands[] = {&left, &right};
    ASTNode addition = {
        .type = AST_BINARY_OP,
        .operator = AST_OP_ADD,
        .children = operands,
        .children_count = 2,
        .children_capacity = 2
    };
    ASTNode *increment_children[] = {&addition};
    ASTNode increment = {
        .type = AST_ASSIGN,
        .text = name,
        .children = increment_children,
        .children_count = 1,
        .children_capacity = 1
    };
    ASTNode *statements[] = {&declaration, &update, &increment};
    const ASTNode block = {
        .type = AST_BLOCK,
        .children = statements,
        .children_count = 3,
        .children_capacity = 3
    };
    if (!codegen_block(tf, &block)) {
        return 1;
    }
    codegen_call("vypis_cislo");
    codegen_write_var(tf, "cislo");
    puts("WRITE string@\\010");
    puts("WRITE int@8");
    puts("WRITE string@\\010");
    puts("POPFRAME");
    puts("EXIT int@0");

    codegen_label("vypis_cislo");
    puts("PUSHFRAME");
    codegen_add_int(lf, "cislo", lf, "cislo", 1);
    puts("POPFRAME");
    codegen_return();

    return 0;
}