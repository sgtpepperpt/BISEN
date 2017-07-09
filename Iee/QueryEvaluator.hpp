#ifndef QueryEvaluator
#define QueryEvaluator

#include "IeeUtils.h"
#include <stdio.h>
#include <iostream>

typedef struct token {
	char type;
	int counter;
	std::string word;
	std::vector<int> docs;
} token;

// this function takes a reverse-polish notation boolean expression
// of text documents and evaluates it, returning the set of documents
// that follow that condition
std::vector<int> evaluate(std::deque<token> rpn_expr);

#endif
