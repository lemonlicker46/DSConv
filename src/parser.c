#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dsconv.h"
#include "parser.h"
#include "lexer.h"
#include "ast.h"

/* Simple symbol/typedef table for aliases */
typedef struct Alias {
    char *name;
    Type *type;
    struct Alias *next;
} Alias;

static Alias *alias_table = NULL;

static void alias_add(const char *name, Type *t) {
    Alias *a = (Alias*)malloc(sizeof(Alias));
    a->name = strdup(name);
    a->type = t;
    a->next = alias_table;
    alias_table = a;
}

static Type *type_make_builtin(const char *name) {
    Type *t = (Type*)calloc(1, sizeof(Type));
    t->kind = TYPE_BUILTIN;
    t->u.builtin_name = strdup(name);
    return t;
}

static Type *type_make_ptr(Type *base) {
    Type *t = (Type*)calloc(1, sizeof(Type));
    t->kind = TYPE_POINTER;
    t->u.ptr.base = base;
    return t;
}

static Type *type_make_array(Type *base, int len) {
    Type *t = (Type*)calloc(1, sizeof(Type));
    t->kind = TYPE_ARRAY;
    t->u.array.base = base;
    t->u.array.length = len;
    return t;
}

static void ast_add_node(ASTRoot *root, Type *t, const char *name, int is_typedef) {
    ASTNode *n = (ASTNode*)calloc(1, sizeof(ASTNode));
    n->type = t;
    if (name) n->name = strdup(name);
    n->is_typedef = is_typedef;
    n->next = root->first;
    root->first = n;
}

/*
static void skip_braces(Lexer *lx) {
    int depth = 1;
    while (depth > 0) {
        Token t = lexer_next(lx);
        if (t.kind == TOK_LBRACE) depth++;
        else if (t.kind == TOK_RBRACE) depth--;
        if (t.text) free(t.text);
        if (t.kind == TOK_EOF) break;
    }
}
*/

