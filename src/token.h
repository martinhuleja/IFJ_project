#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
    TOKEN_EOF,
    TOKEN_NEWLINE,

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

    /* pre @main lebo je specialny lebo @ pouzivame ib v @main*/
    TOKEN_MAIN_ANNOTATION,
} TokenType;


typedef struct {
    TokenType type;
    char *value;

    unsigned int line;
    unsigned int column;

} Token;

/* bude sa menit */
/* na indentacii sa este pracuje */

#endif