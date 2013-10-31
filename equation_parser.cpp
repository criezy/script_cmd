/*
 * Copyright (C) 2000, 2013 Thierry Crozat
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Contact: criezy01@gmail.com
 */

#include "equation_parser.h"
#include "parser_operators.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

String EquationParser::nullStr_;

/*! \fn EquationParser::EquationParser()
 *
 * Constructor of the EquationParser class.
 */
EquationParser::EquationParser() :
	expression_(NULL), auto_add_args_(false), max_nb_args_(0),
	args_double_(NULL), start_point_(NULL)
{
}

/*! \fn EquationParser::~EquationParser()
 *
 * Destructor of the EquationParser class.
 */
EquationParser::~EquationParser() {
	if (start_point_)
		delete start_point_;
	clearArguments();
	errors_.clear();
	equation_.clear();
}

void EquationParser::clearArguments() {
	if (args_double_) {
		delete [] args_double_;
		args_double_ = NULL;
	}
	args_names_.clear();
	max_nb_args_ = 0;
}

/*! \fn double EquationParser::evaluate(double *arg)
 *
 * Evaluate an equation that has been previously parsed. The equation variables
 * values can be given in argument. If not they should have been set in the array
 * returned by variablesDouble(). The computation is done in double precision.
 */
double EquationParser::evaluate(double *arg) {
	if (!start_point_)
		return 0.;
	if (arg != 0)
		memcpy(args_double_, arg, args_names_.size() * sizeof(double));
	double result = start_point_->evaluate();
	if (arg != 0)
		memcpy(arg, args_double_, args_names_.size() * sizeof(double)); // in case it has been modified
	return result;
}

// Parser

/*! \fn bool EquationParser::parse(const String& equation, const StringList& variables_names, bool auto_add_variables = false)
 *
 * Parse an equation.
 *
 * This has to be done before calling evaluate(). You can give
 * the names of the variables used in the script and when the
 * parser find an unrecognized name it will return a parse error.
 * Otherwise, by setting auto_add_variables to true, you can let
 * the parser find the variables for you. In this mode, each time
 * the parser find a name that he thinks is an unknown variable,
 * it will create a variable for you. You can both give some variable
 * names and let the parser find other variables used in the script.
 */
bool EquationParser::parse(
	const String& equation,
	const StringList& variables_names,
	bool auto_add_variables
) {
	if (start_point_) {
		delete start_point_;
		start_point_ = NULL;
	}
	clearArguments();
	errors_.clear();
	expression_ = NULL;
	equation_ = equation;
	if (equation_.isEmpty()) {
		equation_ = NULL;
		return false;
	}

	max_nb_args_ = variables_names.size();
	auto_add_args_ = auto_add_variables;
	if (auto_add_args_)
		max_nb_args_ += 50;
	if (max_nb_args_ > 0)
		args_double_ = new double[max_nb_args_];
	args_names_ = variables_names;

	expression_ = equation_.c_str();

	getToken();
	if (!*token_) {
		syntaxError(2);
		return false;
	}
	start_point_ = eval_exp();
	if (*token_ != 0 && start_point_) {
		delete start_point_;
		start_point_ = NULL;
		syntaxError(0);
	}
	return (start_point_ != NULL);
}

void EquationParser::getToken() {
	register char *temp;
	token_type_ = EquationParser::NONE;
	temp = token_;
	*temp = '\0';

	if (*expression_ == 0)
		return; // at end of expression_
	while (isspace(*expression_))
		++expression_; // skip over white space

	if (*expression_ == '"') {
		token_type_ = EquationParser::STRING;
		++expression_;
		while (*expression_ != '"' && *expression_ != 0)
			*temp++ = *expression_++;
		if (*expression_ != '"')
			syntaxError(8);
		else
			++expression_;
	} else if (isdelim(*expression_)) {
		token_type_ = EquationParser::DELIMITER;
		// advance to next char
		*temp++ = *expression_++;
		// some delimiter are on two chars, but we do not increment equation
		*temp++ = *expression_;
	} else if (isalpha(*expression_)) {
		while (!isdelim(*expression_))
			*temp++ = *expression_++;
		while (isspace(*expression_))
			++expression_; // skip over white space
		if (*expression_ == '(')
			token_type_ = EquationParser::FUNCTION;
		else
			token_type_ = EquationParser::VARIABLE;
	} else if (isdigit(*expression_) || *expression_ == '.') {
		while(
			!isdelim(*expression_) || (
				(*(expression_-1) == 'e' || *(expression_-1) == 'E') &&
				(*expression_ == '+' || *expression_ == '-')
			)
		)
			*temp++ = *expression_++;
		token_type_ = EquationParser::NUMBER;
	}
	*temp = '\0';
}

