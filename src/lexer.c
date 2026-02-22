#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct Lexer {
    char *buf;
    size_t pos;
    int line;
    Token peeked;
    int has_peek;
};

static const char *kw_names[] = {
    "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", ""
};

static Token make_token(TokenKind k, const char *txt, int line) {
    Token t;
    t.kind = k;
    if (txt) t.text = strdup(txt); else t.text = NULL;
    t.line = line;
    return t;
}

Lexer *lexer_create_from_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = (char*)malloc(sz + 1);
    if (!buf) { fclose(f); return NULL; }
    fread(buf, 1, sz, f);
    buf[sz] = '\0';
    fclose(f);
    Lexer *lx = (Lexer*)calloc(1, sizeof(Lexer));
    lx->buf = buf;
    lx->pos = 0;
    lx->line = 1;
    lx->has_peek = 0;
    lx->peeked.text = NULL;
    return lx;
}

Lexer *lexer_create_from_string(const char *str) {
    size_t sz = strlen(str);
    char *buf = (char*)malloc(sz + 1);
    if (!buf) return NULL;
    strcpy(buf, str);
    Lexer *lx = (Lexer*)calloc(1, sizeof(Lexer));
    lx->buf = buf;
    lx->pos = 0;
    lx->line = 1;
    lx->has_peek = 0;
    lx->peeked.text = NULL;
    return lx;
}

static void skip_space(Lexer *lx) {
    char *b = lx->buf;
    while (b[lx->pos]) {
        char c = b[lx->pos];
        if (c == '/') {
            if (b[lx->pos+1] == '/') { /* line comment */
                lx->pos += 2;
                while (b[lx->pos] && b[lx->pos] != '\n') lx->pos++;
                continue;
            } else if (b[lx->pos+1] == '*') { /* block comment */
                lx->pos += 2;
                while (b[lx->pos] && !(b[lx->pos]=='*' && b[lx->pos+1]=='/')) {
                    if (b[lx->pos] == '\n') lx->line++;
                    lx->pos++;
                }
                if (b[lx->pos]) lx->pos += 2;
                continue;
            }
        }
        if (isspace((unsigned char)c)) {
            if (c == '\n') lx->line++;
            lx->pos++;
            continue;
        }
        break;
    }
}

static int is_ident_start(char c) { return isalpha((unsigned char)c) || c=='_'; }
static int is_ident_cont(char c) { return isalnum((unsigned char)c) || c=='_'; }

static Token keyword_or_ident(const char *s, int line) {
    if (strcmp(s, "struct") == 0) return make_token(TOK_STRUCT, NULL, line);
    if (strcmp(s, "union") == 0) return make_token(TOK_UNION, NULL, line);
    if (strcmp(s, "enum") == 0) return make_token(TOK_ENUM, NULL, line);
    if (strcmp(s, "typedef") == 0) return make_token(TOK_TYPEDEF, NULL, line);
    if (strcmp(s, "const") == 0) return make_token(TOK_CONST, NULL, line);
    if (strcmp(s, "volatile") == 0) return make_token(TOK_VOLATILE, NULL, line);
    if (strcmp(s, "unsigned") == 0) return make_token(TOK_UNSIGNED, NULL, line);
    if (strcmp(s, "signed") == 0) return make_token(TOK_SIGNED, NULL, line);
    if (strcmp(s, "short") == 0) return make_token(TOK_SHORT, NULL, line);
    if (strcmp(s, "long") == 0) return make_token(TOK_LONG, NULL, line);
    if (strcmp(s, "int") == 0) return make_token(TOK_INT, NULL, line);
    if (strcmp(s, "char") == 0) return make_token(TOK_CHAR, NULL, line);
    if (strcmp(s, "void") == 0) return make_token(TOK_VOID, NULL, line);
    if (strcmp(s, "float") == 0) return make_token(TOK_FLOAT, NULL, line);
    if (strcmp(s, "double") == 0) return make_token(TOK_DOUBLE, NULL, line);
    return make_token(TOK_IDENT, s, line);
}