/* Parse a simple type specifier (builtin or struct/union/enum tag) */
static Type *parse_type_specifier(Lexer *lx) {
    Token t = lexer_peek(lx);
    if (t.kind == TOK_STRUCT || t.kind == TOK_UNION) {
        Token consumed_kw = lexer_next(lx); if (consumed_kw.text) free(consumed_kw.text);
        int is_struct = (t.kind == TOK_STRUCT);
        Token next = lexer_peek(lx);
        char *tag = NULL;
        if (next.kind == TOK_IDENT) { next = lexer_next(lx); tag = strdup(next.text); free(next.text); }
        Token p = lexer_peek(lx);
        if (p.kind == TOK_LBRACE) {
            /* definition */
            lexer_next(lx); /* consume { */
            Type *tst = (Type*)calloc(1, sizeof(Type));
            tst->kind = is_struct ? TYPE_STRUCT : TYPE_UNION;
            tst->u.s.tag = tag ? tag : NULL;
            Member *last = NULL;
            while (1) {
                Token q = lexer_peek(lx);
                if (q.kind == TOK_RBRACE) { Token tmp = lexer_next(lx); if (tmp.text) free(tmp.text); break; }
                /* parse member declaration: very simple: <builtin> <ident> [array]; */
                Token spec = lexer_next(lx);
                if (spec.kind == TOK_IDENT || spec.kind == TOK_INT || spec.kind == TOK_CHAR ||
                    spec.kind == TOK_VOID || spec.kind == TOK_FLOAT || spec.kind == TOK_DOUBLE ||
                    spec.kind == TOK_UNSIGNED || spec.kind == TOK_SIGNED) {
                    char *typename = spec.text ? spec.text : NULL;
                    Token name = lexer_next(lx);
                    if (name.kind == TOK_IDENT) {
                        Member *m = (Member*)calloc(1, sizeof(Member));
                        m->name = strdup(name.text);
                        Type *mt = type_make_builtin(typename ? typename : "int");
                        m->type = mt;
                        // Check for array
                        Token peek = lexer_peek(lx);
                        if (peek.kind == TOK_LBRACK) {
                            lexer_next(lx); // consume [
                            Token num = lexer_next(lx);
                            if (num.kind == TOK_NUMBER) {
                                int len = atoi(num.text);
                                m->type = type_make_array(m->type, len);
                                lexer_next(lx); // ]
                            }
                            if (num.text) free(num.text);
                        }
                        if (last) last->next = m; else tst->u.s.members = m;
                        last = m;
                        if (name.text) free(name.text);
                    } else {
                        if (name.text) free(name.text);
                    }
                    /* consume until semicolon */
                    while (1) { Token z = lexer_next(lx); if (z.kind==TOK_SEMI) { if (z.text) free(z.text); break; } if (z.text) free(z.text); }
                    if (spec.text) free(spec.text);
                } else {
                    if (spec.text) free(spec.text);
                    /* skip token to avoid infinite loop */
                    Token skip = lexer_next(lx); if (skip.text) free(skip.text);
                }
            }
            return tst;
        } else {
            /* reference to tag */
            Type *tref = (Type*)calloc(1, sizeof(Type));
            tref->kind = TYPE_STRUCT;
            tref->u.s.tag = tag ? tag : NULL;
            return tref;
        }
    } else if (t.kind == TOK_ENUM) {
        lexer_next(lx);
        Token next = lexer_peek(lx);
        char *tag = NULL;
        if (next.kind == TOK_IDENT) { next = lexer_next(lx); tag = strdup(next.text); free(next.text); }
        Token p = lexer_peek(lx);
        if (p.kind == TOK_LBRACE) {
            lexer_next(lx);
            Type *ten = (Type*)calloc(1, sizeof(Type));
            ten->kind = TYPE_ENUM;
            EnumValue *last = NULL;
            int64_t val = 0;
            while (1) {
                Token ev = lexer_next(lx);
                if (ev.kind == TOK_IDENT) {
                    EnumValue *e = (EnumValue*)calloc(1, sizeof(EnumValue));
                    e->name = strdup(ev.text);
                    free(ev.text);
                    Token maybe_eq = lexer_peek(lx);
                    if (maybe_eq.kind == TOK_EQ) {
                        lexer_next(lx);
                        Token num = lexer_next(lx);
                        if (num.kind == TOK_NUMBER) { e->value = strtoll(num.text, NULL, 10); free(num.text); }
                        Token semi_or_comma = lexer_peek(lx);
                    } else {
                        e->value = val++;
                    }
                    val = e->value + 1;
                    if (last) last->next = e; else ten->u.e = e;
                    last = e;
                    Token sep = lexer_next(lx);
                    if (sep.kind == TOK_COMMA) { if (sep.text) free(sep.text); continue; }
                    if (sep.kind == TOK_RBRACE) { if (sep.text) free(sep.text); break; }
                } else if (ev.kind == TOK_RBRACE) { if (ev.text) free(ev.text); break; }
                if (ev.text) free(ev.text);
            }
            return ten;
        } else {
            /* reference */
            Type *ten = (Type*)calloc(1, sizeof(Type));
            ten->kind = TYPE_ENUM;
            return ten;
        }
    } else if (t.kind == TOK_INT || t.kind == TOK_CHAR || t.kind == TOK_VOID ||
               t.kind == TOK_FLOAT || t.kind == TOK_DOUBLE || t.kind == TOK_UNSIGNED || t.kind == TOK_SIGNED) {
        Token tok = lexer_next(lx);
        char *nm = tok.text ? tok.text : strdup("int");
        Type *tb = type_make_builtin(nm);
        if (tok.text) free(tok.text);
        return tb;
    } else if (t.kind == TOK_IDENT) {
        Token tok = lexer_next(lx);
        /* could be typedef name */
        Type *ta = (Type*)calloc(1, sizeof(Type));
        ta->kind = TYPE_ALIAS;
        ta->u.alias_to = strdup(tok.text);
        free(tok.text);
        return ta;
    }
    /* unknown */
    Token skip = lexer_next(lx);
    if (skip.text) free(skip.text);
    return NULL;
}

