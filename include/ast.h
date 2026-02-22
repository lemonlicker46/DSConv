#ifndef DSCONV_AST_H
#define DSCONV_AST_H

/* AST declarations for C-like type system */

#include <stdint.h>

typedef enum {
    TYPE_BUILTIN,
    TYPE_POINTER,
    TYPE_ARRAY,
    TYPE_FUNCTION,
    TYPE_STRUCT,
    TYPE_UNION,
    TYPE_ENUM,
    TYPE_ALIAS
} TypeKind;

typedef struct Type Type;

typedef struct Member {
    char *name; /* may be NULL for anonymous members */
    Type *type;
    struct Member *next;
} Member;

typedef struct StructInfo {
    char *tag; /* may be NULL for anonymous */
    Member *members;
    int is_forward;
} StructInfo;

typedef struct EnumValue {
    char *name;
    int64_t value;
    struct EnumValue *next;
} EnumValue;

struct Type {
    TypeKind kind;
    union {
        char *builtin_name; /* e.g., "int", "char" */
        struct { Type *base; } ptr;
        struct { Type *base; int length; } array;
        struct { Type *ret; Type *params; /* params as linked list via Member.name holding param name */ } func;
        StructInfo s;
        EnumValue *e;
        char *alias_to; /* name of the aliased type */
    } u;
    Type *next; /* for lists */
};

typedef struct ASTNode {
    Type *type; /* top-level type for typedefs or structs/enums */
    char *name; /* name of typedef or tag if applicable */
    int is_typedef; /* 1 if typedef, 0 if definition */
    struct ASTNode *next;
} ASTNode;

typedef struct ASTRoot {
    ASTNode *first;
} ASTRoot;

#endif /* DSCONV_AST_H */
