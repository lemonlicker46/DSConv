#ifndef DSCONV_H
#define DSCONV_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef enum {
    NAME_POLICY_PRESERVE = 0,
    NAME_POLICY_AUTO,
    NAME_POLICY_EXPLICIT
} NamePolicy;

typedef enum {
    ASSIGN_INTERNAL = 0,
    ASSIGN_EXTERNAL,
    ASSIGN_BOTH
} AssignStyle;

typedef enum {
    EXPAND_MEMBERS = 0,
    WRAP_AS_ARRAY
} ExpandMode;

typedef struct {
    const char *input_file;
    const char *input_string;
    const char *src_lang; /* "c", "fb", "fp" */
    const char *targets;  /* comma-separated targets or "all" */
    const char *output_file;
    const char *metadata_file;
    const char *instance_name;
    NamePolicy name_policy;
    AssignStyle assign_style;
    ExpandMode expand_mode;
    int silent;
    int enable_suffixes;
    int array_output;
    int explicit_cast;
    int output_as_struct;
    int output_as_union;
    int output_as_enum;
    int forward_declarations;
    int normal_declarations;
    int global_scope;
    int local_scope;
    char *local_function;
} Options;

#ifdef __cplusplus
}
#endif

#endif /* DSCONV_H */