/* Parse a declarator, returning the full type and setting name_out */
static Type *parse_declarator(Lexer *lx, Type *base, char **name_out) {
    *name_out = NULL;
    // pointers
    while (1) {
        Token p = lexer_peek(lx);
        if (p.kind == TOK_STAR) {
            lexer_next(lx); if (p.text) free(p.text);
            Type *pt = (Type*)calloc(1, sizeof(Type));
            pt->kind = TYPE_POINTER;
            pt->u.ptr.base = base;
            base = pt;
        } else {
            break;
        }
    }
    // direct declarator
    Token d = lexer_peek(lx);
    if (d.kind == TOK_IDENT) {
        Token n = lexer_next(lx);
        *name_out = strdup(n.text);
        if (n.text) free(n.text);
        // function?
        Token f = lexer_peek(lx);
        if (f.kind == TOK_LPAREN) {
            lexer_next(lx); if (f.text) free(f.text);
            Type *ft = (Type*)calloc(1, sizeof(Type));
            ft->kind = TYPE_FUNCTION;
            ft->u.func.ret = base;
            ft->u.func.params = NULL; // TODO: parse params
            // consume until )
            while (1) {
                Token z = lexer_next(lx);
                if (z.kind == TOK_RPAREN) { if (z.text) free(z.text); break; }
                if (z.text) free(z.text);
            }
            base = ft;
        }
        // array?
        while (1) {
            Token a = lexer_peek(lx);
            if (a.kind == TOK_LBRACK) {
                lexer_next(lx); if (a.text) free(a.text);
                Token num = lexer_next(lx);
                int len = 0;
                if (num.kind == TOK_NUMBER) {
                    len = atoi(num.text);
                }
                if (num.text) free(num.text);
                Token r = lexer_next(lx);
                if (r.kind != TOK_RBRACK) {
                    fprintf(stderr, "expected ]\n");
                }
                if (r.text) free(r.text);
                Type *arr = (Type*)calloc(1, sizeof(Type));
                arr->kind = TYPE_ARRAY;
                arr->u.array.base = base;
                arr->u.array.length = len;
                base = arr;
            } else {
                break;
            }
        }
    } else if (d.kind == TOK_LPAREN) {
        lexer_next(lx); if (d.text) free(d.text);
        base = parse_declarator(lx, base, name_out);
        Token rp = lexer_next(lx);
        if (rp.kind != TOK_RPAREN) {
            fprintf(stderr, "expected )\n");
        }
        if (rp.text) free(rp.text);
        // function?
        Token f = lexer_peek(lx);
        if (f.kind == TOK_LPAREN) {
            lexer_next(lx); if (f.text) free(f.text);
            Type *ft = (Type*)calloc(1, sizeof(Type));
            ft->kind = TYPE_FUNCTION;
            ft->u.func.ret = base;
            ft->u.func.params = NULL;
            while (1) {
                Token z = lexer_next(lx);
                if (z.kind == TOK_RPAREN) { if (z.text) free(z.text); break; }
                if (z.text) free(z.text);
            }
            base = ft;
        }
    } else {
        // abstract or error
    }
    return base;
}

