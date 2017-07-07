#include "QueryEvaluator.hpp"

using namespace std;

int evaluate(queue <std::string> rpn_expr)
{
	stack <string> eval_stack;

	while (!rpn_expr.empty()) {
		string token = rpn_expr.front();
		rpn_expr.pop();

		if(!token.compare(AND)) {
			if (eval_stack.size() < 2)
				throw invalid_argument("Insufficient operands for AND!");

			string left = eval_stack.top();
			eval_stack.pop();

			string right = eval_stack.top();
			eval_stack.pop();

			eval_stack.push((!left.compare("0") && !right.compare("0"))?"0":"1");
			
		} else if(!token.compare(OR)) {
			if (eval_stack.size() < 2)
				throw invalid_argument("Insufficient operands for OR!");

			string left = eval_stack.top();
			eval_stack.pop();

			string right = eval_stack.top();
			eval_stack.pop();

			eval_stack.push((!left.compare("0") || !right.compare("0"))?"0":"1");
			
		} else if(!token.compare(NOT)) {
			if (eval_stack.size() < 1)
				throw invalid_argument("Insufficient operands for NOT!");

			string left = eval_stack.top();
			eval_stack.pop();

			eval_stack.push(!left.compare("1")?"1":"0");
			
		} else {
			eval_stack.push(token);
		}
	}

	if (eval_stack.size() != 1) {
		throw invalid_argument("Wrong number of operands left: " + eval_stack.size());
	}

	return eval_stack.top().compare("0");
}
