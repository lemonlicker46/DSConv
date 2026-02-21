#ifndef DSCONV_GENERATOR_H
#define DSCONV_GENERATOR_H

#include "ast.h"
#include "../include/dsconv.h"

/* Generate target code for given AST and options. */
int generate_for_targets(const ASTRoot *ast, const Options *opts);

#endif /* DSCONV_GENERATOR_H */
