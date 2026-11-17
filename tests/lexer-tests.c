/*
 * Written with AI assistance (OpenAI Codex).
 * Relevant prompt: "Generate me some unit tests for lexer part. Start small.
 * Maybe one file lexer-tests.c and one script to run all the tests.
 * Make sure to test every state's return token for start."
 * Follow-up prompt: "maybe on the write out have it more detailed"
 * Follow-up prompt: "Negative tests only check the first token. Add more
 * invalid-input cases: invalid numeric underscores, leading zeros, invalid
 * string escapes, incorrect dedentation, and insufficient indentation inside
 * parentheses."
 * Follow-up prompt: "Add exact positions to selected tests, especially around
 * comments and multiline strings, and check whether positions are checked
 * correctly. You do not need an obscene number of them."
 *
 * Scope: token returns from accepting FSM states, checked through the public
 * get_next_token() API. Comment states skip text; unfinished states return
 * ERROR_LEXICAL. Boolean and for currently finish as identifiers.
 */
#define _POSIX_C_SOURCE 200809L

#include "token.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    TokenType type;
    const char *value;
    bool lf_before_rparen;
    unsigned int line;
    unsigned int column;
} ExpectedToken;

typedef struct {
    const char *name;
    const char *source;
    const ExpectedToken *tokens; /* Successful cases end with TOKEN_EOF. */
    ErrorCode error;
} TestCase;

static const ExpectedToken empty_tokens[] = {
    {.type = TOKEN_EOF, .line = 1, .column = 1}
};

static const ExpectedToken word_tokens[] = {
    {.type = TOKEN_VAL, .value = "val"},
    {.type = TOKEN_IDENTIFIER, .value = "value"},
    {.type = TOKEN_ASSIGN, .value = "="},
    {.type = TOKEN_IDENTIFIER, .value = "returnValue"},
    {.type = TOKEN_IDENTIFIER, .value = "_name"},
    {.type = TOKEN_IDENTIFIER, .value = "__"},
    {.type = TOKEN_MAIN_ANNOTATION, .value = "@main"},
    {.type = TOKEN_EOF}
};

static const ExpectedToken keyword_tokens[] = {
    {.type = TOKEN_DEF, .value = "def"},
    {.type = TOKEN_DO, .value = "do"},
    {.type = TOKEN_DOUBLE, .value = "Double"},
    {.type = TOKEN_ELSE, .value = "else"},
    {.type = TOKEN_IF, .value = "if"},
    {.type = TOKEN_IMPORT, .value = "import"},
    {.type = TOKEN_INT, .value = "Int"},
    {.type = TOKEN_RETURN, .value = "return"},
    {.type = TOKEN_STRING, .value = "String"},
    {.type = TOKEN_THEN, .value = "then"},
    {.type = TOKEN_UNIT, .value = "Unit"},
    {.type = TOKEN_VAL, .value = "val"},
    {.type = TOKEN_VAR, .value = "var"},
    {.type = TOKEN_WHILE, .value = "while"},
    {.type = TOKEN_EOF}
};

