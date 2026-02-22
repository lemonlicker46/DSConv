#ifndef DSCONV_PARSER_H
#define DSCONV_PARSER_H

#include "ast.h"
#include "dsconv.h"

/* Parse an input file into an ASTRoot. Returns NULL on error. */
ASTRoot *parse_file(const char *path, const Options *opts);

/* Parse a string into an ASTRoot. Returns NULL on error. */
ASTRoot *parse_string(const char *code, const Options *opts);

#endif /* DSCONV_PARSER_H */
