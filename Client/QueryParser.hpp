#ifndef QueryParser
#define QueryParser

#define AND "&&"
#define OR "||"
#define NOT "!"

#include "ClientUtils.h"
#include "_TODOremove.cpp"
#include <stdio.h>
#include <iostream>

typedef struct token {
	char type;
	int counter;
	std::string word;
} token;

std::vector<token> shunting_yard(std::string query);
//int evaluate(std::queue<std::string> rpn_expr);

#endif
