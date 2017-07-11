#ifndef QueryParser
#define QueryParser

#define WORD_TOKEN 'w'
#define META_TOKEN 'z'

#include "ClientUtils.h"

typedef struct token {
    char type;
    int counter;
    std::string word;
} token;

std::vector<token> tokenize(std::string query);
std::vector<token> shunting_yard(std::vector<token>);

#endif
