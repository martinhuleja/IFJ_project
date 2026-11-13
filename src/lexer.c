#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "token.h"

typedef struct IndentNode {
    unsigned int spaces;
    struct IndentNode *next;
} IndentNode;

typedef struct {
    IndentNode *top;
} IndentStack;

typedef struct {
    IndentStack indent_stack;
    bool at_line_start;
    unsigned int leading_spaces;
    bool leading_tab;
    unsigned int pending_dedents;
    bool pending_divide;
    unsigned int parenthesis_depth;
    unsigned int comment_depth;
    int lookahead;
    bool has_lookahead;
    unsigned int line;
    unsigned int column;
    bool logical_line_has_token;
    bool lf_since_token;
    bool after_multiline_block_comment;
} LexerState;

typedef struct {
    char *data;
    size_t length;
    size_t capacity;
} CharBuffer;

static LexerState lexer = {.at_line_start = true, .line = 1, .column = 1};

void token_free(Token *token) {
    if (token == NULL) {
        return;
    }

    free(token->value);
    free(token);
}

void indent_stack_init(IndentStack *stack) {
    stack->top = NULL;
}

bool indent_stack_push(IndentStack *stack, unsigned int spaces) {
    IndentNode *node = malloc(sizeof(IndentNode));

    if (node == NULL) {
        return false;
    }

    node->spaces = spaces;
    node->next = stack->top;
    stack->top = node;

    return true;
}

unsigned int indent_stack_top(const IndentStack *stack) {
    if (stack->top == NULL) {
        return 0;
    }
    return stack->top->spaces;
}

void indent_stack_pop(IndentStack *stack) {
    if (stack->top == NULL) {
        return;
    }

    IndentNode *old = stack->top;
    stack->top = old->next;

    free(old);
}

void indent_stack_free(IndentStack *stack) {
    while (stack->top != NULL) {
        indent_stack_pop(stack);
    }
}

typedef enum {
    /* Non-finishing: no token has started. */
    ST_START,

    ST_B, ST_Bo, ST_Boo, ST_Bool, ST_Boole, ST_Boolea, ST_Boolean,
    ST_D,
    ST_Do,
    ST_Dou,
    ST_Doub,
    ST_Doubl,
    ST_Double,
    ST_I, ST_In, ST_Int,
    ST_S, ST_St, ST_Str, ST_Stri, ST_Strin, ST_String,
    ST_U, ST_Un, ST_Uni, ST_Unit,
    ST_d, ST_de, ST_def, ST_do,
    ST_e, ST_el, ST_els, ST_else,
    ST_f, ST_fo, ST_for,
    ST_i, ST_if, ST_im, ST_imp, ST_impo, ST_impor, ST_import,
    ST_r, ST_re, ST_ret, ST_retu, ST_retur, ST_return,
    ST_t, ST_th, ST_the, ST_then,
    ST_v, ST_va, ST_val, ST_var,
    ST_w, ST_wh, ST_whi, ST_whil, ST_while,

    ST_IDENTIFIER,

    ST_LEADING_UNDERSCORE,
    ST_AT, ST_AT_M, ST_AT_MA, ST_AT_MAI, /* non-finishing */
    ST_AT_MAIN,                          /* finishing */

    ST_GREATER, ST_GREATER_EQUAL,
    ST_LESS, ST_LESS_EQUAL,
    ST_BANG,       /* non-finishing */
    ST_NOT_EQUAL,  /* finishing */
    ST_MULTIPLY, ST_DIVIDE,
    ST_PLUS, ST_MINUS,
    ST_ASSIGN, ST_EQUAL,
    ST_DOT,
    ST_NEWLINE,
    ST_LPAREN, ST_RPAREN,
    ST_COLON, ST_COMMA, ST_SEMICOLON,

    ST_COMMENT,                 /* finishing */
    ST_BLOCK_COMMENT,           /* non-finishing */
    ST_BLOCK_COMMENT_SLASH,     /* non-finishing */
    ST_BLOCK_COMMENT_STAR,      /* non-finishing */
    ST_BLOCK_COMMENT_END,       /* finishing */

    ST_INT_ZERO, ST_INT_DIGITS,                  /* finishing */
    ST_INT_SEPARATOR,                           /* non-finishing */
    ST_DOUBLE_DOT,                              /* non-finishing */
    ST_DOUBLE_FRACTION,                         /* finishing */
    ST_DOUBLE_FRACTION_SEPARATOR,               /* non-finishing */
    ST_DOUBLE_EXPONENT, ST_DOUBLE_EXPONENT_SIGN, /* non-finishing */
    ST_DOUBLE_EXPONENT_DIGITS,                  /* finishing */
    ST_DOUBLE_EXPONENT_SEPARATOR,               /* non-finishing */
    ST_DOUBLE_SUFFIX,                           /* finishing */

    ST_STRING_OPEN, ST_STRING_BODY, ST_STRING_ESCAPE, /* non-finishing */
    ST_STRING_AFTER_ESCAPE,                          /* non-finishing */
    ST_STRING,                                        /* finishing */
    ST_STRING_EMPTY_OR_TRIPLE_PREFIX,                 /* finishing */
    ST_STRING_TRIPLE_BODY,                            /* non-finishing */
    ST_STRING_TRIPLE_QUOTE_1, ST_STRING_TRIPLE_QUOTE_2, /* non-finishing */
    ST_STRING_TRIPLE_END                              /* finishing */
} FSMState;

