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
#include "modules.h"
#include <math.h>
#include <ctype.h>
#include <stdio.h>
#ifdef USE_READLINE
#include <readline/readline.h>
#endif

void printEquationModuleHelp(int mode) {
	switch (mode) {
	case 1:
		printf("The recognized constants are:\n");
		printf("  - PI        %f\n", M_PI);
		break;
	case 2:
		printf("The recognized functions are:\n");
		printf("  - sqrt(x)   The square root of x.\n");
		printf("  - cbrt(x)   The cubic root of x.\n");
		printf("  - pow(x, y) x raised to the power of y.\n");
		printf("  - exp(x)    The exponential of x (e raised to the power of x).\n");
		printf("  - log10(x)  The base 10 logarithm of x.\n");
		printf("  - log(x)    The natural logarithm of x.\n");
		printf("  - ln(x)     Same as log(x)\n");
		printf("  - ceil(x)   Round x up (the smallest integer bigger or equal to x).\n");
		printf("  - floor(x)  Round x down (the biggest integer smaller or equal to x).\n");
		printf("  - fabs(x)   Absolute value of x (x if x is positive, -x otherwise).\n");
		printf("  - abs(x)    Same as fabs(x)\n");
		printf("  - cos(x)    The cosine of x.\n");
		printf("  - sin(x)    The sine of x.\n");
		printf("  - tan(x)    The tangent of x.\n");
		printf("  - asin(x)   The arc sine of x.\n");
		printf("  - acos(x)   The arc cosine of x.\n");
		printf("  - atan(x)   The arc tangent of x.\n");
		printf("  - sinh(x)   The hyperbolic sine of x.\n");
		printf("  - cosh(x)   The hyperbolic cosine of x.\n");
		printf("  - tanh(x)   The hyperbolic tangent of x.\n");
		printf("  - if (x, y, z) If x is true (not equal to zero) return y, otherwise return z.\n");
		printf("  - print(x [, y, \"text\", z...])  Print the passed values and strings.\n");
		break;
	case 3:
		printf("The recognized operators are:\n");
		printf("  - x + y     Add y to x.\n");
		printf("  - x - y     Substract y from x.\n");
		printf("  - x * y     Multiply x and y.\n");
		printf("  - x / y     Divide x by y.\n");
		printf("  - x^y       Raise x to the power of y (same as pow(x, y)).\n");
		printf("  - +x or -x  Unary plus and minus operators (change the sign of x).\n");
		printf("  - x && y    Logical and (return true - i.e. 1 - if x and y are not null).\n");
		printf("  - x || y    Logical or (return true - i.e. 1 - if either x or y - or both - is not null).\n");
		printf("  - x == y    Test equality (return true - i.e. 1 - if x is equal to y).\n");
		printf("  - x != y    Test inequality (return true - i.e. 1 - if x and y are different).\n");
		printf("  - x < y     Return true if x is smaller than y.\n");
		printf("  - x <= y    Return true if x is smaller or equal than y.\n");
		printf("  - x > y     Return true if x is greater than y.\n");
		printf("  - x >= y    Return true if x is greater or equal than y.\n");
		printf("  - x = y     Asignement (set x to y and return y).\n");
		break;
	case 4:
		printf("The recognized debug commands are:\n");
		printf("  - tree <equation>  Prints the parser tree for the given equation.\n");
		break;
	default:
		printf("Evaluates C-like mathematical expressions.\n");
		printf("Type 'help topic' to get help on a specific topic.\n");
		printf("Topics are: 'constants', 'functions', 'operators', 'debug'\n");
		printf("Type 'quit' or 'exit' to quit the program.\n");
		break;
	}
}

String readLine(const char* prompt, bool strip_eol) {
#ifdef USE_READLINE
	char *line = readline(prompt);
	String str = line;
	if (line != NULL && *line != 0)
		add_history(line);
	free(line);
	if (!strip_eol)
	str += "\n";
	return str;
#else
	if (prompt != NULL && *prompt != 0)
		printf("%s", prompt);
	return readLine(strip_eol, stdin);
#endif
}

String readLine(bool strip_eol, FILE* stream) {
	char buffer[256];
	String line;
	while (!feof(stream) && (line.isEmpty() || line[line.length() - 1] != '\n')) {
		if (fgets(buffer, 256, stream) != NULL)
			line += buffer;
	}
	if (strip_eol)
		return line.left(-2);
	return line;
}

void runEquationModule() {
	printf("Starting simple equation mode.\nType 'help' to get some help.\n");
	EquationParser parser;
	StringList variables;
	while (1) {
		// Read a line
		String line = readLine("> ");
		if (line.isEmpty())
			continue;

		// Check if it is exit
		if (line == "exit" || line == "quit")
			break;

		// Check if it is help
		if (line.startsWith("help") && (line.length() == 4 || isspace(line[4]))) {
			String topic = line.right(4).trimmed();
			int t = 0;
			if (topic == "constants")
				t = 1;
			else if (topic == "functions")
				t = 2;
			else if (topic == "operators")
				t = 3;
			else if (topic == "debug")
				t = 4;
			printEquationModuleHelp(t);
			continue;
		}

		// Handle Equation
#ifdef PARSER_TREE_DEBUG
		bool tree_debug = false;
#endif
		String equation;
		if (line.startsWith("tree") && (line.length() == 4 || isspace(line[4]))) {
#ifdef PARSER_TREE_DEBUG
			equation = line.right(4).trimmed();
			tree_debug = true;
#else
			printf("The parser tree debugging feature is disabled. You need to recompile the executable with the feature enabled before you can use it.\n");
#endif
		} else
			equation = line;
		if (equation.isEmpty())
			continue;

		// Parse the equation
		if (!parser.parse(equation, variables)) {
			if (parser.nbErrors() == 0)
				printf("Syntax error...\n");
			else {
				printf("Equation contains %d error(s):\n", parser.nbErrors());
				for (int error = 0 ; error < parser.nbErrors() ; ++error)
					printf("  %d: %s\n", error+1, parser.getError(error).c_str());
			}
			continue;
		}
#ifdef PARSER_TREE_DEBUG
		if (tree_debug)
			EquationParser::debugPrint(parser.getParserTreeDescription());
#endif
		printf("%.12g\n", parser.evaluate());
	}
}
