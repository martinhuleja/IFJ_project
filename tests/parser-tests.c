/*
 * Written with AI assistance (OpenAI Codex).
 * Relevant prompt: "You can start writing the test file for the parser.
 * Start very small with the basics."
 * Follow-up prompt: "Create tests for function definitions with multiple
 * functions, multiple parameters, and invalid definitions. Also test function
 * calls with zero, one and several arguments."
 *
 * These tests call parse_program() with the real lexer and expression parser.
 * Successful cases check the AST structure, names, types and argument values.
 * Invalid cases check syntax errors, not semantic errors such as undefined names.
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

static bool check_program(const ASTree *tree, size_t function_count) {
    return check(tree != NULL, "AST exists") &&
           check_node(tree->root, AST_PROGRAM, function_count,
                      "Program contains the expected number of functions");
}

static bool check_function(const ASTNode *function, const char *name,
                           ASTDataType return_type, bool is_main,
                           size_t parameter_count, size_t statement_count) {
    printf("Checking function: %s\n", name);
    return check_node(function, AST_FUNCTION, 2, "Function contains parameters and body") &&
           check(function->text != NULL && strcmp(function->text, name) == 0,
                 "Function name matches its position in the program") &&
           check(function->declared_type == return_type && function->is_main == is_main,
                 "Return type and @main annotation match") &&
           check_node(function->children[0], AST_PARAM_LIST, parameter_count,
                      "Parameter list has the expected number of parameters") &&
           check_node(function->children[1], AST_BLOCK, statement_count,
                      "Body has the expected number of statements");
}

static bool check_named_leaf(const ASTNode *node, ASTNodeType type,
                             const char *name, const char *description) {
    return check_node(node, type, 0, description) &&
           check(node->text != NULL && strcmp(node->text, name) == 0,
                 "Stored text matches the expected value");
}

static bool check_parameter(const ASTNode *parameter, const char *name, ASTDataType type) {
    printf("Checking parameter: %s\n", name);
    return check_named_leaf(parameter, AST_PARAM, name, "Parameter is a named leaf") &&
           check(parameter->declared_type == type, "Parameter has the expected declared type");
}

/* The original simple cases contain one parameterless Unit main with one statement. */
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

static bool check_multiple_functions(const ASTree *tree) {
    if (!check_program(tree, 3)) {
        return false;
    }
    const ASTNode *first = tree->root->children[0];
    const ASTNode *main = tree->root->children[1];
    const ASTNode *last = tree->root->children[2];
    if (!check_function(first, "first", AST_DATA_INT, false, 0, 1) ||
        !check_function(main, "main", AST_DATA_UNIT, true, 0, 1) ||
        !check_function(last, "last", AST_DATA_STRING, false, 0, 1)) {
        return false;
    }
    const ASTNode *number = first->children[1]->children[0];
    return check_node(number, AST_INT_LITERAL, 0, "First function returns an Int literal") &&
           check(number->int_value == 1, "First function returns 1") &&
           check_node(main->children[1]->children[0], AST_UNIT_LITERAL, 0,
                      "Middle function has a Unit body") &&
           check_named_leaf(last->children[1]->children[0], AST_STRING_LITERAL, "done",
                            "Last function returns the expected String");
}

static bool check_multiple_parameters(const ASTree *tree) {
    if (!check_program(tree, 2)) {
        return false;
    }
    const ASTNode *function = tree->root->children[0];
    const ASTNode *main = tree->root->children[1];
    if (!check_function(function, "mixed", AST_DATA_INT, false, 4, 1) ||
        !check_function(main, "main", AST_DATA_UNIT, true, 0, 1)) {
        return false;
    }
    const ASTNode *parameters = function->children[0];
    return check_parameter(parameters->children[0], "number", AST_DATA_INT) &&
           check_parameter(parameters->children[1], "ratio", AST_DATA_DOUBLE) &&
           check_parameter(parameters->children[2], "text", AST_DATA_STRING) &&
           check_parameter(parameters->children[3], "unit", AST_DATA_UNIT) &&
           check_named_leaf(function->children[1]->children[0], AST_IDENTIFIER, "number",
                            "Body refers to the first parameter") &&
           check_node(main->children[1]->children[0], AST_UNIT_LITERAL, 0,
                      "Main body remains separate from the preceding function");
}

