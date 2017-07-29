#include "QueryEvaluator.hpp"

using namespace std;

// receives a set of docs and returns its negation in the
// full docs set; equivalent to the set differentiation:
// {all-docs} \ {negate}
vec_int get_not_docs(int nDocs, vec_int negate){
    int *count = (int*) malloc(sizeof(int) * nDocs);

    for(int i = 0; i < nDocs; i++)
        count[i] = 0;

    // increase the count for elems in the original vector
    for(unsigned i = 0; i < size(negate); i++) {
        count[negate.array[i]]++;
    }

    // all elements that have count == 0 are the negation of the set
    //vector<int> result(nDocs - negate.size());
    vec_int result;
    init(&result, nDocs);

    int res_count = 0;
    for(int i = 0; i < nDocs; i++) {
        if(count[i] == 0) {
           push_back(&result, i);
        }
    }

    free(count);
    return result;
}

// evaluates a query in reverse polish notation, returning
// the resulting set of docs
vec_int evaluate(vec_token rpn_expr, int nDocs) {
    vec_token eval_stack;
    init(&eval_stack, DEFAULT_QUERY_TOKENS);

    iee_token tkn;
    for(unsigned i = 0; i < size(rpn_expr); i++) {
        tkn = rpn_expr.array[i];

        if(tkn.type == '&') {
            if (size(eval_stack) < 2)
                printf("Insufficient operands for AND!\n");
                //throw invalid_argument("Insufficient operands for AND!");

            // get both operands for AND
            vec_int and1 = peek_back(eval_stack).docs;
            pop_back(&eval_stack);

            vec_int and2 = peek_back(eval_stack).docs;
            pop_back(&eval_stack);

            // intersection of the two sets of documents
            vec_int set_inter = vec_intersection(and1, and2);
            //set_intersection(and1.begin(), and1.end(), and2.begin(), and2.end(), back_inserter(set_inter));

            /*printf("intersection ");
            for(int i = 0; i < size(set_inter); i++)
                printf("%i ", set_inter.array[i]);
            printf("\n");*/

            iee_token res;
            res.type = 'r';
            res.docs = set_inter;

            push_back(&eval_stack, res);
        } else if(tkn.type == '|') {
            if (size(eval_stack) < 2)
                printf("Insufficient operands for OR!\n");
                //throw invalid_argument("Insufficient operands for OR!");

            // get both operands for OR
            vec_int or1 = peek_back(eval_stack).docs;
            pop_back(&eval_stack);

            vec_int or2 = peek_back(eval_stack).docs;
            pop_back(&eval_stack);

            // union of the two sets of documents
            vec_int set_un = vec_union(or1, or2);
            //set_union(or1.begin(), or1.end(), or2.begin(), or2.end(), back_inserter(set_un));

            /*printf("union ");
            for(int i = 0; i < size(set_un); i++)
                printf("%i ", set_un.array[i]);
            printf("\n");*/

            iee_token res;
            res.type = 'r';
            res.docs = set_un;

            push_back(&eval_stack, res);
        } else if(tkn.type == '!') {
            if (size(eval_stack) < 1)
                printf("Insufficient operands for NOT!\n");
                //throw invalid_argument("Insufficient operands for NOT!");

            vec_int negate = peek_back(eval_stack).docs;
            pop_back(&eval_stack);
            
            // difference between all docs and the docs we don't want
            vec_int set_diff = get_not_docs(nDocs, negate);

            /*printf("not ");
            for(int i = 0; i < size(set_diff); i++)
                printf("%i ", set_diff.array[i]);
            printf("\n");*/

            iee_token res;
            res.type = 'r';
            res.docs = set_diff;
            
            push_back(&eval_stack, res);
        } else {
            push_back(&eval_stack, tkn);
        }
    }

    if (size(eval_stack) != 1) {
        printf("Wrong number of operands left: %u\n", size(eval_stack));
        //string err = "Wrong number of operands left: " + eval_stack.size();c
        //throw invalid_argument(err);
    }

    return peek_back(eval_stack).docs;
}
