#ifndef QueryEvaluator
#define QueryEvaluator

#include "IeeCrypt.hpp"
#include <stdio.h>
#include "Definitions.h"
#include "vec_int.h"

// this function takes a reverse-polish notation boolean expression
// of text documents and evaluates it, returning the set of documents
// that follow that condition
vec_int evaluate(std::vector<token> rpn_expr, int nDocs);

#endif