/* Every accepting keyword-prefix state must also finish as an identifier. */
static const ExpectedToken prefix_tokens[] = {
    {.type = TOKEN_IDENTIFIER, .value = "B"},
    {.type = TOKEN_IDENTIFIER, .value = "Bo"},
    {.type = TOKEN_IDENTIFIER, .value = "Boo"},
    {.type = TOKEN_IDENTIFIER, .value = "Bool"},
    {.type = TOKEN_IDENTIFIER, .value = "Boole"},
    {.type = TOKEN_IDENTIFIER, .value = "Boolea"},
    {.type = TOKEN_IDENTIFIER, .value = "Boolean"},
    {.type = TOKEN_IDENTIFIER, .value = "D"},
    {.type = TOKEN_IDENTIFIER, .value = "Do"},
    {.type = TOKEN_IDENTIFIER, .value = "Dou"},
    {.type = TOKEN_IDENTIFIER, .value = "Doub"},
    {.type = TOKEN_IDENTIFIER, .value = "Doubl"},
    {.type = TOKEN_IDENTIFIER, .value = "I"},
    {.type = TOKEN_IDENTIFIER, .value = "In"},
    {.type = TOKEN_IDENTIFIER, .value = "S"},
    {.type = TOKEN_IDENTIFIER, .value = "St"},
    {.type = TOKEN_IDENTIFIER, .value = "Str"},
    {.type = TOKEN_IDENTIFIER, .value = "Stri"},
    {.type = TOKEN_IDENTIFIER, .value = "Strin"},
    {.type = TOKEN_IDENTIFIER, .value = "U"},
    {.type = TOKEN_IDENTIFIER, .value = "Un"},
    {.type = TOKEN_IDENTIFIER, .value = "Uni"},
    {.type = TOKEN_IDENTIFIER, .value = "d"},
    {.type = TOKEN_IDENTIFIER, .value = "de"},
    {.type = TOKEN_IDENTIFIER, .value = "e"},
    {.type = TOKEN_IDENTIFIER, .value = "el"},
    {.type = TOKEN_IDENTIFIER, .value = "els"},
    {.type = TOKEN_IDENTIFIER, .value = "f"},
    {.type = TOKEN_IDENTIFIER, .value = "fo"},
    {.type = TOKEN_IDENTIFIER, .value = "for"},
    {.type = TOKEN_IDENTIFIER, .value = "i"},
    {.type = TOKEN_IDENTIFIER, .value = "im"},
    {.type = TOKEN_IDENTIFIER, .value = "imp"},
    {.type = TOKEN_IDENTIFIER, .value = "impo"},
    {.type = TOKEN_IDENTIFIER, .value = "impor"},
    {.type = TOKEN_IDENTIFIER, .value = "r"},
    {.type = TOKEN_IDENTIFIER, .value = "re"},
    {.type = TOKEN_IDENTIFIER, .value = "ret"},
    {.type = TOKEN_IDENTIFIER, .value = "retu"},
    {.type = TOKEN_IDENTIFIER, .value = "retur"},
    {.type = TOKEN_IDENTIFIER, .value = "t"},
    {.type = TOKEN_IDENTIFIER, .value = "th"},
    {.type = TOKEN_IDENTIFIER, .value = "the"},
    {.type = TOKEN_IDENTIFIER, .value = "v"},
    {.type = TOKEN_IDENTIFIER, .value = "va"},
    {.type = TOKEN_IDENTIFIER, .value = "w"},
    {.type = TOKEN_IDENTIFIER, .value = "wh"},
    {.type = TOKEN_IDENTIFIER, .value = "whi"},
    {.type = TOKEN_IDENTIFIER, .value = "whil"},
    {.type = TOKEN_EOF}
};

static const ExpectedToken operator_tokens[] = {
    {.type = TOKEN_ASSIGN, .value = "="},
    {.type = TOKEN_EQUAL, .value = "=="},
    {.type = TOKEN_NOT_EQUAL, .value = "!="},
    {.type = TOKEN_DIVIDE, .value = "/"},
    {.type = TOKEN_DOT, .value = "."},
    {.type = TOKEN_LPAREN, .value = "("},
    {.type = TOKEN_RPAREN, .value = ")"},
    {.type = TOKEN_GREATER, .value = ">"},
    {.type = TOKEN_GREATER_EQUAL, .value = ">="},
    {.type = TOKEN_LESS, .value = "<"},
    {.type = TOKEN_LESS_EQUAL, .value = "<="},
    {.type = TOKEN_PLUS, .value = "+"},
    {.type = TOKEN_MINUS, .value = "-"},
    {.type = TOKEN_MULTIPLY, .value = "*"},
    {.type = TOKEN_COLON, .value = ":"},
    {.type = TOKEN_COMMA, .value = ","},
    {.type = TOKEN_SEMICOLON, .value = ";"},
    {.type = TOKEN_EOF}
};

