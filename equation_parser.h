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

#ifndef equation_parser_h
#define equation_parser_h

#include <stdlib.h>
#include "str.h"
#include "strlist.h"

class ParserOperator;

/*! \class EquationParser
 *
 * Parser class. This is the only class that you need to take care about.
 * To parse an equation, call the parse() method. To evaluate a parsed equation,
 * call the evaluateDouble() or evaluateFloat() method.
 * If you have arguments, you either need to give them as arguments to the evaluateDouble()
 * or evaluateFloat() functions or you need to set them using the variables_dbl() or
 * variables_flt() functions.
 *
 * This parser uses the C syntax for equations.
 *
 * The recognized constants are:
 *   - PI
 *
 * The recognized functions are:
 *   - sqrt(x), the square root of x.
 *   - cbrt(x), the cubic root of x.
 *   - pow(x, y), x raised to the power of y.
 *   - exp(x), the exponential of x (e raised to the power of x).
 *   - log10(x), the base 10 logarithm of x.
 *   - log(x) or ln(x), the natural logarithm of x.
 *   - ceil(x), round x up (the smallest integer bigger or equal to x).
 *   - floor(x), round x down (the biggest integer smaller or equal to x).
 *   - fabs(x) or abs(x), absolute value of x (x if x is positive, -x otherwise).
 *   - sign(x), the sign of x (-1 for a negative value, 1 otherwise).
 *   - sin(x), the sine of x.
 *   - cos(x), the cosine of x.
 *   - tan(x), the tangent of x.
 *   - asin(x), the arc sine of x.
 *   - acos(x), the arc cosine of x.
 *   - atan(x), the arc tangent of x.
 *   - atan2(y, x), the angle between the x axis and the segment defined by the points (0,0) and (x,y), which corresponds to the phase of the complex number x + iy.
 *   - sinh(x), the hyperbolic sine of x.
 *   - cosh(x), the hyperbolic cosine of x.
 *   - tanh(x), the hyperbolic tangent of x.
 *   - asinh(x), the arc hyperbolic sine of x.
 *   - acosh(x), the arc hyperbolic cosine of x.
 *   - atanh(x), the arc hyperbolic tangent of x.
 *   - degToRad(x), converts an angle in degrees to radians.
 *   - radToDeg(x), converts an angle in radians to degrees.
 *   - min(value1, value2), the minimum of the two given values.
 *   - max(value1, value2), the maximum of the two given values.
 *   - if (x, y, z), if x is true (not equal to zero) return y, otherwise return z.
 *   - print(x), print the value of x and return that value.
 *
 * The recognized operators are:
 *   - x + y, add y to x.
 *   - x - y, substract y from x.
 *   - x * y, multiply x and y.
 *   - x / y, divide x by y.
 *   - x^y, raise x to the power of y (same as pow(x, y)).
 *   - +x or -x, unary plus and minus operators (change the sign of x).
 *   - x && y, logical and (return true - i.e. 1 - if x and y are not null).
 *   - x || y, logical or (return true - i.e. 1 - if either x or y - or both - is not null).
 *   - x == y, test equality (return true - i.e. 1 - if x is equal to y).
 *   - x != y, test inequality (return true - i.e. 1 - if x and y are different).
 *   - x < y, return true if x is smaller than y.
 *   - x <= y, return true if x is smaller or equal than y.
 *   - x > y, return true if x is greater than y.
 *   - x >= y, return true if x is greater or equal than y.
 *   - x = y, asignement (set x to y and return y).
 *   - x += y, add y to x and store the result in x.
 *   - x -= y, substract y from x and store the result in y.
 *   - x *= y, multiply x and y and store the result in y.
 *   - x /= y, divide x by y and store the result in y.
 *   - ++x, prefix increment operator (add 1 to x).
 *   - --x, prefix decrement operator (remove 1 from x).
 *
 * And you can of course use parenthesis to make sure the expressions are evaluated in the
 * order than you want them to be. The priority of the operators is the same than in C.
 *
 * Example:
 * \code
	double result = 0.;
	double variable[2] = { 1. , 2. };
	StringList var_names;
	var_names << "x" << "y";
	EquationParser parser;
	if (parser.parse("x + 2 * y", var_names))
		result = parser.evaluate(variable);
 * \endcode
 *
 * The same code could also be written as follow:
 * \code
	double result = 0.;
	StringList var_names;
	var_names << "x" << "y";
	EquationParser parser;
	if (parser.parse("x + 2 * y", var_names)) {
		parser.variablesValue()[0] = 1.;
		parser.variablesValue()[1] = 2.;
		result = parser.evaluate();
	}
 * \endcode
 */
