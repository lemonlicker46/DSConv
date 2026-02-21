#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "generator.h"
#include "ast.h"

static void print_type(const Type *t, int indent);

static void indent_print(int indent, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    for (int i = 0; i < indent; ++i) putchar(' ');
    vprintf(fmt, ap);
    va_end(ap);
}

static void print_members(const Member *m, int indent) {
    const Member *cur = m;
    while (cur) {
        indent_print(indent, "- %s: ", cur->name ? cur->name : "(anon)");
        print_type(cur->type, 0);
        putchar('\n');
        cur = cur->next;
    }
}

static void print_type(const Type *t, int indent) {
    if (!t) { printf("<null>"); return; }
    switch (t->kind) {
        case TYPE_BUILTIN: printf("%s", t->u.builtin_name); break;
        case TYPE_POINTER: printf("*"); print_type(t->u.ptr.base, 0); break;
        case TYPE_ARRAY: print_type(t->u.array.base, 0); printf("[%d]", t->u.array.length); break;
        case TYPE_FUNCTION: printf("func"); break;
        case TYPE_STRUCT: printf("struct %s", t->u.s.tag ? t->u.s.tag : "(anon)"); break;
        case TYPE_UNION: printf("union %s", t->u.s.tag ? t->u.s.tag : "(anon)"); break;
        case TYPE_ENUM: printf("enum"); break;
        case TYPE_ALIAS: printf("alias->%s", t->u.alias_to); break;
        default: printf("<unknown>"); break;
    }
}

static void print_type_to_file(FILE *f, const Type *t, int indent) {
    if (!t) { fprintf(f, "<null>"); return; }
    switch (t->kind) {
        case TYPE_BUILTIN: fprintf(f, "%s", t->u.builtin_name); break;
        case TYPE_POINTER: fprintf(f, "*"); print_type_to_file(f, t->u.ptr.base, 0); break;
        case TYPE_ARRAY: print_type_to_file(f, t->u.array.base, 0); fprintf(f, "[%d]", t->u.array.length); break;
        case TYPE_FUNCTION: fprintf(f, "(*)("); // TODO: params
            fprintf(f, ")"); print_type_to_file(f, t->u.func.ret, 0); break;
        case TYPE_STRUCT: fprintf(f, "struct");
            if (t->u.s.tag) fprintf(f, " %s", t->u.s.tag);
            break;
        case TYPE_UNION: fprintf(f, "union");
            if (t->u.s.tag) fprintf(f, " %s", t->u.s.tag);
            break;
        case TYPE_ENUM: fprintf(f, "enum");
            if (t->u.e) {
                fprintf(f, " { ");
                EnumValue *cur = t->u.e;
                while (cur) {
                    fprintf(f, "%s = %lld", cur->name, cur->value);
                    cur = cur->next;
                    if (cur) fprintf(f, ", ");
                }
                fprintf(f, " }");
            }
            break;
        case TYPE_ALIAS: fprintf(f, "%s", t->u.alias_to); break;
        default: fprintf(f, "<unknown>"); break;
    }
}

static void print_members_to_file(FILE *f, const Member *m, int indent) {
    const Member *cur = m;
    while (cur) {
        for (int i = 0; i < indent; ++i) fprintf(f, " ");
        if (cur->type->kind == TYPE_ARRAY) {
            print_type_to_file(f, cur->type->u.array.base, 0);
            fprintf(f, " %s[%d];\n", cur->name ? cur->name : "(anon)", cur->type->u.array.length);
        } else {
            print_type_to_file(f, cur->type, 0);
            fprintf(f, " %s;\n", cur->name ? cur->name : "(anon)");
        }
        cur = cur->next;
    }
}

static void print_declaration_to_file(FILE *f, const Type *t, const char *name) {
    if (!t) { fprintf(f, "<null>"); return; }
    if (t->kind == TYPE_FUNCTION) {
        // For function typedefs: return_type (*name)(params)
        print_type_to_file(f, t->u.func.ret, 0);
        fprintf(f, " (*%s)(", name ? name : "(anon)");
        // TODO: print params
        fprintf(f, ")");
    } else if (t->kind == TYPE_ARRAY) {
        print_declaration_to_file(f, t->u.array.base, name);
        fprintf(f, "[%d]", t->u.array.length);
    } else {
        print_type_to_file(f, t, 0);
        fprintf(f, " %s", name ? name : "(anon)");
    }
}

int generate_for_targets(const ASTRoot *ast, const Options *opts) {
    if (!ast) return 1;
    FILE *out = opts->output_file ? fopen(opts->output_file, "w") : stdout;
    if (!out) {
        fprintf(stderr, "Failed to open output file: %s\n", opts->output_file);
        return 1;
    }
    for (ASTNode *n = ast->first; n; n = n->next) {
        if (n->type->kind == TYPE_STRUCT) {
            fprintf(out, "struct");
            if (n->type->u.s.tag) fprintf(out, " %s", n->type->u.s.tag);
            if (!n->type->u.s.is_forward) {
                fprintf(out, " {\n");
                print_members_to_file(out, n->type->u.s.members, 1);
                fprintf(out, "}");
            }
            fprintf(out, ";\n");
            if (!n->is_typedef && n->name) {
                fprintf(out, "struct");
                if (n->type->u.s.tag) fprintf(out, " %s", n->type->u.s.tag);
                fprintf(out, " %s;\n", n->name);
            }
        } else if (n->type->kind == TYPE_UNION) {
            fprintf(out, "union");
            if (n->type->u.s.tag) fprintf(out, " %s", n->type->u.s.tag);
            if (!n->type->u.s.is_forward) {
                fprintf(out, " {\n");
                print_members_to_file(out, n->type->u.s.members, 1);
                fprintf(out, "}");
            }
            fprintf(out, ";\n");
            if (!n->is_typedef && n->name) {
                fprintf(out, "union");
                if (n->type->u.s.tag) fprintf(out, " %s", n->type->u.s.tag);
                fprintf(out, " %s;\n", n->name);
            }
        } else if (n->type->kind == TYPE_ENUM) {
            fprintf(out, "enum");
            if (n->type->u.e) {
                fprintf(out, " { ");
                EnumValue *cur = n->type->u.e;
                while (cur) {
                    fprintf(out, "%s = %lld", cur->name, cur->value);
                    cur = cur->next;
                    if (cur) fprintf(out, ", ");
                }
                fprintf(out, " }");
            }
            if (n->is_typedef) {
                fprintf(out, " %s", n->name ? n->name : "(anon)");
            }
            fprintf(out, ";\n");
        } else {
            if (n->is_typedef) {
                fprintf(out, "typedef ");
            }
            print_declaration_to_file(out, n->type, n->name);
            fprintf(out, ";\n");
        }
    }
    if (out != stdout) fclose(out);
    return 0;
}