static const ExpectedToken number_tokens[] = {
    {.type = TOKEN_INT_LITERAL, .value = "0"},
    {.type = TOKEN_INT_LITERAL, .value = "12"},
    {.type = TOKEN_INT_LITERAL, .value = "1000"},
    {.type = TOKEN_DOUBLE_LITERAL, .value = ".5"},
    {.type = TOKEN_DOUBLE_LITERAL, .value = "0.5"},
    {.type = TOKEN_DOUBLE_LITERAL, .value = "1.23"},
    {.type = TOKEN_DOUBLE_LITERAL, .value = "0e5"},
    {.type = TOKEN_DOUBLE_LITERAL, .value = "1E-2"},
    {.type = TOKEN_DOUBLE_LITERAL, .value = "3e+4"},
    {.type = TOKEN_DOUBLE_LITERAL, .value = "5e10"},
    {.type = TOKEN_DOUBLE_LITERAL, .value = "2.5e+2"},
    {.type = TOKEN_DOUBLE_LITERAL, .value = "1.0"},
    {.type = TOKEN_EOF}
};

static const ExpectedToken string_tokens[] = {
    {.type = TOKEN_STRING_LITERAL, .value = "", .line = 1, .column = 1},
    {.type = TOKEN_STRING_LITERAL, .value = "A\nB\tC\rD\\E\"F", .line = 1, .column = 4},
    {.type = TOKEN_STRING_LITERAL, .value = "G\\nH\nI", .line = 1, .column = 23},
    {.type = TOKEN_IDENTIFIER, .value = "x", .line = 2, .column = 5},
    {.type = TOKEN_EOF, .line = 2, .column = 6}
};

static const ExpectedToken line_comment_tokens[] = {
    {.type = TOKEN_IDENTIFIER, .value = "value", .line = 2, .column = 1},
    {.type = TOKEN_NEWLINE, .line = 2, .column = 14},
    {.type = TOKEN_EOF, .line = 3, .column = 9}
};

static const ExpectedToken triple_quote_tokens[] = {
    {.type = TOKEN_STRING_LITERAL, .value = ""},
    {.type = TOKEN_STRING_LITERAL, .value = "a\"b\"\"c"},
    {.type = TOKEN_EOF}
};

static const ExpectedToken adjacent_tokens[] = {
    {.type = TOKEN_IDENTIFIER, .value = "a", .line = 1, .column = 1},
    {.type = TOKEN_EQUAL, .value = "==", .line = 1, .column = 2},
    {.type = TOKEN_IDENTIFIER, .value = "b", .line = 1, .column = 4},
    {.type = TOKEN_EOF, .line = 1, .column = 5}
};

static const ExpectedToken block_comment_tokens[] = {
    {.type = TOKEN_IDENTIFIER, .value = "a"},
    {.type = TOKEN_IDENTIFIER, .value = "b"},
    {.type = TOKEN_NEWLINE},
    {.type = TOKEN_INDENT},
    {.type = TOKEN_IDENTIFIER, .value = "c"},
    {.type = TOKEN_NEWLINE},
    {.type = TOKEN_IDENTIFIER, .value = "d", .line = 3, .column = 3},
    {.type = TOKEN_DEDENT},
    {.type = TOKEN_EOF}
};

static const ExpectedToken indentation_tokens[] = {
    {.type = TOKEN_IDENTIFIER, .value = "a", .line = 1, .column = 1},
    {.type = TOKEN_NEWLINE, .line = 1, .column = 2},
    {.type = TOKEN_INDENT, .line = 3, .column = 3},
    {.type = TOKEN_IDENTIFIER, .value = "b", .line = 3, .column = 3},
    {.type = TOKEN_NEWLINE, .line = 3, .column = 4},
    {.type = TOKEN_INDENT, .line = 5, .column = 5},
    {.type = TOKEN_IDENTIFIER, .value = "c", .line = 5, .column = 5},
    {.type = TOKEN_NEWLINE, .line = 5, .column = 6},
    {.type = TOKEN_DEDENT, .line = 6, .column = 3},
    {.type = TOKEN_IDENTIFIER, .value = "d", .line = 6, .column = 3},
    {.type = TOKEN_DEDENT, .line = 6, .column = 4},
    {.type = TOKEN_EOF, .line = 6, .column = 4}
};