static bool check_omitted_return_type(const ASTree *tree) {
    if (!check_program(tree, 2)) {
        return false;
    }
    const ASTNode *function = tree->root->children[0];
    const ASTNode *main = tree->root->children[1];
    if (!check_function(function, "identity", AST_DATA_UNKNOWN, false, 1, 1) ||
        !check_function(main, "main", AST_DATA_UNIT, true, 0, 1)) {
        return false;
    }
    return check_parameter(function->children[0]->children[0], "value", AST_DATA_INT) &&
           check_named_leaf(function->children[1]->children[0], AST_IDENTIFIER, "value",
                            "Body is the parameter reference") &&
           check_node(main->children[1]->children[0], AST_UNIT_LITERAL, 0,
                      "Main body is the Unit literal");
}

/* Call cases declare a helper followed by main. */
static const ASTNode *call_main_body(const ASTree *tree, size_t statement_count) {
    if (!check_program(tree, 2) ||
        !check_node(tree->root->children[0], AST_FUNCTION, 2, "Helper function is preserved")) {
        return NULL;
    }
    const ASTNode *main = tree->root->children[1];
    if (!check_function(main, "main", AST_DATA_UNIT, true, 0, statement_count)) {
        return NULL;
    }
    return main->children[1];
}

static const ASTNode *call_arguments(const ASTNode *call, const char *name, size_t argument_count) {
    printf("Checking call: %s (%zu arguments)\n", name, argument_count);
    if (!check_node(call, AST_CALL, 1, "Call has exactly one argument-list child") ||
        !check(call->text != NULL && strcmp(call->text, name) == 0, "Callee name matches") ||
        !check_node(call->children[0], AST_ARG_LIST, argument_count,
                    "Argument list has the expected number of arguments")) {
        return NULL;
    }
    return call->children[0];
}

static bool check_call_zero_arguments(const ASTree *tree) {
    const ASTNode *body = call_main_body(tree, 1);
    return body != NULL && call_arguments(body->children[0], "ping", 0) != NULL;
}

static bool check_identity_call(const ASTNode *call) {
    const ASTNode *arguments = call_arguments(call, "identity", 1);
    return arguments != NULL &&
           check_node(arguments->children[0], AST_INT_LITERAL, 0, "Argument is an Int literal") &&
           check(arguments->children[0]->int_value == 7, "Argument value is 7");
}

static bool check_call_one_argument(const ASTree *tree) {
    const ASTNode *body = call_main_body(tree, 1);
    return body != NULL && check_identity_call(body->children[0]);
}

static bool check_call_one_unit_argument(const ASTree *tree) {
    const ASTNode *body = call_main_body(tree, 1);
    if (body == NULL) {
        return false;
    }
    const ASTNode *arguments = call_arguments(body->children[0], "consume", 1);
    return arguments != NULL &&
           check_node(arguments->children[0], AST_UNIT_LITERAL, 0,
                      "Unit literal () is one argument, not an empty argument list");
}