static bool is_identifier_continue(int c) {
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
           (c >= '0' && c <= '9') ||
           (c == '_');
}

static bool is_digit(int c) {
    return c >= '0' && c <= '9';
}

static int peek_char(void) {
    if (!lexer.has_lookahead) {
        lexer.lookahead = getchar();
        lexer.has_lookahead = true;
    }
    return lexer.lookahead;
}

static int take_char(void) {
    int c = peek_char();
    lexer.has_lookahead = false;

    if (c == '\n') {
        lexer.lf_since_token = true;
        lexer.line++;
        lexer.column = 1;
    } else if (c != EOF) {
        lexer.column++;
    }
    return c;
}

static void out_of_memory(void) {
    fputs("internal compiler error: out of memory\n", stderr);
    exit(EXIT_FAILURE);
}

static void buffer_append(CharBuffer *buffer, char c) {
    if (buffer->length + 1 >= buffer->capacity) {
        size_t new_capacity = buffer->capacity ? buffer->capacity * 2 : 32;
        if (new_capacity <= buffer->capacity) {
            out_of_memory();
        }
        char *new_data = realloc(buffer->data, new_capacity);
        if (new_data == NULL) {
            out_of_memory();
        }
        buffer->data = new_data;
        buffer->capacity = new_capacity;
    }

    buffer->data[buffer->length++] = c;
    buffer->data[buffer->length] = '\0';
}

static Token *make_token(TokenType type, char *value,
                         unsigned int line, unsigned int column) {
    Token *token = malloc(sizeof(*token));
    if (token == NULL) {
        out_of_memory();
    }
    token->type = type;
    token->value = value;
    token->lf_before_rparen = type == TOKEN_RPAREN && lexer.lf_since_token;
    lexer.lf_since_token = false;
    token->line = line;
    token->column = column;
    if (type != TOKEN_NEWLINE && type != TOKEN_EOF && type != TOKEN_INDENT && type != TOKEN_DEDENT) {
        lexer.logical_line_has_token = true;
    }
    return token;
}