class EquationParser {
	enum TokenType { DELIMITER , VARIABLE , FUNCTION , NUMBER , STRING, NONE };
public:
	EquationParser();
	~EquationParser();

	bool parse(
		const String& equation,
		const StringList& variables_names,
		bool auto_add_variables = false
	);

	double evaluate(double *var = NULL);

	double *variablesValue();
	int nbVariables() const;
	const StringList& variablesName() const;

	int nbErrors() const;
	const String& getError(int) const;
	const String& getLastError() const;

private:
	void getToken();
	ParserOperator *eval_exp();  // Operator && and ||
	ParserOperator *eval_exp1(); // Operator =, ==, !=, <, <, <=, >=, +=, -=, *=. /=
	ParserOperator *eval_exp2(); // Add or subtract two terms.
	ParserOperator *eval_exp3(); // Multiply or divide two factors.
	ParserOperator *eval_exp4(); // Process ^ operator.
	ParserOperator *eval_exp5(); // Process a unary + or - and prefix ++ or --
	ParserOperator *eval_exp6(); // Process a parenthesized expression , including functions.
	ParserOperator *eval_exp7(); // Process functions, constant number and variable
	bool isdelim(char c);
	void syntaxError(int type);
	void clearArguments();

private:
	// Equation parsing
	String equation_;
	const char *expression_;
	char token_[512];
	TokenType token_type_;
	bool auto_add_args_;
	// Equation evaluation
	int max_nb_args_;
	double *args_double_;
	StringList args_names_;
	ParserOperator *start_point_;
	StringList errors_;

	static String nullStr_;
};

/*! \fn double *EquationParser::variablesValue()
 * Return the variables array. The variable are sorted in the same order
 * as the one for the variables names given to the parse() function.
 * This function can be used to set the variables value in double precision
 * mode before the evaluateDouble() function is called (but after the equation
 * has been succesfully parsed):
 * \code
    parser->variablesDouble()[0] = 12.327;
    double result = parser->evaluateDouble();
 * \endcode
 */
inline double *EquationParser::variablesValue() {
	return args_double_;
}

/*! \fn int EquationParser::nbVariables() const
 *
 * Return the number of variables (given to parse() or auto-detected).
 */
inline int EquationParser::nbVariables() const {
	return args_names_.size();
}

/*! \fn const StringList& EquationParser::variablesName() const
 *
 * Return an array containing the name of the variables.
 * This can be usefull when using the 'auto-add variables'
 * features.
 */
inline const StringList& EquationParser::variablesName() const {
	return args_names_;
}

/*! \fn int EquationParser::nbErrors() const
 *
 * Get the number of error messages since the last call to parse.
 */
inline int EquationParser::nbErrors() const {
	return errors_.size();
}

/*! \fn const String& EquationParser::getError(int i) const
 *
 * Get error at index i (between 0 and nb_errors()-1).
 * Returns an empty string if the index is invalid.
 */
inline const String& EquationParser::getError(int i) const {
	if (i < 0 || i >= errors_.size())
		return nullStr_;
	return errors_[i];
}

/*! \fn const String& EquationParser::getLastError() const
 *
 * Get the last error message or 0 is there is no errors.
 */
inline const String& EquationParser::getLastError() const {
	if (errors_.isEmpty())
		return nullStr_;
	return errors_.last();
}

#endif
