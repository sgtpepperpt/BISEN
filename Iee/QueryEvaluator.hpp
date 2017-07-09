#ifndef QueryEvaluator
#define QueryEvaluator

#define AND "&&"
#define OR "||"
#define NOT "!"

#include "IeeUtils.h"
#include <stdio.h>
#include <iostream>

typedef struct token {
	char type;
	int counter;
	std::string word;
	std::set<int> docs;
} token;

// this function takes a reverse-polish notation boolean expression
// of text documents and evaluates it, returning the set of documents
// that follow that condition
std::vector<int> evaluate(std::deque<token> rpn_expr);

#endif