static bool prepare_indentation(Token **indent_token,
                                unsigned int line, unsigned int column,
                                int first_char) {
    *indent_token = NULL;
    if (!lexer.at_line_start) {
        return true;
    }
    if (lexer.leading_tab) {
        return false;
    }

    if (lexer.parenthesis_depth > 0) {
        /* The stack top is the indentation of this logical line. */
        if (first_char != ')' &&
            lexer.leading_spaces <= indent_stack_top(&lexer.indent_stack)) {
            return false;
        }
        lexer.at_line_start = false;
        return true;
    }

    lexer.at_line_start = false;
    unsigned int current = indent_stack_top(&lexer.indent_stack);
    if (lexer.leading_spaces > current) {
        if (!indent_stack_push(&lexer.indent_stack, lexer.leading_spaces)) {
            out_of_memory();
        }
        *indent_token = make_token(TOKEN_INDENT, NULL, line, column);
        return true;
    }
    if (lexer.leading_spaces == current) {
        return true;
    }

    unsigned int dedent_count = 0;
    IndentNode *level = lexer.indent_stack.top;
    while (level != NULL && level->spaces > lexer.leading_spaces) {
        dedent_count++;
        level = level->next;
    }
    unsigned int matching_level = 0;
    if (level != NULL) {
        matching_level = level->spaces;
    }
    if (matching_level != lexer.leading_spaces) {
        return false;
    }

    for (unsigned int i = 0; i < dedent_count; i++) {
        indent_stack_pop(&lexer.indent_stack);
    }
    lexer.pending_dedents = dedent_count - 1;
    *indent_token = make_token(TOKEN_DEDENT, NULL, line, column);
    return true;
}

static Token *scan_string(unsigned int line, unsigned int column) {
    CharBuffer buffer = {0};
    FSMState state = ST_STRING_OPEN;

    take_char(); 
    buffer_append(&buffer, '\0'); 
    buffer.length = 0;

    while (true) {
        int c = peek_char();

        switch (state) {
            case ST_STRING_OPEN:
            case ST_STRING_BODY:
            case ST_STRING_AFTER_ESCAPE:
                if (c == EOF || c == '\n') goto invalid_string;
                if (c == '"') {
                    take_char();
                    state = state == ST_STRING_OPEN
                                ? ST_STRING_EMPTY_OR_TRIPLE_PREFIX : ST_STRING;
                } else if (c == '\\') {
                    take_char();
                    state = ST_STRING_ESCAPE;
                } else {
                    buffer_append(&buffer, (char)take_char());
                    state = ST_STRING_BODY;
                }
                break;

            case ST_STRING_ESCAPE:
                switch (c) {
                    case 'n': c = '\n'; break;
                    case 't': c = '\t'; break;
                    case 'r': c = '\r'; break;
                    case '\\': break;
                    case '"': break;
                    default: goto invalid_string;
                }
                take_char();
                buffer_append(&buffer, (char)c);
                state = ST_STRING_AFTER_ESCAPE;
                break;

            case ST_STRING_EMPTY_OR_TRIPLE_PREFIX:
                if (c != '"') {
                    return make_token(TOKEN_STRING_LITERAL, buffer.data,
                                      line, column);
                }
                take_char();
                state = ST_STRING_TRIPLE_BODY;
                break;

            case ST_STRING_TRIPLE_BODY:
                if (c == EOF) goto invalid_string;
                if (c == '"') {
                    take_char();
                    state = ST_STRING_TRIPLE_QUOTE_1;
                } else {
                    buffer_append(&buffer, (char)take_char());
                }
                break;

            case ST_STRING_TRIPLE_QUOTE_1:
                if (c == EOF) goto invalid_string;
                if (c == '"') {
                    take_char();
                    state = ST_STRING_TRIPLE_QUOTE_2;
                } else {
                    buffer_append(&buffer, '"');
                    state = ST_STRING_TRIPLE_BODY;
                }
                break;

            case ST_STRING_TRIPLE_QUOTE_2:
                if (c == EOF) goto invalid_string;
                if (c == '"') {
                    take_char();
                    state = ST_STRING_TRIPLE_END;
                } else {
                    buffer_append(&buffer, '"');
                    buffer_append(&buffer, '"');
                    state = ST_STRING_TRIPLE_BODY;
                }
                break;

            case ST_STRING:
            case ST_STRING_TRIPLE_END:
                return make_token(TOKEN_STRING_LITERAL, buffer.data,
                                  line, column);

            default:
                goto invalid_string;
        }
    }

invalid_string:
    free(buffer.data);
    return NULL;
}