static bool check_call_several_arguments(const ASTree *tree) {
    const ASTNode *body = call_main_body(tree, 2);
    if (body == NULL) {
        return false;
    }
    const ASTNode *declaration = body->children[0];
    if (!check_node(declaration, AST_VAR_DECLARATION, 1, "Declaration precedes the call") ||
        !check(declaration->text != NULL && strcmp(declaration->text, "number") == 0 &&
               !declaration->is_changeable && declaration->declared_type == AST_DATA_INT,
               "Declaration is immutable val number: Int") ||
        !check_node(declaration->children[0], AST_INT_LITERAL, 0, "Initializer is an Int literal") ||
        !check(declaration->children[0]->int_value == 7, "Initializer value is 7")) {
        return false;
    }
    const ASTNode *arguments = call_arguments(body->children[1], "consume", 4);
    if (arguments == NULL) {
        return false;
    }
    return check_named_leaf(arguments->children[0], AST_IDENTIFIER, "number",
                            "First argument is the variable reference") &&
           check_node(arguments->children[1], AST_DOUBLE_LITERAL, 0,
                      "Second argument is a Double literal") &&
           check(arguments->children[1]->double_value == 2.5, "Second argument value is 2.5") &&
           check_named_leaf(arguments->children[2], AST_STRING_LITERAL, "hello",
                            "Third argument is the expected String") &&
           check_node(arguments->children[3], AST_UNIT_LITERAL, 0, "Fourth argument is Unit");
}

