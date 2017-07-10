#include "QueryParser.hpp"

using namespace std;

int precedence(string op) {
    if(!op.compare(NOT))
        return 3;
    else if(!op.compare(AND))
        return 2;
    else if(!op.compare(OR))
        return 1;
    else
        return -1;
}

char to_type(string token) {
    if(!token.compare(NOT))
        return 'n';
    else if(!token.compare(AND))
        return 'a';
    else if(!token.compare(OR))
        return 'o';
    else
        return 't';
}

vector<token> shunting_yard(string query) {
    stack <string> operators;
    vector <token> output;

    int i = 0;
    while (query[i] != '\0') {
        if (query[i] == ' ') {
            i++;
            continue;
        }

        if(query[i] == '(') {
            operators.push("(");
            i++;
            continue;
        }

        if(query[i] == ')') {
            while (operators.top().compare("(")) {
                // push the top of the operator stack into the output
                token tkn;
                tkn.type = to_type(operators.top());

                output.push_back(tkn);
                operators.pop();

                if (operators.empty())
                    throw invalid_argument("Mismatched parenthesis while parsing!");
            }

            operators.pop(); // pops the "("
            i++;
            continue;
        }

        // we have either AND, OR, NOT or a word-token
        string word;
        while(query[i] != '\0' && query[i] != ' ') {
            word += query[i];
            i++;
        } // TODO recognize token without spaces

        //printf("iteration %d %c\n", i, token);
        //cout << "op stack\t"<<operators<<endl;
        //cout << "out\t\t"<< output<<endl;

        int is_operator = !word.compare(AND) || !word.compare(NOT) || !word.compare(OR);

        if(is_operator) {
            int curr_precedence = operators.empty()? -1 : precedence(operators.top());
            //printf("curr_precedence %d\n", curr_precedence);

            while (curr_precedence >= precedence(word)) {
                token tkn;
                tkn.type = to_type(operators.top());

                output.push_back(tkn);
                operators.pop();
                curr_precedence = operators.empty()? -1 : precedence(operators.top());
            }

            operators.push(word);
        } else {
            token tkn;
            tkn.type = 't';
            tkn.word = word;

            output.push_back(tkn);
        }
        //printf("-----------\n");
    }

    while (!operators.empty()) {
        string top = operators.top();
        operators.pop();

        if (!top.compare("(") || !top.compare(")"))
            throw invalid_argument("Mismatched parenthesis!");

        token tkn;
        tkn.type = to_type(top);
        tkn.word = top;

        output.push_back(tkn);
    }

    /*cout << "print out" << endl << "[";
    for(int i = 0; i < output.size(); i++)
        cout << output[i].type << " ";
    printf("]\n");*/

    return output;
}
