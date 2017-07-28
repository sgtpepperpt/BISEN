#include "QueryParser.hpp"

using namespace std;

int is_operator(char c) {
    return c == '&' || c == '|' || c == '!' || c == '(' || c == ')';
}

int precedence(char op) {
    if(op == '!')
        return 3;
    else if(op == '&')
        return 2;
    else if(op == '|')
        return 1;
    else
        return -1;
}

vector<token> QueryParser::tokenize(char* query) {
    vector <token> result;

    int i = 0;
    while (query[i] != '\0') {
        if (query[i] == ' ') {
            i++;
            continue;
        }

        token tkn;
        if(is_operator(query[i])) {
            tkn.type = query[i];
            
            // eliminate next character if it's the same (&& or ||)
            if((query[i] == '&' || query[i] == '|') && query[i+1] == query[i])
                i++;
            
            i++;
        } else {
            // it's a word
            tkn.type = WORD_TOKEN;
            tkn.word = (char*) malloc(sizeof(char) * MAX_WORD_SIZE);
            int pos = 0;

            while(query[i] != '\0' && query[i] != ' ' && !is_operator(query[i]) && pos < MAX_WORD_SIZE){
                tkn.word[pos++] = tolower(query[i]);
                i++;
            }
            // NULL termination is added in serialization
            //tkn.word += '\0';
            tkn.word = analyzer.stemWord(tkn.word);
        }

        result.push_back(tkn);
    }

    return result;
}

vector<token> QueryParser::shunting_yard(vector<token> infix_query) {
    stack<char> operators;
    vector<token> output;

    for(unsigned i = 0; i < infix_query.size(); i++) {
        token tkn = infix_query[i];

        if(tkn.type == WORD_TOKEN) {
            output.push_back(tkn);
        } else if(tkn.type == '&' || tkn.type == '|' || tkn.type == '!') {
            int curr_precedence = operators.empty()? -1 : precedence(operators.top());
            
            while (curr_precedence >= precedence(tkn.type)) {
                token tkn2;
                tkn2.type = operators.top();
                operators.pop();
                output.push_back(tkn2);
                
                curr_precedence = operators.empty()? -1 : precedence(operators.top());
            }

            operators.push(tkn.type);
        } else if(tkn.type == '(') {
            operators.push(tkn.type);
        } else if(tkn.type == ')') {
            while (operators.top() != '(') {
                token tkn2;
                tkn2.type = operators.top();
                operators.pop();
                
                output.push_back(tkn2);

                if (operators.empty())
                    throw invalid_argument("Mismatched parenthesis while parsing!");
            }

            operators.pop(); // pops the '('
        }
    }
    
    while (!operators.empty()) {
        char top = operators.top();
        operators.pop();

        if (top == '(' || top == ')')
            throw invalid_argument("Mismatched parenthesis!");

        token tkn;
        tkn.type = top;
        output.push_back(tkn);
    }

    return output;
}