static bool skip_block_comment(unsigned int *newline_line, unsigned int *newline_column) {
    FSMState state = ST_BLOCK_COMMENT;
    lexer.comment_depth = 1;

    while (state != ST_BLOCK_COMMENT_END) {
        int c = peek_char();
        if (c == EOF) {
            return false;
        }

        if (c == '\n' && *newline_line == 0) {
            *newline_line = lexer.line;
            *newline_column = lexer.column;
        }
        take_char();

        switch (state) {
            case ST_BLOCK_COMMENT:
                if (c == '/') {
                    state = ST_BLOCK_COMMENT_SLASH;
                } else if (c == '*') {
                    state = ST_BLOCK_COMMENT_STAR;
                }
                break;

            case ST_BLOCK_COMMENT_SLASH:
                if (c == '*') {
                    lexer.comment_depth++;
                    state = ST_BLOCK_COMMENT;
                } else if (c == '/') {
                    state = ST_BLOCK_COMMENT_SLASH;
                } else {
                    state = ST_BLOCK_COMMENT;
                }
                break;

            case ST_BLOCK_COMMENT_STAR:
                if (c == '/') {
                    lexer.comment_depth--;
                    if (lexer.comment_depth == 0) {
                        state = ST_BLOCK_COMMENT_END;
                    } else {
                        state = ST_BLOCK_COMMENT;
                    }
                } else if (c == '*') {
                    state = ST_BLOCK_COMMENT_STAR;
                } else {
                    state = ST_BLOCK_COMMENT;
                }
                break;

            default:
                return false;
        }
    }
    return true;
}

static Token *scan_number(FSMState state, CharBuffer buffer, unsigned int line, unsigned int column) {
    while (true) {
        int c = peek_char();
        FSMState next = state;
        bool advance = false;
        bool append = true;

        switch (state) {
            case ST_INT_ZERO:
                if (c == '.') next = ST_DOUBLE_DOT;
                else if (c == 'e' || c == 'E') next = ST_DOUBLE_EXPONENT;
                else break;
                advance = true;
                break;
            case ST_INT_DIGITS:
                if (is_digit(c)) next = ST_INT_DIGITS;
                else if (c == '_') {
                    next = ST_INT_SEPARATOR;
                    append = false;
                } else if (c == '.') next = ST_DOUBLE_DOT;
                else if (c == 'e' || c == 'E') next = ST_DOUBLE_EXPONENT;
                else break;
                advance = true;
                break;
            case ST_INT_SEPARATOR:
                if (is_digit(c)) {
                    next = ST_INT_DIGITS;
                    advance = true;
                }
                break;
            case ST_DOUBLE_DOT:
                if (is_digit(c)) {
                    next = ST_DOUBLE_FRACTION;
                    advance = true;
                }
                break;
            case ST_DOUBLE_FRACTION:
                if (is_digit(c)) next = ST_DOUBLE_FRACTION;
                else if (c == '_') {
                    next = ST_DOUBLE_FRACTION_SEPARATOR;
                    append = false;
                } else if (c == 'e' || c == 'E') next = ST_DOUBLE_EXPONENT;
                else if (c == 'd' || c == 'D') {
                    next = ST_DOUBLE_SUFFIX;
                    append = false;
                } else break;
                advance = true;
                break;
            case ST_DOUBLE_FRACTION_SEPARATOR:
                if (is_digit(c)) {
                    next = ST_DOUBLE_FRACTION;
                    advance = true;
                }
                break;
            case ST_DOUBLE_EXPONENT:
                if (is_digit(c)) next = ST_DOUBLE_EXPONENT_DIGITS;
                else if (c == '+' || c == '-') next = ST_DOUBLE_EXPONENT_SIGN;
                else break;
                advance = true;
                break;
            case ST_DOUBLE_EXPONENT_SIGN:
                if (is_digit(c)) {
                    next = ST_DOUBLE_EXPONENT_DIGITS;
                    advance = true;
                }
                break;
            case ST_DOUBLE_EXPONENT_DIGITS:
                if (is_digit(c)) next = ST_DOUBLE_EXPONENT_DIGITS;
                else if (c == '_') {
                    next = ST_DOUBLE_EXPONENT_SEPARATOR;
                    append = false;
                } else if (c == 'd' || c == 'D') {
                    next = ST_DOUBLE_SUFFIX;
                    append = false;
                } else break;
                advance = true;
                break;
            case ST_DOUBLE_EXPONENT_SEPARATOR:
                if (is_digit(c)) {
                    next = ST_DOUBLE_EXPONENT_DIGITS;
                    advance = true;
                }
                break;
            case ST_DOUBLE_SUFFIX:
                break;
            default:
                break;
        }

        if (!advance) break;
        take_char();
        if (append) buffer_append(&buffer, (char)c);
        state = next;
    }

    TokenType type;
    if (state == ST_INT_ZERO || state == ST_INT_DIGITS) {
        type = TOKEN_INT_LITERAL;
    } else if (state == ST_DOUBLE_FRACTION ||
               state == ST_DOUBLE_EXPONENT_DIGITS ||
               state == ST_DOUBLE_SUFFIX) {
        type = TOKEN_DOUBLE_LITERAL;
    } else {
        free(buffer.data);
        return NULL;
    }

    if (is_identifier_continue(peek_char())) {
        free(buffer.data);
        return NULL;
    }
    return make_token(type, buffer.data, line, column);
}