/* The newline token points to the first LF inside the outer comment. */
static const ExpectedToken multiline_comment_position_tokens[] = {
    {.type = TOKEN_IDENTIFIER, .value = "a", .line = 1, .column = 1},
    {.type = TOKEN_NEWLINE, .line = 1, .column = 10},
    {.type = TOKEN_IDENTIFIER, .value = "b", .line = 5, .column = 1},
    {.type = TOKEN_EOF, .line = 5, .column = 2}
};

/* Internal quotes remain string content; only physical LF advances the line. */
static const ExpectedToken multiline_string_position_tokens[] = {
    {.type = TOKEN_STRING_LITERAL, .value = "one\n\"two\"\"\nthree", .line = 1, .column = 1},
    {.type = TOKEN_IDENTIFIER, .value = "next", .line = 3, .column = 10},
    {.type = TOKEN_NEWLINE, .line = 3, .column = 14},
    {.type = TOKEN_IDENTIFIER, .value = "last", .line = 4, .column = 1},
    {.type = TOKEN_EOF, .line = 4, .column = 5}
};

static const ExpectedToken parenthesis_tokens[] = {
    {.type = TOKEN_IDENTIFIER, .value = "f"},
    {.type = TOKEN_LPAREN, .value = "("},
    {.type = TOKEN_INT_LITERAL, .value = "1"},
    {.type = TOKEN_COMMA, .value = ","},
    {.type = TOKEN_RPAREN, .value = ")", .lf_before_rparen = true},
    {.type = TOKEN_NEWLINE},
    {.type = TOKEN_IDENTIFIER, .value = "f"},
    {.type = TOKEN_LPAREN, .value = "("},
    {.type = TOKEN_INT_LITERAL, .value = "2"},
    {.type = TOKEN_COMMA, .value = ","},
    {.type = TOKEN_RPAREN, .value = ")"},
    {.type = TOKEN_EOF}
};