Token lexer_next(Lexer *lx) {
    {
        int start = (int)lx->pos;
        int len = 20;
        char temp[64];
        int i = 0;
        for (int k = 0; k < len && lx->buf[start + k]; ++k) temp[i++] = lx->buf[start + k];
        temp[i] = '\0';
        fprintf(stderr, "lexer_next: pos=%zu buf=""%s"" line=%d\n", lx->pos, temp, lx->line);
    }
    if (lx->has_peek) {
        lx->has_peek = 0;
        Token t = lx->peeked;
        lx->peeked.text = NULL; /* ownership moved */
        return t;
    }
    skip_space(lx);
    char *b = lx->buf;
    int line = lx->line;
    char c = b[lx->pos];
    if (!c) return make_token(TOK_EOF, NULL, line);
    if (is_ident_start(c)) {
        size_t start = lx->pos;
        lx->pos++;
        while (is_ident_cont(b[lx->pos])) lx->pos++;
        size_t len = lx->pos - start;
        char *s = (char*)malloc(len+1);
        memcpy(s, b+start, len);
        s[len] = '\0';
        Token t = keyword_or_ident(s, line);
        if (t.kind == TOK_IDENT) { /* keep text */ }
        else { free(s); s = NULL; }
        if (t.text == NULL && s) t.text = s; /* keyword tokens keep NULL text */
        fprintf(stderr, "lexer_next: return ident/kw kind=%d text=%s pos=%zu\n", t.kind, t.text ? t.text : "(null)", lx->pos);
        return t;
    }
    if (isdigit((unsigned char)c)) {
        size_t start = lx->pos;
        while (isdigit((unsigned char)b[lx->pos])) lx->pos++;
        size_t len = lx->pos - start;
        char *s = (char*)malloc(len+1);
        memcpy(s, b+start, len);
        s[len] = '\0';
        Token t = make_token(TOK_NUMBER, s, line);
        fprintf(stderr, "lexer_next: return number text=%s pos=%zu\n", t.text ? t.text : "(null)", lx->pos);
        return t;
    }
    /* simple single-char tokens */
    lx->pos++;
    switch (c) {
        case ';': { Token t = make_token(TOK_SEMI, NULL, line); fprintf(stderr, "lexer_next: return ; pos=%zu\n", lx->pos); return t; }
        case ',': { Token t = make_token(TOK_COMMA, NULL, line); fprintf(stderr, "lexer_next: return , pos=%zu\n", lx->pos); return t; }
        case '{': { Token t = make_token(TOK_LBRACE, NULL, line); fprintf(stderr, "lexer_next: return { pos=%zu\n", lx->pos); return t; }
        case '}': { Token t = make_token(TOK_RBRACE, NULL, line); fprintf(stderr, "lexer_next: return } pos=%zu\n", lx->pos); return t; }
        case '(': { Token t = make_token(TOK_LPAREN, NULL, line); fprintf(stderr, "lexer_next: return ( pos=%zu\n", lx->pos); return t; }
        case ')': { Token t = make_token(TOK_RPAREN, NULL, line); fprintf(stderr, "lexer_next: return ) pos=%zu\n", lx->pos); return t; }
        case '[': { Token t = make_token(TOK_LBRACK, NULL, line); fprintf(stderr, "lexer_next: return [ pos=%zu\n", lx->pos); return t; }
        case ']': { Token t = make_token(TOK_RBRACK, NULL, line); fprintf(stderr, "lexer_next: return ] pos=%zu\n", lx->pos); return t; }
        case ':': { Token t = make_token(TOK_COLON, NULL, line); fprintf(stderr, "lexer_next: return : pos=%zu\n", lx->pos); return t; }
        case '*': { Token t = make_token(TOK_STAR, NULL, line); fprintf(stderr, "lexer_next: return * pos=%zu\n", lx->pos); return t; }
        case '=': { Token t = make_token(TOK_EQ, NULL, line); fprintf(stderr, "lexer_next: return = pos=%zu\n", lx->pos); return t; }
        case '"': {
            size_t start = lx->pos;
            while (b[lx->pos] && b[lx->pos] != '"') {
                if (b[lx->pos] == '\\') lx->pos += 2; else lx->pos++;
            }
            size_t len = lx->pos - start;
            char *s = (char*)malloc(len+1);
            memcpy(s, b+start, len);
            s[len] = '\0';
            if (b[lx->pos] == '"') lx->pos++;
            Token t = make_token(TOK_STRING, s, line);
            fprintf(stderr, "lexer_next: return string pos=%zu\n", lx->pos);
            return t;
        }
        default: { Token t = make_token(TOK_OTHER, NULL, line); fprintf(stderr, "lexer_next: return other pos=%zu\n", lx->pos); return t; }
    }
}

Token lexer_peek(Lexer *lx) {
    if (!lx->has_peek) {
        lx->peeked = lexer_next(lx);
        lx->has_peek = 1;
    }
    return lx->peeked;
}

int token_is_ident(const Token *t, const char *s) {
    if (!t) return 0;
    if (t->kind != TOK_IDENT) return 0;
    return strcmp(t->text, s) == 0;
}

void lexer_destroy(Lexer *lx) {
    if (!lx) return;
    if (lx->peeked.text) free(lx->peeked.text);
    free(lx->buf);
    free(lx);
}
