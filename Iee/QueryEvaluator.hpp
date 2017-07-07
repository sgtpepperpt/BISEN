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
} token;

int evaluate(std::queue<std::string> rpn_expr);

#endif