// Operator && and ||
ParserOperator *EquationParser::eval_exp() {
	register char op;
	ParserOperator *lop = eval_exp1();
	if (lop == 0)
		return NULL;
	while (((op = *token_) == '&' && *(token_+1) == '&') || (op == '|' && *(token_+1) == '|')) {
		getToken();
		getToken();
		ParserOperator *rop = eval_exp1();
		if (rop == 0) {
			delete lop;
			return NULL;
		}
		switch(op) {
			case '&':
				lop = new AndOperator(lop, rop);
				break;
			case '|':
				lop = new OrOperator(lop, rop);
				break;
		}
	}
	return lop;
}

// Operator =, ==, !=, <, <, <=, >=
ParserOperator *EquationParser::eval_exp1() {
	register char op1, op2 = ' ';
	ParserOperator *lop = eval_exp2();
	if (lop == 0)
		return NULL;
	op2 = *(token_+1); // make sure this is initialized
	while (((op1 = *token_) == '!' && op2 == '=') || op1 == '=' || op1 == '<' || op1 == '>') {
		getToken();
		if (op2 == '=')
			getToken();
		ParserOperator *rop = eval_exp2();
		if (rop == 0) {
			delete lop;
			return NULL;
		}
		switch (op1) {
			case '!':
				lop = new NotEqualOperator(lop, rop);
				break;
			case '=':
				if (op2 == '=')
					lop = new EqualOperator(lop, rop);
				else {
					if (dynamic_cast<VariableOperator*>(lop) == NULL) {
						delete lop;
						syntaxError(9);
						return NULL;
					}
					lop = new AssignmentOperator(lop, rop);
				}
				break;
			case '<':
				if (op2 == '=')
					lop = new EqualOrSmallerOperator(lop, rop);
				else
					lop = new SmallerOperator(lop, rop);
				break;
			case '>':
				if (op2 == '=')
					lop = new EqualOrGreaterOperator(lop, rop);
				else
					lop = new GreaterOperator(lop, rop);
				break;
		}
	}
	return lop;
}

// Add or subtract two terms.
ParserOperator *EquationParser::eval_exp2() {
	register char op;
	ParserOperator *lop = eval_exp3();
	if (lop == 0)
		return NULL;
	while ((op = *token_) == '+' || op == '-') {
		getToken();
		ParserOperator *rop = eval_exp3();
		if (rop == 0) {
			delete lop;
			return NULL;
		}
		switch(op) {
			case '-':
				lop = new MinusOperator(lop, rop);
				break;
			case '+':
				lop = new PlusOperator(lop, rop);
				break;
		}
	}
	return lop;
}

// Multiply or divide two factors.
ParserOperator *EquationParser::eval_exp3() {
	register char op;
	ParserOperator *lop = eval_exp4();
	if (lop == 0)
		return NULL;
	while ((op = *token_) == '*' || op == '/') {
		getToken();
		ParserOperator *rop = eval_exp4();
		if (rop == 0) {
			delete lop;
			return NULL;
		}
		switch (op) {
			case '*':
				lop = new MultiplyOperator(lop, rop);
				break;
			case '/':
				lop = new DivideOperator(lop, rop);
				break;
		}
	}
	return lop;
}

// Process ^ (power) operator.
ParserOperator *EquationParser::eval_exp4() {
	register char op;
	ParserOperator *lop = eval_exp5();
	if (lop == 0)
		return NULL;
	while ((op = *token_) == '^') {
		getToken();
		ParserOperator *rop = eval_exp5();
		if (rop == 0) {
			delete lop;
			return NULL;
		}
		lop = new PowOperator(lop, rop);
	}
	return lop;
}

// Process a unary + or -
ParserOperator *EquationParser::eval_exp5() {
	register char op;
	op = 0;
	if ((token_type_ == EquationParser::DELIMITER) && (*token_ == '+' || *token_ == '-')) {
		op = *token_;
		getToken();
	}
	ParserOperator *lop = eval_exp6();
	if (lop == 0)
		return NULL;
	if (op == '-')
		lop = new NSignOperator(lop);
	return lop;
}

// Process a parenthesized expression
ParserOperator *EquationParser::eval_exp6() {
	if ((*token_ == '(')) {
		getToken();
		if ((*token_ == ')')) {
			syntaxError(4);
			return NULL;
		}
		ParserOperator *pop = eval_exp();
		if (*token_ != ')') {
			syntaxError(1);
			if (pop != NULL) {
				delete pop;
				pop = NULL;
			}
		}
		getToken();
		return pop;
	} else return eval_exp7();
}