ASTRoot *parse_string(const char *code, const Options *opts) {
    (void)opts;
    fprintf(stderr, "parse_string: starting\n");
    Lexer *lx = lexer_create_from_string(code);
    if (!lx) { fprintf(stderr, "failed to create lexer from string\n"); return NULL; }
    ASTRoot *root = (ASTRoot*)calloc(1, sizeof(ASTRoot));

    fprintf(stderr, "parse_string: starting token loop\n");

    while (1) {
        Token t = lexer_peek(lx);
        fprintf(stderr, "token peek: kind=%d text=%s line=%d\n", t.kind, t.text ? t.text : "(null)", t.line);
        if (t.kind == TOK_EOF) { break; }
        if (t.kind == TOK_TYPEDEF) {
            fprintf(stderr, "parse: found TYPEDEF\n");
            Token consumed = lexer_next(lx); if (consumed.text) free(consumed.text);
            Type *spec = parse_type_specifier(lx);
            fprintf(stderr, "parse: typedef spec parsed: %p\n", (void*)spec);
            fprintf(stderr, "parse: about to parse declarator\n");
            char *name;
            Type *full_type = parse_declarator(lx, spec, &name);
            fprintf(stderr, "parse: declarator parsed, name=%s\n", name ? name : "null");
            if (name) {
                alias_add(name, full_type);
                ast_add_node(root, full_type, name, 1); // typedef
            }
            Token semi = lexer_next(lx);
            if (semi.kind != TOK_SEMI) {
                fprintf(stderr, "expected ; after typedef\n");
            }
            if (semi.text) free(semi.text);
            fprintf(stderr, "parse: typedef done\n");
        } else {
            // attempt to parse a declaration
            Type *spec = parse_type_specifier(lx);
            if (spec) {
                char *name;
                Type *full_type = parse_declarator(lx, spec, &name);
                if (name) {
                    ast_add_node(root, full_type, name, 0); // not typedef
                    free(name);
                }
                Token next = lexer_peek(lx);
                if (next.kind == TOK_LBRACE) {
                    // function definition, skip body
                    // skip_braces(lx);
                } else {
                    Token semi = lexer_next(lx);
                    if (semi.kind != TOK_SEMI) {
                        fprintf(stderr, "expected ; after declaration\n");
                    }
                    if (semi.text) free(semi.text);
                }
            } else {
                /* skip token */
                Token sk = lexer_next(lx); if (sk.text) free(sk.text);
            }
        }
        fprintf(stderr, "parse: iteration end\n");
    }

    lexer_destroy(lx);
    fprintf(stderr, "parse_string: finished, returning AST\n");
    return root;
}

ASTRoot *parse_file(const char *path, const Options *opts) {
    (void)opts;
    fprintf(stderr, "parse_file: opening '%s'\n", path);
    Lexer *lx = lexer_create_from_file(path);
    if (!lx) { fprintf(stderr, "failed to open: %s\n", path); return NULL; }
    ASTRoot *root = (ASTRoot*)calloc(1, sizeof(ASTRoot));

    fprintf(stderr, "parse_file: starting token loop\n");

    while (1) {
        Token t = lexer_peek(lx);
        fprintf(stderr, "token peek: kind=%d text=%s line=%d\n", t.kind, t.text ? t.text : "(null)", t.line);
        if (t.kind == TOK_EOF) { break; }
        if (t.kind == TOK_TYPEDEF) {
            fprintf(stderr, "parse: found TYPEDEF\n");
            Token consumed = lexer_next(lx); if (consumed.text) free(consumed.text);
            Type *spec = parse_type_specifier(lx);
            fprintf(stderr, "parse: typedef spec parsed: %p\n", (void*)spec);
            fprintf(stderr, "parse: about to parse declarator\n");
            char *name;
            Type *full_type = parse_declarator(lx, spec, &name);
            fprintf(stderr, "parse: declarator parsed, name=%s\n", name ? name : "null");
            if (name) {
                alias_add(name, full_type);
                ast_add_node(root, full_type, name, 1); // typedef
            }
            Token semi = lexer_next(lx);
            if (semi.kind != TOK_SEMI) {
                fprintf(stderr, "expected ; after typedef\n");
            }
            if (semi.text) free(semi.text);
            fprintf(stderr, "parse: typedef done\n");
        } else {
            // attempt to parse a declaration
            Type *spec = parse_type_specifier(lx);
            if (spec) {
                char *name;
                Type *full_type = parse_declarator(lx, spec, &name);
                if (name) {
                    ast_add_node(root, full_type, name, 0); // not typedef
                    free(name);
                }
                Token next = lexer_peek(lx);
                if (next.kind == TOK_LBRACE) {
                    // function definition, skip body
                    // skip_braces(lx);
                } else {
                    Token semi = lexer_next(lx);
                    if (semi.kind != TOK_SEMI) {
                        fprintf(stderr, "expected ; after declaration\n");
                    }
                    if (semi.text) free(semi.text);
                }
            } else {
                /* skip token */
                Token sk = lexer_next(lx); if (sk.text) free(sk.text);
            }
        }
        fprintf(stderr, "parse: iteration end\n");
    }

    lexer_destroy(lx);
    fprintf(stderr, "parse_file: finished, returning AST\n");
    return root;
}