static Token *scan_symbol(unsigned int line, unsigned int column) {
    CharBuffer buffer = {0};
    int c = take_char();
    TokenType type;

    buffer_append(&buffer, (char)c);
    switch (c) {
        case '>':
            type = TOKEN_GREATER;
            if (peek_char() == '=') type = TOKEN_GREATER_EQUAL;
            break;
        case '<':
            type = TOKEN_LESS;
            if (peek_char() == '=') type = TOKEN_LESS_EQUAL;
            break;
        case '=':
            type = TOKEN_ASSIGN;
            if (peek_char() == '=') type = TOKEN_EQUAL;
            break;
        case '!':
            if (peek_char() != '=') {
                free(buffer.data);
                return NULL;
            }
            type = TOKEN_NOT_EQUAL;
            break;
        case '+': type = TOKEN_PLUS; break;
        case '-': type = TOKEN_MINUS; break;
        case '*': type = TOKEN_MULTIPLY; break;
        case '(':
            type = TOKEN_LPAREN;
            lexer.parenthesis_depth++;
            break;
        case ')':
            type = TOKEN_RPAREN;
            if (lexer.parenthesis_depth > 0) {
                lexer.parenthesis_depth--;
            }
            break;
        case ':': type = TOKEN_COLON; break;
        case ',': type = TOKEN_COMMA; break;
        case ';': type = TOKEN_SEMICOLON; break;
        case '.':
            if (is_digit(peek_char())) {
                return scan_number(ST_DOUBLE_DOT, buffer, line, column);
            }
            type = TOKEN_DOT;
            break;
        default:
            free(buffer.data);
            return NULL;
    }

    if (type == TOKEN_GREATER_EQUAL || type == TOKEN_LESS_EQUAL ||
        type == TOKEN_EQUAL || type == TOKEN_NOT_EQUAL) {
        buffer_append(&buffer, (char)take_char());
    }
    return make_token(type, buffer.data, line, column);
}

