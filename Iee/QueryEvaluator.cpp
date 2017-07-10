#include "QueryEvaluator.hpp"

using namespace std;

// receives a set of docs and returns its negation in the
// full docs set; equivalent to the set differentiation:
// {all-docs} \ {negate}
vector<int> get_not_docs(int nDocs, vector<int> negate){
    int *count = new int[nDocs];
    
    for(int i = 0; i < nDocs; i++)
        count[i] = 0;

    // increase the count for elems in the original vector
    for(int i = 0; i < negate.size(); i++) {
        count[negate[i]]++;
    }
        
    // all elements that have count == 0 are the negation of the set
    vector<int> result(nDocs-negate.size());
    int res_count = 0;
    for(int i = 0; i < nDocs; i++) {
        if(count[i] == 0)
            result[res_count++] = i;
    }

    delete[] count;
    return result;
}

vector<int> evaluate(deque <token> rpn_expr, int nDocs) {
    stack<token> eval_stack;

    while (!rpn_expr.empty()) {
        token tkn = rpn_expr.front();
        rpn_expr.pop_front();

        if(tkn.type == 'a') {
            if (eval_stack.size() < 2)
                throw invalid_argument("Insufficient operands for AND!");

            // get both operands for AND
            vector<int> and1 = eval_stack.top().docs;
            eval_stack.pop();

            vector<int> and2 = eval_stack.top().docs;
            eval_stack.pop();

            // intersection of the two sets of documents
            vector<int> set_inter;
            set_intersection(and1.begin(), and1.end(), and2.begin(), and2.end(), back_inserter(set_inter));

            printf("intersection ");
            for(int x : set_inter)
                printf("%i ", x);
            printf("\n");

            token res;
            res.type = 'r';
            res.docs = set_inter;

            eval_stack.push(res);

        } else if(tkn.type == 'o') {
            if (eval_stack.size() < 2)
                throw invalid_argument("Insufficient operands for OR!");

            // get both operands for OR
            vector<int> or1 = eval_stack.top().docs;
            eval_stack.pop();

            vector<int> or2 = eval_stack.top().docs;
            eval_stack.pop();

            // union of the two sets of documents
            vector<int> set_un;
            set_union(or1.begin(), or1.end(), or2.begin(), or2.end(), back_inserter(set_un));

            printf("union ");
            for(int x : set_un)
                printf("%i ", x);
            printf("\n");

            token res;
            res.type = 'r';
            res.docs = set_un;

            eval_stack.push(res);

        } else if(tkn.type == 'n') {
            if (eval_stack.size() < 1)
                throw invalid_argument("Insufficient operands for NOT!");

            vector<int> negate = eval_stack.top().docs;
            eval_stack.pop();
            
            // difference between all docs and the docs we don't want
            vector<int> set_diff = get_not_docs(nDocs, negate);

            printf("not ");
            for(int x : set_diff)
                printf("%i ", x);
            printf("\n");

            token res;
            res.type = 'r';
            res.docs = set_diff;
            
            eval_stack.push(res);
            
        } else {
            eval_stack.push(tkn);
        }
    }

    if (eval_stack.size() != 1) {
        string err = "Wrong number of operands left: " + eval_stack.size();
        throw invalid_argument(err);
    }

    return eval_stack.top().docs;
}