static const TestCase tests[] = {
    {"empty_input", "", empty_tokens, ERROR_OK},
    {"keywords_and_identifiers", "val value = returnValue _name __ @main", word_tokens, ERROR_OK},
    {"all_keywords", "def do Double else if import Int return String then Unit val var while", keyword_tokens, ERROR_OK},
    {"all_keyword_prefix_states",
     "B Bo Boo Bool Boole Boolea Boolean D Do Dou Doub Doubl I In S St Str Stri Strin U Un Uni "
     "d de e el els f fo for i im imp impo impor r re ret retu retur t th the v va w wh whi whil",
     prefix_tokens, ERROR_OK},
    {"operators", "= == != / . ( ) > >= < <= + - * : , ;", operator_tokens, ERROR_OK},
    {"adjacent_tokens", "a==b", adjacent_tokens, ERROR_OK},
    {"numbers", "0 12 1_000 .5 0.5 1.2_3 0e5 1E-2 3e+4 5e1_0 2.5e+2D 1.0d", number_tokens, ERROR_OK},
    {"strings", "\"\" \"A\\nB\\tC\\rD\\\\E\\\"F\" \"\"\"G\\nH\nI\"\"\"x", string_tokens, ERROR_OK},
    {"triple_quote_states", "\"\"\"\"\"\" \"\"\"a\"b\"\"c\"\"\"", triple_quote_tokens, ERROR_OK},
    {"line_comments", "// heading\nvalue // tail\n// final", line_comment_tokens, ERROR_OK},
    {"block_comments", "a/* outer /* inner */ outer */b\n  /* comment */   c\n  d", block_comment_tokens, ERROR_OK},
    {"indentation", "a\n  // ignored\n  b\n\n    c\n  d", indentation_tokens, ERROR_OK},
    {"parenthesis_newlines", "f(\n  1, // comment\n  )\nf(\n  2,)", parenthesis_tokens, ERROR_OK},
    {"multiline_comment_positions", "a/* outer\n /* inner\n */ outer\n */\nb", multiline_comment_position_tokens, ERROR_OK},
    {"multiline_string_positions", "\"\"\"one\n\"two\"\"\nthree\"\"\" next\nlast", multiline_string_position_tokens, ERROR_OK},
    {"unfinished_string", "\"unfinished", NULL, ERROR_LEXICAL},
    {"unfinished_block_comment", "/* outer /* inner */", NULL, ERROR_LEXICAL},
    {"leading_tab", "\tname", NULL, ERROR_LEXICAL},
    {"incomplete_exponent", "1e+", NULL, ERROR_LEXICAL},
    {"unfinished_bang", "!", NULL, ERROR_LEXICAL},
    {"unfinished_identifier", "_", NULL, ERROR_LEXICAL},
    {"double_numeric_underscore", "1__2", NULL, ERROR_LEXICAL},
    {"double_numeric_underscore_after_tokens", "val x = 1__2", NULL, ERROR_LEXICAL},
    {"trailing_numeric_underscore", "val x = 1_", NULL, ERROR_LEXICAL},
    {"double_fraction_underscore", "val x = 1.2__3", NULL, ERROR_LEXICAL},
    {"underscore_after_exponent", "val x = 1e_2", NULL, ERROR_LEXICAL},
    {"underscore_after_exponent_sign", "val x = 1e+_2", NULL, ERROR_LEXICAL},
    {"double_exponent_underscore", "val x = 1e2__3", NULL, ERROR_LEXICAL},
    {"leading_zero", "0123", NULL, ERROR_LEXICAL},
    {"leading_zero_after_tokens", "val x = 0123", NULL, ERROR_LEXICAL},
    {"repeated_leading_zero", "val x = 000", NULL, ERROR_LEXICAL},
    {"invalid_string_escape", "\"hello\\q\"", NULL, ERROR_LEXICAL},
    {"invalid_string_escape_after_tokens", "val s = \"hello\\q\"", NULL, ERROR_LEXICAL},
    {"newline_in_ordinary_string", "val s = \"hello\nworld\"", NULL, ERROR_LEXICAL},
    {"unfinished_string_after_tokens", "val s = \"unfinished", NULL, ERROR_LEXICAL},
    {"unfinished_triple_string_after_tokens", "val s = \"\"\"unfinished", NULL, ERROR_LEXICAL},
    {"unfinished_comment_after_tokens", "val x = 1 /* outer /* inner */", NULL, ERROR_LEXICAL},
    {"incorrect_dedent", "a\n    b\n  c", NULL, ERROR_LEXICAL},
    {"incorrect_nested_dedent", "a\n  b\n    c\n   d", NULL, ERROR_LEXICAL},
    {"leading_tab_after_tokens", "a\n\tb", NULL, ERROR_LEXICAL},
    {"insufficient_parenthesis_indentation", "f(\n1\n )", NULL, ERROR_LEXICAL},
    {"insufficient_parenthesis_indentation_in_block", "a\n  f(\n  1\n    )", NULL, ERROR_LEXICAL},
    {"insufficient_closing_parenthesis_indentation", "f(\n  1\n)", NULL, ERROR_LEXICAL},
    {"tab_inside_parentheses", "f(\n\t1\n )", NULL, ERROR_LEXICAL},
    {"invalid_main_suffix_after_tokens", "val x = 1\n@mainfoo", NULL, ERROR_LEXICAL},
    {"invalid_character_after_tokens", "val x = $", NULL, ERROR_LEXICAL}
};

/* Each case runs in a new process so the lexer's static state starts fresh. */
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

