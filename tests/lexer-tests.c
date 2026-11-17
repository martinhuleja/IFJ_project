/*
 * Written with AI assistance (OpenAI Codex).
 * Relevant prompt: "Generate me some unit tests for lexer part. Start small.
 * Maybe one file lexer-tests.c and one script to run all the tests.
 * Make sure to test every state's return token for start."
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
    {.type = TOKEN_STRING_LITERAL, .value = ""},
    {.type = TOKEN_STRING_LITERAL, .value = "A\nB\tC\rD\\E\"F"},
    {.type = TOKEN_STRING_LITERAL, .value = "G\\nH\nI"},
    {.type = TOKEN_IDENTIFIER, .value = "x", .line = 2, .column = 5},
    {.type = TOKEN_EOF}
};

static const ExpectedToken line_comment_tokens[] = {
    {.type = TOKEN_IDENTIFIER, .value = "value", .line = 2, .column = 1},
    {.type = TOKEN_NEWLINE, .line = 2, .column = 14},
    {.type = TOKEN_EOF}
};

static const ExpectedToken triple_quote_tokens[] = {
    {.type = TOKEN_STRING_LITERAL, .value = ""},
    {.type = TOKEN_STRING_LITERAL, .value = "a\"b\"\"c"},
    {.type = TOKEN_EOF}
};

static const ExpectedToken adjacent_tokens[] = {
    {.type = TOKEN_IDENTIFIER, .value = "a"},
    {.type = TOKEN_EQUAL, .value = "=="},
    {.type = TOKEN_IDENTIFIER, .value = "b"},
    {.type = TOKEN_EOF}
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
    {.type = TOKEN_IDENTIFIER, .value = "a"},
    {.type = TOKEN_NEWLINE},
    {.type = TOKEN_INDENT},
    {.type = TOKEN_IDENTIFIER, .value = "b"},
    {.type = TOKEN_NEWLINE},
    {.type = TOKEN_INDENT},
    {.type = TOKEN_IDENTIFIER, .value = "c"},
    {.type = TOKEN_NEWLINE},
    {.type = TOKEN_DEDENT},
    {.type = TOKEN_IDENTIFIER, .value = "d"},
    {.type = TOKEN_DEDENT},
    {.type = TOKEN_EOF}
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
    {"unfinished_string", "\"unfinished", NULL, ERROR_LEXICAL},
    {"unfinished_block_comment", "/* outer /* inner */", NULL, ERROR_LEXICAL},
    {"leading_tab", "\tname", NULL, ERROR_LEXICAL},
    {"incomplete_exponent", "1e+", NULL, ERROR_LEXICAL},
    {"unfinished_bang", "!", NULL, ERROR_LEXICAL},
    {"unfinished_identifier", "_", NULL, ERROR_LEXICAL}
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

static bool check_token(const Token *actual, const ExpectedToken *expected, size_t index) {
    if (actual->type != expected->type) {
        fprintf(stderr, "Token %zu: expected type %d, got %d\n",
                index, expected->type, actual->type);
        return false;
    }
    if ((actual->value == NULL) != (expected->value == NULL) ||
        (actual->value != NULL && strcmp(actual->value, expected->value) != 0)) {
        fprintf(stderr, "Token %zu: expected value [%s], got [%s]\n", index,
                expected->value == NULL ? "NULL" : expected->value,
                actual->value == NULL ? "NULL" : actual->value);
        return false;
    }
    if (actual->lf_before_rparen != expected->lf_before_rparen) {
        fprintf(stderr, "Token %zu: incorrect lf_before_rparen\n", index);
        return false;
    }
    /* Zero means the case does not check this position component. */
    if ((expected->line != 0 && actual->line != expected->line) ||
        (expected->column != 0 && actual->column != expected->column)) {
        fprintf(stderr, "Token %zu: expected position %u:%u, got %u:%u\n",
                index, expected->line, expected->column, actual->line, actual->column);
        return false;
    }
    return true;
}

static bool run_test(const TestCase *test) {
    if (!set_input(test->source)) {
        return false;
    }
    if (test->error != ERROR_OK) {
        Token token = {0};
        ErrorCode error = get_next_token(&token);
        free(token.value);
        if (error != test->error) {
            fprintf(stderr, "Expected error %d, got %d\n", test->error, error);
            return false;
        }
        return true;
    }

    for (size_t i = 0; ; i++) {
        Token token = {0};
        ErrorCode error = get_next_token(&token);
        if (error != ERROR_OK) {
            fprintf(stderr, "Token %zu: unexpected error %d\n", i, error);
            free(token.value);
            return false;
        }
        bool matches = check_token(&token, &test->tokens[i], i);
        free(token.value);
        if (!matches) {
            return false;
        }
        if (token.type == TOKEN_EOF) {
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
