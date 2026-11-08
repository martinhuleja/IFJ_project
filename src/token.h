#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_EOF,
    TOKEN_NEWLINE,
    TOKEN_INDENT,
    TOKEN_DEDENT,

    TOKEN_IDENTIFIER,
    TOKEN_INT_LITERAL,
    TOKEN_DOUBLE_LITERAL,
    TOKEN_STRING_LITERAL,

    /* Klucove slova */
    TOKEN_DEF,
    TOKEN_DO,
    TOKEN_DOUBLE,
    TOKEN_ELSE,
    TOKEN_IF,
    TOKEN_IMPORT,
    TOKEN_INT,
    TOKEN_RETURN,
    TOKEN_STRING,
    TOKEN_THEN,
    TOKEN_UNIT,
    TOKEN_VAL,
    TOKEN_VAR,
    TOKEN_WHILE,

    TOKEN_ASSIGN,       /* =  */
    TOKEN_PLUS,         /* +  */
    TOKEN_MINUS,        /* -  */
    TOKEN_MULTIPLY,     /* *  */
    TOKEN_DIVIDE,       /* /  */

    TOKEN_EQUAL,        /* == */
    TOKEN_NOT_EQUAL,    /* != */
    TOKEN_LESS,         /* <  */
    TOKEN_LESS_EQUAL,   /* <= */
    TOKEN_GREATER,      /* >  */
    TOKEN_GREATER_EQUAL,/* >= */

    TOKEN_LPAREN,       /* ( */
    TOKEN_RPAREN,       /* ) */
    TOKEN_COLON,        /* : */
    TOKEN_COMMA,        /* , */
    TOKEN_SEMICOLON,    /* ; */
    TOKEN_DOT,          /* . */

    /* pre @main lebo je specialny, @ pouzivame iba v @main a stringoch*/
    TOKEN_MAIN_ANNOTATION,
} TokenType;


typedef struct {
    TokenType type;
    char *value;

    bool lf_before_rparen,
    unsigned int line;
    unsigned int column;

} Token;

void token_free(Token *token);

#endif