static const char *token_name(TokenType type) {
    switch (type) {
        case TOKEN_EOF: return "TOKEN_EOF";
        case TOKEN_NEWLINE: return "TOKEN_NEWLINE";
        case TOKEN_INDENT: return "TOKEN_INDENT";
        case TOKEN_DEDENT: return "TOKEN_DEDENT";
        case TOKEN_IDENTIFIER: return "TOKEN_IDENTIFIER";
        case TOKEN_INT_LITERAL: return "TOKEN_INT_LITERAL";
        case TOKEN_DOUBLE_LITERAL: return "TOKEN_DOUBLE_LITERAL";
        case TOKEN_STRING_LITERAL: return "TOKEN_STRING_LITERAL";
        case TOKEN_DEF: return "TOKEN_DEF";
        case TOKEN_DO: return "TOKEN_DO";
        case TOKEN_DOUBLE: return "TOKEN_DOUBLE";
        case TOKEN_ELSE: return "TOKEN_ELSE";
        case TOKEN_IF: return "TOKEN_IF";
        case TOKEN_IMPORT: return "TOKEN_IMPORT";
        case TOKEN_INT: return "TOKEN_INT";
        case TOKEN_RETURN: return "TOKEN_RETURN";
        case TOKEN_STRING: return "TOKEN_STRING";
        case TOKEN_THEN: return "TOKEN_THEN";
        case TOKEN_UNIT: return "TOKEN_UNIT";
        case TOKEN_VAL: return "TOKEN_VAL";
        case TOKEN_VAR: return "TOKEN_VAR";
        case TOKEN_WHILE: return "TOKEN_WHILE";
        case TOKEN_ASSIGN: return "TOKEN_ASSIGN";
        case TOKEN_PLUS: return "TOKEN_PLUS";
        case TOKEN_MINUS: return "TOKEN_MINUS";
        case TOKEN_MULTIPLY: return "TOKEN_MULTIPLY";
        case TOKEN_DIVIDE: return "TOKEN_DIVIDE";
        case TOKEN_EQUAL: return "TOKEN_EQUAL";
        case TOKEN_NOT_EQUAL: return "TOKEN_NOT_EQUAL";
        case TOKEN_LESS: return "TOKEN_LESS";
        case TOKEN_LESS_EQUAL: return "TOKEN_LESS_EQUAL";
        case TOKEN_GREATER: return "TOKEN_GREATER";
        case TOKEN_GREATER_EQUAL: return "TOKEN_GREATER_EQUAL";
        case TOKEN_LPAREN: return "TOKEN_LPAREN";
        case TOKEN_RPAREN: return "TOKEN_RPAREN";
        case TOKEN_COLON: return "TOKEN_COLON";
        case TOKEN_COMMA: return "TOKEN_COMMA";
        case TOKEN_SEMICOLON: return "TOKEN_SEMICOLON";
        case TOKEN_DOT: return "TOKEN_DOT";
        case TOKEN_MAIN_ANNOTATION: return "TOKEN_MAIN_ANNOTATION";
        default: return "UNKNOWN_TOKEN";
    }
}

static const char *error_name(ErrorCode error) {
    switch (error) {
        case ERROR_OK: return "ERROR_OK";
        case ERROR_LEXICAL: return "ERROR_LEXICAL";
        case ERROR_INTERNAL: return "ERROR_INTERNAL";
        default: return "UNEXPECTED_ERROR";
    }
}

/* Keep newlines, tabs and quotes visible without breaking the report layout. */
static void print_text(const char *text) {
    if (text == NULL) {
        fputs("NULL", stdout);
        return;
    }
    putchar('"');
    for (const unsigned char *p = (const unsigned char *)text; *p != '\0'; p++) {
        switch (*p) {
            case '\n': fputs("\\n", stdout); break;
            case '\t': fputs("\\t", stdout); break;
            case '\r': fputs("\\r", stdout); break;
            case '\\': fputs("\\\\", stdout); break;
            case '"': fputs("\\\"", stdout); break;
            default:
                if (*p >= 32 && *p <= 126) {
                    putchar(*p);
                } else {
                    printf("\\x%02X", (unsigned int)*p);
                }
                break;
        }
    }
    putchar('"');
}

static void print_actual_token(const Token *token) {
    printf("  actual:   %s, value=", token_name(token->type));
    print_text(token->value);
    printf(", position=%u:%u, lf_before_rparen=%s\n", token->line, token->column,
           token->lf_before_rparen ? "true" : "false");
}

static bool positions_match(const Token *actual, const ExpectedToken *expected) {
    /* Zero means the case does not check this position component. */
    return (expected->line == 0 || actual->line == expected->line) &&
           (expected->column == 0 || actual->column == expected->column);
}

