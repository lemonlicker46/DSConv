#ifndef DSCONV_LEXER_H
#define DSCONV_LEXER_H

#include <stddef.h>

typedef enum {
    TOK_EOF = 0,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_SEMI,
    TOK_COMMA,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACK,
    TOK_RBRACK,
    TOK_COLON,
    TOK_STAR,
    TOK_EQ,
    TOK_OTHER,
    /* keywords */
    TOK_STRUCT,
    TOK_UNION,
    TOK_ENUM,
    TOK_TYPEDEF,
    TOK_CONST,
    TOK_VOLATILE,
    TOK_UNSIGNED,
    TOK_SIGNED,
    TOK_SHORT,
    TOK_LONG,
    TOK_INT,
    TOK_CHAR,
    TOK_VOID,
    TOK_FLOAT,
    TOK_DOUBLE
} TokenKind;

typedef struct {
    TokenKind kind;
    char *text; /* allocated; NULL for simple tokens */
    int line;
} Token;

typedef struct Lexer Lexer;

Lexer *lexer_create_from_file(const char *path);
Lexer *lexer_create_from_string(const char *str);
void lexer_destroy(Lexer *lx);
Token lexer_next(Lexer *lx);
Token lexer_peek(Lexer *lx);

/* helpers */
int token_is_ident(const Token *t, const char *s);

#endif /* DSCONV_LEXER_H */