static bool check_call_initializer(const ASTree *tree) {
    const ASTNode *body = call_main_body(tree, 1);
    if (body == NULL) {
        return false;
    }
    const ASTNode *declaration = body->children[0];
    return check_node(declaration, AST_VAR_DECLARATION, 1, "Declaration has one initializer") &&
           check(declaration->text != NULL && strcmp(declaration->text, "result") == 0 &&
                 !declaration->is_changeable && declaration->declared_type == AST_DATA_INT,
                 "Declaration is immutable val result: Int") &&
           check_identity_call(declaration->children[0]);
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
     ERROR_SYNTACTIC, NULL},

    /* Definitions: function order, parameter order/types and optional return type. */
    {"multiple_functions",
     "import ifj26.*\n"
     "def first(): Int = 1\n"
     "@main def main(): Unit = ()\n"
     "def last(): String = \"done\"\n",
     ERROR_OK, check_multiple_functions},
    {"multiple_parameters",
     "import ifj26.*\n"
     "def mixed(number: Int, ratio: Double, text: String, unit: Unit): Int = number\n"
     "@main def main(): Unit = ()\n",
     ERROR_OK, check_multiple_parameters},
    {"multiline_parameters_with_trailing_comma",
     "import ifj26.*\n"
     "def mixed(\n"
     "  number: Int,\n"
     "  ratio: Double,\n"
     "  text: String,\n"
     "  unit: Unit,\n"
     "  ): Int =\n"
     "  number\n"
     "@main\n"
     "def main(): Unit = ()\n",
     ERROR_OK, check_multiple_parameters},
    {"omitted_function_return_type",
     "import ifj26.*\n"
     "def identity(value: Int) = value\n"
     "@main def main(): Unit = ()\n",
     ERROR_OK, check_omitted_return_type},

    /* Calls: empty lists, individual values, mixed arguments and an initializer. */
    {"call_zero_arguments",
     "import ifj26.*\n"
     "def ping(): Unit = ()\n"
     "@main def main(): Unit =\n"
     "  ping()\n",
     ERROR_OK, check_call_zero_arguments},
    {"call_one_argument",
     "import ifj26.*\n"
     "def identity(value: Int): Int = value\n"
     "@main def main(): Unit =\n"
     "  identity(7)\n",
     ERROR_OK, check_call_one_argument},
    {"call_one_unit_argument",
     "import ifj26.*\n"
     "def consume(value: Unit): Unit = ()\n"
     "@main def main(): Unit =\n"
     "  consume(())\n",
     ERROR_OK, check_call_one_unit_argument},
    {"call_several_arguments",
     "import ifj26.*\n"
     "def consume(number: Int, ratio: Double, text: String, unit: Unit): Unit = ()\n"
     "@main def main(): Unit =\n"
     "  val number: Int = 7\n"
     "  consume(number, 2.5, \"hello\", ())\n",
     ERROR_OK, check_call_several_arguments},
    {"multiline_call_with_trailing_comma",
     "import ifj26.*\n"
     "def consume(number: Int, ratio: Double, text: String, unit: Unit): Unit = ()\n"
     "@main def main(): Unit =\n"
     "  val number: Int = 7\n"
     "  consume(\n"
     "    number,\n"
     "    2.5,\n"
     "    \"hello\",\n"
     "    (),\n"
     "    )\n",
     ERROR_OK, check_call_several_arguments},
    {"call_as_initializer",
     "import ifj26.*\n"
     "def identity(value: Int): Int = value\n"
     "@main def main(): Unit =\n"
     "  val result: Int = identity(7)\n",
     ERROR_OK, check_call_initializer},

    /* Invalid definitions are lexically valid but must return ERROR_SYNTACTIC. */
    {"definition_missing_name",
     "import ifj26.*\ndef (): Unit = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"definition_keyword_as_name",
     "import ifj26.*\ndef if(): Unit = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"definition_missing_opening_parenthesis",
     "import ifj26.*\ndef broken: Unit = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"definition_missing_closing_parenthesis",
     "import ifj26.*\ndef broken(value: Int = value\n",
     ERROR_SYNTACTIC, NULL},
    {"parameter_missing_name",
     "import ifj26.*\ndef broken(: Int): Unit = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"parameter_missing_colon",
     "import ifj26.*\ndef broken(value Int): Unit = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"parameter_missing_type",
     "import ifj26.*\ndef broken(value:): Unit = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"parameter_invalid_type",
     "import ifj26.*\ndef broken(value: Thing): Unit = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"parameters_missing_comma",
     "import ifj26.*\ndef broken(left: Int right: Int): Int = left\n",
     ERROR_SYNTACTIC, NULL},
    {"parameters_double_comma",
     "import ifj26.*\ndef broken(left: Int,, right: Int): Int = left\n",
     ERROR_SYNTACTIC, NULL},
    {"parameters_trailing_comma_without_newline",
     "import ifj26.*\ndef broken(value: Int,): Int = value\n",
     ERROR_SYNTACTIC, NULL},
    {"definition_missing_return_type",
     "import ifj26.*\ndef broken(): = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"definition_missing_equals",
     "import ifj26.*\ndef broken(): Unit ()\n",
     ERROR_SYNTACTIC, NULL},
    {"definition_missing_body",
     "import ifj26.*\ndef broken(): Unit =\n@main def main(): Unit = ()\n",
     ERROR_SYNTACTIC, NULL},
    {"invalid_definition_after_valid_functions",
     "import ifj26.*\n"
     "def first(): Int = 1\n"
     "@main def main(): Unit = ()\n"
     "def broken(left: Int right: Int): Int = left\n",
     ERROR_SYNTACTIC, NULL},
    {"nested_function_definition",
     "import ifj26.*\n@main def main(): Unit =\n  def nested(): Unit = ()\n",
     ERROR_SYNTACTIC, NULL},

    /* Invalid argument separators and incomplete calls. */
    {"call_leading_comma",
     "import ifj26.*\n@main def main(): Unit =\n  consume(, 1)\n",
     ERROR_SYNTACTIC, NULL},
    {"call_missing_comma",
     "import ifj26.*\n@main def main(): Unit =\n  consume(1 2)\n",
     ERROR_SYNTACTIC, NULL},
    {"call_double_comma",
     "import ifj26.*\n@main def main(): Unit =\n  consume(1,, 2)\n",
     ERROR_SYNTACTIC, NULL},
    {"call_trailing_comma_without_newline",
     "import ifj26.*\n@main def main(): Unit =\n  consume(1,)\n",
     ERROR_SYNTACTIC, NULL},
    {"call_comma_without_argument",
     "import ifj26.*\n@main def main(): Unit =\n  consume(,)\n",
     ERROR_SYNTACTIC, NULL},
    {"call_missing_closing_parenthesis",
     "import ifj26.*\n@main def main(): Unit =\n  consume(1\n",
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