/* Test the position comparison used by every token assertion. */
static bool run_position_checker_test(void) {
    const Token actual = {.line = 2, .column = 5};
    const struct {
        const char *name;
        unsigned int line;
        unsigned int column;
        bool should_match;
    } checks[] = {
        {"exact match", 2, 5, true},
        {"wrong line", 3, 5, false},
        {"wrong column", 2, 6, false},
        {"both positions unchecked", 0, 0, true},
        {"line checked, column unchecked", 2, 0, true},
        {"line unchecked, column checked", 0, 5, true},
        {"wrong line with unchecked column", 3, 0, false},
        {"wrong column with unchecked line", 0, 6, false}
    };

    puts("Checking the test framework's position comparison (0 means unchecked).");
    for (size_t i = 0; i < sizeof(checks) / sizeof(checks[0]); i++) {
        const ExpectedToken expected = {.line = checks[i].line, .column = checks[i].column};
        bool matches = positions_match(&actual, &expected);
        printf("%s: actual=2:5, expected=%u:%u, expected match=%s, actual match=%s\n",
               checks[i].name, expected.line, expected.column,
               checks[i].should_match ? "true" : "false", matches ? "true" : "false");
        if (matches != checks[i].should_match) {
            puts("FAIL: position comparison returned the wrong result.");
            return false;
        }
    }
    return true;
}

static bool check_token(const Token *actual, const ExpectedToken *expected, size_t index) {
    bool type_matches = actual->type == expected->type;
    bool value_matches = actual->value == NULL && expected->value == NULL;
    if (actual->value != NULL && expected->value != NULL) {
        value_matches = strcmp(actual->value, expected->value) == 0;
    }
    bool flag_matches = actual->lf_before_rparen == expected->lf_before_rparen;
    bool position_matches = positions_match(actual, expected);
    bool matches = type_matches && value_matches && flag_matches && position_matches;

    printf("Token %zu: %s\n", index + 1, matches ? "PASS" : "FAIL");
    printf("  expected: %s, value=", token_name(expected->type));
    print_text(expected->value);
    fputs(", position=", stdout);
    if (expected->line == 0) {
        putchar('*');
    } else {
        printf("%u", expected->line);
    }
    putchar(':');
    if (expected->column == 0) {
        putchar('*');
    } else {
        printf("%u", expected->column);
    }
    printf(", lf_before_rparen=%s\n", expected->lf_before_rparen ? "true" : "false");

    print_actual_token(actual);

    if (!type_matches) {
        puts("  Mismatch: token type");
    }
    if (!value_matches) {
        puts("  Mismatch: token value");
    }
    if (!flag_matches) {
        puts("  Mismatch: lf_before_rparen");
    }
    if (!position_matches) {
        puts("  Mismatch: source position");
    }
    return matches;
}

static bool run_test(const TestCase *test) {
    fputs("Input: ", stdout);
    print_text(test->source);
    printf("\nExpected result: %s (%d)\n", error_name(test->error), test->error);
    if (!set_input(test->source)) {
        return false;
    }
    if (test->error != ERROR_OK) {
        for (size_t i = 0; ; i++) {
            Token token = {0};
            ErrorCode error = get_next_token(&token);
            if (error != ERROR_OK) {
                free(token.value);
                printf("Actual result:   %s (%d) after %zu successful tokens\n",
                       error_name(error), error, i);
                return error == test->error;
            }

            printf("Token %zu: read before expected error\n", i + 1);
            print_actual_token(&token);
            free(token.value);
            if (token.type == TOKEN_EOF) {
                puts("FAIL: reached EOF without the expected error.");
                return false;
            }
        }
    }

    for (size_t i = 0; ; i++) {
        Token token = {0};
        ErrorCode error = get_next_token(&token);
        if (error != ERROR_OK) {
            printf("Token %zu: FAIL, unexpected %s (%d)\n", i + 1, error_name(error), error);
            free(token.value);
            return false;
        }
        bool matches = check_token(&token, &test->tokens[i], i);
        free(token.value);
        if (!matches) {
            return false;
        }
        if (token.type == TOKEN_EOF) {
            printf("Checked tokens: %zu (including EOF).\n", i + 1);
            return true;
        }
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s --list | TEST_NAME\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "--list") == 0) {
        puts("position_checker");
        for (size_t i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
            puts(tests[i].name);
        }
        return EXIT_SUCCESS;
    }
    if (strcmp(argv[1], "position_checker") == 0) {
        if (run_position_checker_test()) {
            return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
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
