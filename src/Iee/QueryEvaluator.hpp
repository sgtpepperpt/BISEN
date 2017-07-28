#ifndef QueryEvaluator
#define QueryEvaluator

#define WORD_TOKEN 'w'
#define META_TOKEN 'z'

#include "IeeCrypt.hpp"
//#include "IeeUtils.h"
#include <stdio.h>
//#include <iostream>
#include "Definitions.h"

// this function takes a reverse-polish notation boolean expression
// of text documents and evaluates it, returning the set of documents
// that follow that condition
std::vector<int> evaluate(std::vector<token> rpn_expr, int nDocs);

#endif
