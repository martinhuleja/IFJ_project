/*
 * Written with AI assistance (OpenAI Codex).
 * Relevant prompt: "You can start writing the test file for the parser.
 * Start very small with the basics."
 *
 * These tests call parse_program() with the real lexer and expression parser.
 * Successful cases check a few AST properties; invalid cases check the error.
 * Each case runs in a fresh process because the lexer keeps static state.
 */
#define _POSIX_C_SOURCE 200809L

#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    const char *name;
    const char *source;
    ErrorCode expected_error;
    bool (*check_ast)(const ASTree *tree);
} TestCase;

static bool check(bool condition, const char *description) {
    printf("%s: %s\n", condition ? "PASS" : "FAIL", description);
    return condition;
}

static bool check_node(const ASTNode *node, ASTNodeType type,
                       size_t children_count, const char *description) {
    return check(node != NULL && node->type == type &&
                 node->children_count == children_count &&
                 (children_count == 0 || node->children != NULL), description);
}

/* Both valid programs contain one parameterless Unit main with one statement. */
static const ASTNode *main_statement(const ASTree *tree) {
    const ASTNode *program = tree->root;
    if (!check_node(program, AST_PROGRAM, 1, "Program contains exactly one function")) {
        return NULL;
    }
    const ASTNode *function = program->children[0];
    if (!check_node(function, AST_FUNCTION, 2, "Function contains parameters and body")) {
        return NULL;
    }
    if (!check(function->text != NULL && strcmp(function->text, "main") == 0 &&
               function->is_main && function->declared_type == AST_DATA_UNIT,
               "Function is named main, has @main, and declares Unit")) {
        return NULL;
    }
    if (!check_node(function->children[0], AST_PARAM_LIST, 0, "Parameter list is empty")) {
        return NULL;
    }
    const ASTNode *block = function->children[1];
    if (!check_node(block, AST_BLOCK, 1, "Body contains exactly one statement")) {
        return NULL;
    }
    return block->children[0];
}

static bool check_minimal_main(const ASTree *tree) {
    const ASTNode *statement = main_statement(tree);
    if (statement == NULL) {
        return false;
    }
    return check_node(statement, AST_UNIT_LITERAL, 0, "Body is the Unit literal ()");
}

static bool check_val_addition(const ASTree *tree) {
    const ASTNode *declaration = main_statement(tree);
    if (declaration == NULL) {
        return false;
    }
    if (!check_node(declaration, AST_VAR_DECLARATION, 1, "Declaration has one initializer")) {
        return false;
    }
    if (!check(declaration->text != NULL && strcmp(declaration->text, "x") == 0 &&
               !declaration->is_changeable && declaration->declared_type == AST_DATA_INT,
               "Declaration is immutable val x with declared type Int")) {
        return false;
    }
    const ASTNode *addition = declaration->children[0];
    if (!check_node(addition, AST_BINARY_OP, 2, "Initializer is a binary expression")) {
        return false;
    }
    if (!check(addition->operator == AST_OP_ADD, "Expression uses addition")) {
        return false;
    }
    const ASTNode *left = addition->children[0];
    const ASTNode *right = addition->children[1];
    if (!check_node(left, AST_INT_LITERAL, 0, "Left operand is an Int literal") ||
        !check_node(right, AST_INT_LITERAL, 0, "Right operand is an Int literal")) {
        return false;
    }
    return check(left->int_value == 2 && right->int_value == 3,
                 "Operands are 2 and 3 in source order");
}

static const TestCase tests[] = {
    {"minimal_main",
     "import ifj26.*\n@main def main(): Unit = ()\n",
     ERROR_OK, check_minimal_main},
    {"val_addition",
     "import ifj26.*\n@main def main(): Unit =\n  val x: Int = 2 + 3\n",
     ERROR_OK, check_val_addition},
    {"missing_import",
     "@main def main(): Unit = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"missing_initializer",
     "import ifj26.*\n@main def main(): Unit =\n  val x: Int =\n",
     ERROR_SYNTACTIC, NULL}
};

static bool set_input(const char *source) {
    FILE *input = tmpfile();
    if (input == NULL) {
        perror("tmpfile");
        return false;
    }
    if (fputs(source, input) == EOF || fflush(input) != 0) {
        perror("writing test input");
        fclose(input);
        return false;
    }
    rewind(input);
    if (dup2(fileno(input), STDIN_FILENO) == -1) {
        perror("redirecting stdin");
        fclose(input);
        return false;
    }
    fclose(input);
    return true;
}

static const char *error_name(ErrorCode error) {
    switch (error) {
        case ERROR_OK: return "ERROR_OK";
        case ERROR_LEXICAL: return "ERROR_LEXICAL";
        case ERROR_SYNTACTIC: return "ERROR_SYNTACTIC";
        case ERROR_INTERNAL: return "ERROR_INTERNAL";
        default: return "UNEXPECTED_ERROR";
    }
}

static bool run_test(const TestCase *test) {
    printf("Input:\n%s", test->source);
    if (!set_input(test->source)) {
        return false;
    }
    ASTree *tree = ast_init();
    if (tree == NULL) {
        puts("FAIL: could not allocate the AST");
        return false;
    }

    ErrorCode error = parse_program(tree);
    printf("Expected result: %s (%d)\n", error_name(test->expected_error), test->expected_error);
    printf("Actual result:   %s (%d)\n", error_name(error), error);
    bool passed = error == test->expected_error;
    if (passed && test->check_ast != NULL) {
        passed = test->check_ast(tree);
    }
    ast_free(tree);
    return passed;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s --list | TEST_NAME\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "--list") == 0) {
        for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
            puts(tests[i].name);
        }
        return EXIT_SUCCESS;
    }
    for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        if (strcmp(argv[1], tests[i].name) == 0) {
            if (run_test(&tests[i])) {
                return EXIT_SUCCESS;
            }
            return EXIT_FAILURE;
        }
    }
    fprintf(stderr, "Unknown test: %s\n", argv[1]);
    return EXIT_FAILURE;
}
