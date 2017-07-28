#ifndef QueryEvaluator
#define QueryEvaluator

#include <stdio.h>

#include "IeeCrypt.hpp"
#include "IeeUtils.h"
#include "Definitions.h"
#include "vec_int.h"

// this function takes a reverse-polish notation boolean expression
// of text documents and evaluates it, returning the set of documents
// that follow that condition
vec_int evaluate(std::vector<iee_token> rpn_expr, int nDocs);

#endif