static TokenType word_type(FSMState state) {
    switch (state) {
        case ST_Double: return TOKEN_DOUBLE;
        case ST_Int: return TOKEN_INT;
        case ST_String: return TOKEN_STRING;
        case ST_Unit: return TOKEN_UNIT;
        case ST_def: return TOKEN_DEF;
        case ST_do: return TOKEN_DO;
        case ST_else: return TOKEN_ELSE;
        case ST_if: return TOKEN_IF;
        case ST_import: return TOKEN_IMPORT;
        case ST_return: return TOKEN_RETURN;
        case ST_then: return TOKEN_THEN;
        case ST_val: return TOKEN_VAL;
        case ST_var: return TOKEN_VAR;
        case ST_while: return TOKEN_WHILE;
        default: return TOKEN_IDENTIFIER;
    }
}

Token *get_next_token(void) {
    FSMState state = ST_START;
    bool scanning_word = false;
    bool counting_leading_spaces = true;
    CharBuffer buffer = {0};
    unsigned int token_line = 0;
    unsigned int token_column = 0;
    int c;

    if (lexer.pending_dedents > 0) {
        lexer.pending_dedents--;
        unsigned int column = lexer.column;
        if (lexer.pending_divide) {
            /* The slash was consumed before the dedents were returned. */
            column--;
        }
        return make_token(TOKEN_DEDENT, NULL, lexer.line, column);
    }
    if (lexer.pending_divide) {
        lexer.pending_divide = false;
        buffer_append(&buffer, '/');
        return make_token(TOKEN_DIVIDE, buffer.data,
                          lexer.line, lexer.column - 1);
    }

    while (true) {
        c = peek_char();

        /* Numeric, string, and comment states need their own end rules. */
        if (scanning_word && !is_identifier_continue(c)) {
            if (state == ST_LEADING_UNDERSCORE) {
                free(buffer.data);
                return NULL;
            }
            return make_token(word_type(state), buffer.data,
                              token_line, token_column);
        }

        if (state == ST_AT_MAIN) {
            if (is_identifier_continue(c)) {
                free(buffer.data);
                return NULL;
            }
            return make_token(TOKEN_MAIN_ANNOTATION, buffer.data,
                              token_line, token_column);
        }

        if (state == ST_DIVIDE) {
            if (c == '/') {
                take_char();
                state = ST_COMMENT;
                continue;
            }
            if (c == '*') {
                unsigned int newline_line = 0;
                unsigned int newline_column = 0;
                take_char();
                if (!skip_block_comment(&newline_line, &newline_column)) {
                    return NULL;
                }
                counting_leading_spaces = false;
                state = ST_START;
                if (newline_line != 0) {
                    lexer.after_multiline_block_comment = true;
                    lexer.at_line_start = true;
                    lexer.leading_spaces = 0;
                    lexer.leading_tab = false;
                    if (lexer.logical_line_has_token &&
                        lexer.parenthesis_depth == 0) {
                        lexer.logical_line_has_token = false;
                        return make_token(TOKEN_NEWLINE, NULL,
                                          newline_line, newline_column);
                    }
                }
                continue;
            }
            if (lexer.after_multiline_block_comment) {
                return NULL;
            }
            Token *indent_token = NULL;
            if (!prepare_indentation(&indent_token,
                                     token_line, token_column, '/')) {
                return NULL;
            }
            if (indent_token != NULL) {
                /* The slash was consumed while checking for a comment. */
                lexer.pending_divide = true;
                return indent_token;
            }
            buffer_append(&buffer, '/');
            return make_token(TOKEN_DIVIDE, buffer.data,
                              token_line, token_column);
        }

        if (state == ST_COMMENT) {
            if (c == '\n' || c == EOF) {
                state = ST_START; 
            } else {
                take_char();
            }
            continue;
        }

        if (state == ST_START) {
            if (c == ' ') {
                if (lexer.at_line_start && counting_leading_spaces) {
                    lexer.leading_spaces++;
                }
                take_char();
                continue;
            }
            if (c == '\t') {
                if (lexer.at_line_start && counting_leading_spaces) {
                    lexer.leading_tab = true;
                }
                take_char();
                continue;
            }
            if (c == '\r') {
                take_char();
                continue;
            }
            if (c == '\n') {
                unsigned int line = lexer.line;
                unsigned int column = lexer.column;
                take_char();
                lexer.after_multiline_block_comment = false;
                lexer.at_line_start = true;
                lexer.leading_spaces = 0;
                lexer.leading_tab = false;
                counting_leading_spaces = true;
                if (lexer.parenthesis_depth > 0) {
                    continue;
                }
                if (!lexer.logical_line_has_token) {
                    continue;
                }
                lexer.logical_line_has_token = false;
                return make_token(TOKEN_NEWLINE, NULL, line, column);
            }
            if (c == EOF) {
                if (lexer.indent_stack.top != NULL) {
                    indent_stack_pop(&lexer.indent_stack);
                    return make_token(TOKEN_DEDENT, NULL,
                                      lexer.line, lexer.column);
                }
                return make_token(TOKEN_EOF, NULL, lexer.line, lexer.column);
            }
            if (lexer.after_multiline_block_comment && c != '/') {
                return NULL;
            }
            token_line = lexer.line;
            token_column = lexer.column;
            if (c == '/') {
                take_char();
                state = ST_DIVIDE;
                continue;
            }
            Token *indent_token = NULL;
            if (!prepare_indentation(&indent_token,
                                     token_line, token_column, c)) {
                return NULL;
            }
            if (indent_token != NULL) {
                return indent_token;
            }
            if (is_digit(c)) {
                state = c == '0' ? ST_INT_ZERO : ST_INT_DIGITS;
                buffer_append(&buffer, (char)take_char());
                return scan_number(state, buffer, token_line, token_column);
            }
            if (c == '@') {
                state = ST_AT;
                buffer_append(&buffer, (char)take_char());
                continue;
            }
            if (c == '"') {
                return scan_string(token_line, token_column);
            }
            if (!((c >= 'a' && c <= 'z') ||
                  (c >= 'A' && c <= 'Z') || c == '_')) {
                return scan_symbol(token_line, token_column);
            }
            scanning_word = true;
        }

        switch (state) {
            case ST_START:
                if (c == 'B') state = ST_B;
                else if (c == 'D') state = ST_D;
                else if (c == 'I') state = ST_I;
                else if (c == 'S') state = ST_S;
                else if (c == 'U') state = ST_U;
                else if (c == 'd') state = ST_d;
                else if (c == 'e') state = ST_e;
                else if (c == 'f') state = ST_f;
                else if (c == 'i') state = ST_i;
                else if (c == 'r') state = ST_r;
                else if (c == 't') state = ST_t;
                else if (c == 'v') state = ST_v;
                else if (c == 'w') state = ST_w;
                else if (c == '_') state = ST_LEADING_UNDERSCORE;
                else if ((c >= 'a' && c <= 'z') ||
                         (c >= 'A' && c <= 'Z')) {
                    state = ST_IDENTIFIER;
                }
                break;
            case ST_LEADING_UNDERSCORE:
                if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_AT:
                if (c != 'm') {
                    free(buffer.data);
                    return NULL;
                }
                state = ST_AT_M;
                break;
            case ST_AT_M:
                if (c != 'a') {
                    free(buffer.data);
                    return NULL;
                }
                state = ST_AT_MA;
                break;
            case ST_AT_MA:
                if (c != 'i') {
                    free(buffer.data);
                    return NULL;
                }
                state = ST_AT_MAI;
                break;
            case ST_AT_MAI:
                if (c != 'n') {
                    free(buffer.data);
                    return NULL;
                }
                state = ST_AT_MAIN;
                break;
            case ST_D:
                if (c == 'o') {
                    state = ST_Do;
                    break;
                }
                else if (is_identifier_continue(c)) {
                    state = ST_IDENTIFIER;
                    break;
                }
                break;
            case ST_Do:
                if (c == 'u') {
                    state = ST_Dou;
                    break;
                }
                else if (is_identifier_continue(c)) {
                    state = ST_IDENTIFIER;
                    break;
                }
                break;
            case ST_Dou:
                if (c=='b') {
                    state = ST_Doub;
                    break;
                }
                else if (is_identifier_continue(c)) {
                    state = ST_IDENTIFIER;
                    break;
                }
                break;
            case ST_Doub:
                if (c=='l') {
                    state = ST_Doubl;
                    break;
                }
                else if (is_identifier_continue(c)) {
                    state = ST_IDENTIFIER;
                    break;
                }
                break;
            case ST_Doubl:
                if (c=='e') {
                    state = ST_Double;
                    break;
                }
                else if (is_identifier_continue(c)) {
                    state = ST_IDENTIFIER;
                    break;
                }
                break;
            case ST_Double:
                if (is_identifier_continue(c)) {
                    state = ST_IDENTIFIER;
                    break;
                }
                break;
            case ST_B:
                if (c == 'o') state = ST_Bo;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Bo:
                if (c == 'o') state = ST_Boo;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Boo:
                if (c == 'l') state = ST_Bool;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Bool:
                if (c == 'e') state = ST_Boole;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Boole:
                if (c == 'a') state = ST_Boolea;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Boolea:
                if (c == 'n') state = ST_Boolean;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_I:
                if (c == 'n') state = ST_In;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_In:
                if (c == 't') state = ST_Int;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_S:
                if (c == 't') state = ST_St;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_St:
                if (c == 'r') state = ST_Str;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Str:
                if (c == 'i') state = ST_Stri;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Stri:
                if (c == 'n') state = ST_Strin;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Strin:
                if (c == 'g') state = ST_String;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_U:
                if (c == 'n') state = ST_Un;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Un:
                if (c == 'i') state = ST_Uni;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Uni:
                if (c == 't') state = ST_Unit;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_d:
                if (c == 'e') state = ST_de;
                else if (c == 'o') state = ST_do;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_de:
                if (c == 'f') state = ST_def;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_e:
                if (c == 'l') state = ST_el;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_el:
                if (c == 's') state = ST_els;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_els:
                if (c == 'e') state = ST_else;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_f:
                if (c == 'o') state = ST_fo;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_fo:
                if (c == 'r') state = ST_for;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_i:
                if (c == 'f') state = ST_if;
                else if (c == 'm') state = ST_im;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_im:
                if (c == 'p') state = ST_imp;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_imp:
                if (c == 'o') state = ST_impo;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_impo:
                if (c == 'r') state = ST_impor;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_impor:
                if (c == 't') state = ST_import;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_r:
                if (c == 'e') state = ST_re;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_re:
                if (c == 't') state = ST_ret;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_ret:
                if (c == 'u') state = ST_retu;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_retu:
                if (c == 'r') state = ST_retur;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_retur:
                if (c == 'n') state = ST_return;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_t:
                if (c == 'h') state = ST_th;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_th:
                if (c == 'e') state = ST_the;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_the:
                if (c == 'n') state = ST_then;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_v:
                if (c == 'a') state = ST_va;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_va:
                if (c == 'l') state = ST_val;
                else if (c == 'r') state = ST_var;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_w:
                if (c == 'h') state = ST_wh;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_wh:
                if (c == 'i') state = ST_whi;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_whi:
                if (c == 'l') state = ST_whil;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_whil:
                if (c == 'e') state = ST_while;
                else if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_Boolean:
            case ST_Int:
            case ST_String:
            case ST_Unit:
            case ST_def:
            case ST_do:
            case ST_else:
            case ST_for:
            case ST_if:
            case ST_import:
            case ST_return:
            case ST_then:
            case ST_val:
            case ST_var:
            case ST_while:
                if (is_identifier_continue(c)) state = ST_IDENTIFIER;
                break;
            case ST_IDENTIFIER:
                if (is_identifier_continue(c)) {
                    state = ST_IDENTIFIER;
                    break;
                }
                break;
            default:
                free(buffer.data);
                return NULL;
        }

        if (state == ST_START) {
            free(buffer.data);
            return NULL;
        }
        buffer_append(&buffer, (char)take_char());
    }
}
