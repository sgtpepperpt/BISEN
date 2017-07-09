#include "QueryEvaluator.hpp"

using namespace std;

vector<int> evaluate(deque <token> rpn_expr)
{
	stack<token> eval_stack;
	vector<int> result;

	while (!rpn_expr.empty()) {
		token tkn = rpn_expr.front();
		rpn_expr.pop_front();

		if(tkn.type == 'a') {
			if (eval_stack.size() < 2)
				throw invalid_argument("Insufficient operands for AND!");

			token left = eval_stack.top();
			eval_stack.pop();

			token right = eval_stack.top();
			eval_stack.pop();
			
			printf("intersection ");
			std::vector<int> v_intersection;
			std::set_intersection(left.docs.begin(), left.docs.end(),
                          right.docs.begin(), right.docs.end(),
                          std::back_inserter(v_intersection));
                          
			for(int x : v_intersection)
        		printf("%i ", x);
    		printf("\n");
    		
    		token res;
    		res.type = 'r';
    		
    		std::set<int> s(v_intersection.begin(), v_intersection.end());
    		res.docs = s;
    		
    		eval_stack.push(res);
			
		} else if(tkn.type == 'o') {
			if (eval_stack.size() < 2)
				throw invalid_argument("Insufficient operands for OR!");

			token left = eval_stack.top();
			eval_stack.pop();

			token right = eval_stack.top();
			eval_stack.pop();

			std::vector<int> v_intersection;
			std::set_intersection(left.docs.begin(), left.docs.end(),
                          right.docs.begin(), right.docs.end(),
                          std::back_inserter(v_intersection));
                          
			
		} else if(tkn.type == 'n') {
			if (eval_stack.size() < 1)
				throw invalid_argument("Insufficient operands for NOT!");

			token left = eval_stack.top();
			eval_stack.pop();

			//eval_stack.push(!left.compare("1")?"1":"0");
			
		} else {
			eval_stack.push(tkn);
		}
	}

	if (eval_stack.size() != 1) {
		throw invalid_argument("Wrong number of operands left: " + eval_stack.size());
	}

	copy(eval_stack.top().docs.begin(), eval_stack.top().docs.end(), back_inserter(result));
	return result;
}