// Process functions, constant number and variable
ParserOperator *EquationParser::eval_exp7() {
	ParserOperator *result = NULL;
	switch (token_type_) {
		case EquationParser::NUMBER:
			result = new ConstantOperator(atof(token_));
			break;
		case EquationParser::VARIABLE: {
				// Built-in variables
				if (strcmp(token_, "PI") == 0) {
					result = new ConstantOperator(M_PI);
					break;
				}
				// look if variable exists
				String var_name(token_);
				int arg = args_names_.indexOf(var_name);
				if (arg == -1) {
					if (auto_add_args_) {
						if (args_names_.size() < max_nb_args_) {
							arg = args_names_.size();
							args_double_[arg] = 0.;
							args_names_ << var_name;
						} else {
							syntaxError(7);
						}
					} else {
						syntaxError(6);
					}
				}
				if (arg != -1)
					result = new VariableOperator(args_double_ + arg, var_name);
			}
			break;
		case EquationParser::FUNCTION: {
				if (strcmp(token_, "print") == 0) {
					getToken(); // skip (
					getToken();
					List<ParserOperator*> values;
					StringList strings;
					do {
						if (token_type_ == EquationParser::STRING) {
							values << NULL;
							strings << String(token_);
							getToken();
						} else {
							ParserOperator *pop = eval_exp();
							if (pop != NULL)
								values << pop;
							else
								break;
						}
						if (*token_ != ',')
							break;
						getToken();
					} while (1);
					if (!values.isEmpty())
						result = new PrintOperator(values, strings);
				} else if (strcmp(token_, "cos") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new CosOperator(pop);
				} else if (strcmp(token_, "sin") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new SinOperator(pop);
				} else if (strcmp(token_, "tan") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new TanOperator(pop);
				} else if (strcmp(token_, "sqrt") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new SqrtOperator(pop);
				} else if (strcmp(token_, "cbrt") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new CbrtOperator(pop);
				} else if (strcmp(token_, "exp") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new ExpOperator(pop);
				} else if (strcmp(token_, "pow") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *lop = eval_exp();
					if (lop) {
						if (*token_ != ',') delete lop;
						else {
							getToken();
							ParserOperator *rop = eval_exp();
							if (!rop) delete lop;
							else result = new PowOperator(lop,rop);
						}
					}
				} else if (strcmp(token_, "ceil") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new CeilOperator(pop);
				} else if (strcmp(token_, "floor") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new FloorOperator(pop);
				} else if (strcmp(token_, "fabs" ) == 0 || strcmp(token_, "abs" ) == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new FAbsOperator(pop);
				} else if (strcmp(token_, "log10") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new Log10Operator(pop);
				} else if (strcmp(token_, "log") == 0 || strcmp(token_, "ln") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new LogOperator(pop);
				} else if (strcmp(token_, "asin") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new ASinOperator(pop);
				} else if (strcmp(token_, "acos") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new ACosOperator(pop);
				} else if (strcmp(token_, "atan") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new ATanOperator(pop);
				} else if (strcmp(token_, "sinh") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new SinHOperator(pop);
				} else if (strcmp(token_, "cosh") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new CosHOperator(pop);
				} else if (strcmp(token_, "tanh") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *pop = eval_exp();
					if (pop != NULL)
						result = new TanHOperator(pop);
				} else if (strcmp(token_, "if") == 0) {
					getToken(); // skip (
					getToken();
					ParserOperator *test = eval_exp();
					if (test) {
						if (*token_ != ',')
							delete test;
						else {
							getToken();
							ParserOperator *lop = eval_exp();
							if (lop == NULL || *token_ != ',') {
								delete test;
								delete lop;
							} else {
								getToken();
								ParserOperator *rop = eval_exp();
								if (rop == NULL) {
									delete test;
									delete lop;
								} else
									result = new IfOperator(test, lop, rop);
							}
						}
					}
				} else {
					syntaxError(5);
					getToken(); // skip (
					break;
				}
				if (*token_ != ')') {
					syntaxError(1);
					if (result) {
						delete result;
						result = NULL;
					}
				}
			}
			break;
		default:
			syntaxError(3);
			break;
	}
	getToken();
	return result;
}


bool EquationParser::isdelim(char c) {
	if (strchr(" +-/*^(),=!<>|&", c) || c == 9 || c == '\r' || c == 0)
		return 1;
	return 0;
}

void EquationParser::syntaxError(int type) {
	String error;
	switch(type) {
		case 0:
			error = String::format("Syntax error near %s", token_);
			break;
		case 1:
			error = "Unbalanced Parentheses";
			break;
		case 2:
			error = "No expression Present";
			break;
		case 3:
			if (token_type_ == EquationParser::STRING)
				error = "Strings are only supported in print() functions";
			else if (*token_ == 0)
				error = "Unexpected end of equation";
			else {
				// Delimiter can have one or two characters, so make sure we don't print the second one
				// if it is not a delimiter.
				if (token_type_ == EquationParser::DELIMITER && !isdelim(token_[1]))
					token_[1] = 0;
				error = String::format("Unexpected token: %s", token_);
			}
			break;
		case 4:
			error = "Empty Parentheses";
			break;
		case 5:
			error = String::format("Unkown function: %s()", token_);
			break;
		case 6:
			error = String::format("Unkown variable: %s", token_);
			break;
		case 7:
			error = String::format("Detected variable %s but maximum number of unknown variables has been reached", token_);
			break;
		case 8:
			error = "Unbalanced quotes";
			break;
		case 9:
			error = "Non assignable statement on left of = operator.";
			break;
	}

	if (!error.isEmpty())
		errors_ << error;
}

