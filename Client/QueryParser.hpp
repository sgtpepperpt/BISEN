#ifndef QueryParser_hpp
#define QueryParser_hpp

#define WORD_TOKEN 'w'
#define META_TOKEN 'z'

#include "EnglishAnalyzer.h"
#include "ClientUtils.h"

typedef struct token {
    char type;
    int counter;
    std::string word;
} token;

class QueryParser {
private:
    EnglishAnalyzer* analyzer;

public:
    std::vector<token> tokenize(std::string query);
    std::vector<token> shunting_yard(std::vector<token>);
};

#endif
