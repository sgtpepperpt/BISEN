#include "QueryEvaluator.hpp"

using namespace std;

vector<int> evaluate(deque <token> rpn_expr)
{
	stack<token> eval_stack;

	while (!rpn_expr.empty()) {
		token tkn = rpn_expr.front();
		rpn_expr.pop_front();

		if(tkn.type == 'a') {
			if (eval_stack.size() < 2)
				throw invalid_argument("Insufficient operands for AND!");

			// get both operands for AND
			vector<int> left = eval_stack.top().docs;
			eval_stack.pop();

			vector<int> right = eval_stack.top().docs;
			eval_stack.pop();
			
			// intersection of the two sets of documents
			vector<int> set_inter;
			set_intersection(left.begin(), left.end(), right.begin(), right.end(), back_inserter(set_inter));

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
			vector<int> left = eval_stack.top().docs;
			eval_stack.pop();

			vector<int> right = eval_stack.top().docs;
			eval_stack.pop();

			// union of the two sets of documents
			vector<int> set_un;
			set_union(left.begin(), left.end(), right.begin(), right.end(), back_inserter(set_un));

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

			vector<int> left = eval_stack.top().docs;
			eval_stack.pop();

			// TODO get all documents
			//eval_stack.push(!left.compare("1")?"1":"0");
			
		} else {
			eval_stack.push(tkn);
		}
	}

	if (eval_stack.size() != 1) {
		throw invalid_argument("Wrong number of operands left: " + eval_stack.size());
	}

	return eval_stack.top().docs;
}
