#ifndef QueryParser_hpp
#define QueryParser_hpp

#include "EnglishAnalyzer.h"
#include "../Utils.h"
#include "../Definitions.h"

class QueryParser {
private:
    EnglishAnalyzer analyzer;

public:
    std::vector<token> tokenize(char* query);
    std::vector<token> shunting_yard(std::vector<token>);
};

#endif
