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

#include "script_parser.h"
#include "math_utils.h"
#include <ctype.h>

/*! \fn ScriptParser::ScriptParser()
 *
 * Create a ScriptParser object.
 */
ScriptParser::ScriptParser() :
	args_double_(NULL)
{
}

ScriptParser::~ScriptParser() {
	clear();
}

void ScriptParser::clear() {
	for (List<ScriptParserExpression*>::iterator it = expressions_.begin() ; it != expressions_.end() ; ++it)
		delete (*it);
	expressions_.clear();
	delete [] args_double_;
	args_double_ = NULL;
	args_names_.clear();
	errors_.clear();
}

/*! \fn bool ScriptParser::parse(const String &script, const StringList &variable_names)
 *
 * Parse the given script.
 * Parsing has to be done before calling evaluate(). If a parsing
 * error occurs this function return false. You can get the errors
 * with nbErrors() and getError(int).
 */
bool ScriptParser::parse(
	const String &script,
	const StringList &variable_names
) {
	clear();

	args_names_ = variable_names;
	if (!args_names_.isEmpty()) {
		args_double_ = new double[args_names_.size()];
		memset(args_double_, 0, args_names_.size() * sizeof(double));
	}

	breakBlock(script, expressions_, variable_names, false, errors_, args_double_);

	if (!errors_.isEmpty()) {
		for (List<ScriptParserExpression*>::iterator it = expressions_.begin() ; it != expressions_.end() ; ++it)
			delete (*it);
		expressions_.clear();
		delete [] args_double_;
		args_double_ = NULL;
		return false;
	}

	return true;
}

/*! \fn StringList ScriptParser::getVariablesList(const String &script)
 *
 * Get the list of variables from the script. This will
 * also report parsing errors, so in most cases you will
 * want to check that nbErrors() return 0 after calling
 * this function.
 */
StringList ScriptParser::getVariablesList(const String &script) {
	// We parse the script with the breakBlock() function and auto_add_variables
	// variable set to true. This is not correct because the created EquationParser
	// will use different list of variables, but at least it can be used to list all
	// the variables.
	clear();

	List<ScriptParserExpression*> expressions;
	breakBlock(script, expressions, StringList(), true, errors_);
	StringList variables;
	for (int i = 0 ; i < expressions.size() ; ++i) {
		StringList vars = expressions[i]->variablesName();
		delete expressions[i];
		for (int j = 0 ; j < vars.size() ; ++j) {
			if (!variables.contains(vars[j]))
				variables.append(vars[j]);
		}
	}

	return variables;
}

/*! \fn void ScriptParser::evaluate(double *var)
 *
 * Evaluate the last script parsed with the given variable values
 * or the variable values set in the VariablesValue() array if
 * no value array is given.
 */
void ScriptParser::evaluate(double *var) {
	if (var != NULL && args_double_ != NULL)
		memcpy(args_double_, var, args_names_.size() * sizeof(double));

	for (List<ScriptParserExpression*>::iterator it = expressions_.begin() ; it != expressions_.end() ; ++it)
		(*it)->evaluate();

	if (var != NULL && args_double_ != NULL)
		memcpy(var, args_double_, args_names_.size() * sizeof(double));
}

/*! \fn double *ScriptParser::VariablesValue()
 *
 * Return the array of double precision floating point number
 * that is used during the evaluate() call.
 */
double *ScriptParser::VariablesValue() {
	return args_double_;
}

/*! \fn const StringList &ScriptParser::variablesName() const
 *
 * Return the list of variables used in the last script parsed.
 */
const StringList &ScriptParser::variablesName() const {
	return args_names_;
}

/*! \fn int ScriptParser::nbErrors() const
 *
 * Get the number of script parsing errors.
 */
int ScriptParser::nbErrors() const {
	return errors_.size();
}

/*! \fn String ScriptParser::getError(int index) const
 *
 * Get the script parsing errors.
 */
String ScriptParser::getError(int index) const {
	if (index < 0 || index >= errors_.size())
		return String();
	return errors_[index];
}

/*! \fn String ScriptParser::getLastError() const
 *
 * Return the last script parsing error or an empty String
 * if there is no parsing errors.
 */
String ScriptParser::getLastError() const {
	return getError(nbErrors() - 1);
}

/*! \fn void ScriptParser::breakBlock(const String &script_block, List<ScriptParserExpression*> &expressions, const StringList &variable_names, bool auto_add_variables, StringList &errors, double* variable_array = NULL)
 *
 * Parse the given script block and fill the given ScriptParserExpression
 * list with expressions found in the script. If error are found during the
 * parsing they will be appended to the errors list.
 *
 * If \p auto_add_variables is false, you can pass the value array for the
 * variables. The \p variable_array should therefore be allocated to hold as
 * many value as elements in the \p variable_names list. The values will
 * also be stored in the same order as the list.
 */
void ScriptParser::breakBlock(
	const String &script_block, List<ScriptParserExpression*> &expressions,
	const StringList &variable_names, bool auto_add_variables,
	StringList &errors,
	double* variable_array
) {
	String script;
	bool is_original_script = false;
	if (!script_block.startsWith("!!")) {
		is_original_script = true;
		// Inject line numbers (used for parsing error messages
		StrReadStream stream(script_block);
		String s;
		int line = 1;
		while (!stream.atEnd()) {
			if (line > 1)
				s += '\n';
			s += String::format("!!%d\n", line) + stream.readLine();
			++line;
		}

		// remove C style comments
		// remove C++ and Shell (one line) style comments
		// add new line after ;
		// add new line after and before } and {
		// simplify new lines
		// This way each line only contains one expression.
		// This also means that if { ... } else { ... } construct will look like
		// if
		// {
		// ....
		// }
		// else
		// {
		// ....
		// }
		// whatever the original way if was formatted
		int i = 0, length = s.length();
		while (i < length) {
			char c = s[i];
			if (i+1 < length && c == '/' && s[i+1] == '*') {
				// C style comment
				i += 4;
				while (i < length && (s[i-2] != '*' || s[i-1] != '/')) {
					// keep line numbers
					if (s[i-2] == '!' && s[i-1] == '!') {
						int j = i;
						while (j < length && s[j] != '\n') {
							if (!isdigit(s[j]))
								j = length;
						}
						if (j < length) {
							script += s.mid(i-2, j);
							i = j+1;
						}
					}
					// skip character
					++i;
				}
			} else if (i+1 < length && c == '/' && s[i+1] == '/') {
				// C++ style comment
				i += 2;
				while (i < length && s[i] != '\n')
					++i;
			} else if (c == '#') {
				// Shell style comment
				++i;
				while (i < length && s[i] != '\n')
					++i;
			} else {
				if (c == '{' || c == '}')
					script += '\n';
				script += c;
				if (c == '{' || c == '}' || c == ';')
					script += '\n';
				++i;
			}
		}
	} else
		script = script_block;

	// Simplify new lines and spaces
	script.simplify('\n');
	script.replaceChar('\t', ' ');
	script.simplify(' ');

	// Parsing state
	// 0: normal
	// 1: leaving 'if' block (=>check if we have a 'else' or 'else if' block starting)
	// 2: leaving 'else if' block (=>check if we have a 'else' or 'else if' block starting)
	// 3: leaving 'else' block
	int state = 0;

	String if_condition, else_if_condition, if_block, else_block, expression;
	bool conditional_else_block = false;

	// Use a StrReadStream to read the script line by line
	int line_number = 1;
	StrReadStream stream(script);
	while (!stream.atEnd()) {
		String line = stream.readLine().trimmed();

		// Check there is something on the line
		if (line.isEmpty())
			continue;

		if (line.startsWith("!!")) {
			line_number = line.right(2).toInt();
			continue;
		}

		// Check if we are leaving a block
		if (line == "}") {
			errors << String::format("Script parsing error line %d: unexpected '}'.",line_number);
			return;
		}

		// If we have left a 'if', 'else if' block check if we have something more
		// and complete the conditional expression if needed.
		if (state == 1 || state == 2) {
			if (line == "else") {
				// Look for start of block
				do {
					line = stream.readLine().trimmed();
					if (line.startsWith("!!")) {
						line_number = line.right(2).toInt();
						line.clear();
					}
				} while (line.isEmpty() && !stream.atEnd());
				if (line != "{") {
					errors << String::format("Script parsing error line %d : '{' expected after 'else'.", line_number);
					return;
				}
				// Read block
				if (conditional_else_block)
					else_block += " else {\n";
				if (!readBlock(else_block, stream, line_number, errors))
					return;
				if (conditional_else_block)
					else_block += "}\n";
				state = 3;
				continue;

			} else if (line.startsWith("else if")) {
				// Look for start of condition
				line = line.right(7).trimmed();
				while (line.isEmpty() && !stream.atEnd()) {
					line = stream.readLine().trimmed();
					if (line.startsWith("!!")) {
						line_number = line.right(2).toInt();
						line.clear();
					}
				}
				if (line[0] != '(') {
					errors << String::format("Script parsing error line %d: '(' expected after 'else if'.", line_number);
					return;
				}
				// Read condition
				line = line.right(1);
				if (!readCondition(else_if_condition, stream, line, line_number, errors))
					return;
				// Add condition
				if (else_block.isEmpty())
					else_block += String::format("!!%d\nif (", line_number) + else_if_condition + ") {\n";
				else {
					else_block += "else if (" + else_if_condition + ") {\n";
				}
				else_if_condition.clear();
				// Read block
				if (!readBlock(else_block, stream, line_number, errors))
					return;
				// Add a '}' to match the added 'if (...) {' at the beginning of the else block
				else_block += "}\n";
				state = 2;
				conditional_else_block = true;
				continue;

			} else
				state = 3;

		}
		// If we are at the end of a conditional expression, create it and
		// start normal reading
		if (state == 3) {
			ScriptParserExpression *exp = 0;
			if (else_block.isEmpty())
				exp = new ScriptParserConditionalExpression(if_condition, if_block, variable_names, auto_add_variables, variable_array);
			else
				exp = new ScriptParserConditionalExpression(if_condition, if_block, else_block, variable_names, auto_add_variables, variable_array);
			expressions.append(exp);
			for (int e = 0 ; e < exp->nbErrors() ; ++e)
				errors.append(exp->getError(e));
			if_condition.clear();
			if_block.clear();
			else_block.clear();
			conditional_else_block = false;
			state = 0;
		}

		// Normal reading
		if (state == 0) {
			// Checking if we start a if statement
			if (line.startsWith("if") && (line.length() == 2 || line.right(2).trimmed()[0] == '(')) {
				if (!expression.isEmpty()) {
					errors << String::format("Script parsing error line %d: missing ';' before 'if'.", line_number);
					return;
				}
				line = line.right(2).trimmed();
				while (line.isEmpty() && !stream.atEnd()) {
					line = stream.readLine().trimmed();
					if (line.startsWith("!!")) {
						line_number = line.right(2).toInt();
						line.clear();
					}
				}
				if (line[0] != '(') {
					errors << String::format("Script parsing error line %d: '(' expected after 'if'.", line_number);
					return;
				}
				line = line.right(1);
				if (!readCondition(if_condition, stream, line, line_number, errors))
					return;
				// Read block
				if (!readBlock(if_block, stream, line_number, errors))
					return;
				state = 1;
				continue;
			}
			// Checking if we start a while statement
			else if (line.startsWith("while") && (line.length() == 5 || line.right(5).trimmed()[0] == '(')) {
				if (!expression.isEmpty()) {
					errors << String::format("Script parsing error line %d: missing ';' before 'while'.", line_number);
					return;
				}
				line = line.right(5).trimmed();
				while (line.isEmpty() && !stream.atEnd()) {
					line = stream.readLine().trimmed();
					if (line.startsWith("!!")) {
						line_number = line.right(2).toInt();
						line.clear();
					}
				}
				if (line[0] != '(') {
					errors << String::format("Script parsing error line %d: '(' expected after 'while'.", line_number);
					return;
				}
				// Read condition
				line = line.right(1);
				String while_condition;
				if (!readCondition(while_condition, stream, line, line_number, errors))
					return;
				// Read block
				String while_block;
				if (!readBlock(while_block, stream, line_number, errors))
					return;
				// Create while parser object
				ScriptParserExpression *exp = new ScriptParserWhileExpression(while_condition, while_block, variable_names, auto_add_variables, variable_array);
				expressions.append(exp);
				for (int e = 0 ; e < exp->nbErrors() ; ++e)
					errors.append(exp->getError(e));
				state = 0;
				continue;
			}
			// Otherwise read until we reach a ';' (which should be at the end of a line)
			else if (!line.isEmpty() && line[line.length() - 1] == ';') {
				expression += line.left(-2);
				if (!expression.isEmpty()) {
					ScriptParserExpression *exp = new ScriptParserEquationExpression(expression, variable_names, auto_add_variables, variable_array);
					expressions.append(exp);
					if (exp->nbErrors() > 0) {
						errors << String::format("Script parsing error line %d:invalid expression '%s'.", line_number, expression.c_str());
						for (int e = 0 ; e < exp->nbErrors() ; ++e)
							errors.append(exp->getError(e));
					}
					expression.clear();
				}
			} else {
				expression += line;
			}
		}

	}

	if (state == 1 || state == 2 || state == 3) {
		ScriptParserExpression *exp = 0;
		if (else_block.isEmpty())
			exp = new ScriptParserConditionalExpression(if_condition, if_block, variable_names, auto_add_variables, variable_array);
		else
			exp = new ScriptParserConditionalExpression(if_condition, if_block, else_block, variable_names, auto_add_variables, variable_array);
		expressions.append(exp);
		for (int e = 0 ; e < exp->nbErrors() ; ++e)
			errors.append(exp->getError(e));
	}

	else if (state != 0 || !expression.isEmpty()) {
		if (is_original_script)
			errors << String("Script parsing error: unexpected end of script.");
		else
			// this function was called with a if or else block
			errors << String::format("Script parsing error line %d: missing ';' before '}'.", line_number);
	}
}

/*! \fn bool ScriptParser::readCondition(String &condition, StrReadStream &script, String &line, int &line_number, StringList &errors)
 *
 * Internal function used to read a condition from the stream.
 */
bool ScriptParser::readCondition(
	String &condition, StrReadStream &stream,
	String &line, int &line_number,
	StringList &errors
) {
	int condition_level = 1;
	do {
		while (!line.isEmpty() && condition_level > 0) {
			int close_parenthesis_index = line.findChar(')');
			if (close_parenthesis_index == -1) {
				condition += line;
				condition_level += line.countChar('(');
				line.clear();
			} else {
				String left = line.left(close_parenthesis_index);
				condition += left;
				condition_level += left.countChar('(') - 1;
				line = line.right(close_parenthesis_index + 1).trimmed();
			}
		}
		if (condition_level == 0)
			break;

		// read next line
		line = stream.readLine().trimmed();

		// Check there is something on the line
		if (line.startsWith("!!")) {
			line_number = line.right(2).toInt();
			line.clear();
		}
	} while (!stream.atEnd());

	if (condition_level != 0) {
		errors << String("Script parsing error: unexpected end of script (unbalanced parenthesis).");
		return false;
	}

	if (!line.isEmpty()) {
		errors << String::format("Script parsing error line %d: { expected after conditional expression but '%s' found.", line_number, line.c_str());
		return false;
	}
	// remove last parenthesis
	condition = condition.left(-2).trimmed();
	// check the condition is not empty
	if (condition.isEmpty()) {
		errors << String::format("Script parsing error line %d: empty conditional expression.", line_number);
		return false;
	}
	// read next lines to find start of block
	do {
		line = stream.readLine().trimmed();
		if (line.startsWith("!!")) {
			line_number = line.right(2).toInt();
			line.clear();
		}
	} while (line.isEmpty() && !stream.atEnd());
	if (line != "{") {
		errors << String::format("Script parsing error line %d: '{' expected after conditional expression.", line_number);
		return false;
	}

	return true;
}

/*! \fn bool ScriptParser::readBlock(String &block, StrReadStream &stream, int &line_number, StringList &errors)
 *
 * Internal function used to read a block of expressions from the stream.
 */
bool ScriptParser::readBlock(
	String &block, StrReadStream &stream,
	int &line_number,
	StringList &errors
 ) {
	int block_level = 1;
	while (!stream.atEnd()) {
		String line = stream.readLine().trimmed();

		// Check there is something on the line
		if (line.isEmpty())
			continue;

		if (line.startsWith("!!"))
			line_number = line.right(2).toInt();

		// Check if we are leaving a block
		if (line == "}") {
			--block_level;
			if (block_level == 0)
				return true;
		} else if (line == "{")
			++block_level;

		block += line + "\n";
	}

	errors << String("Script parsing error: unexpected end of script (unbalanced '{' and '}').");
	return false;
}

/*! \fn ScriptParserExpression::ScriptParserExpression()
 *
 * Cnstructor for the ScriptParserExpression class.
 */
ScriptParserExpression::ScriptParserExpression() {
}

ScriptParserExpression::~ScriptParserExpression() {
}

/*! \fn int ScriptParserExpression::nbErrors() const
 *
 * Return the number of errors of this ScriptParserExpression.
 */
int ScriptParserExpression::nbErrors() const {
	return 0;
}

/*! \fn String ScriptParserExpression::getError(int) const
 *
 * Return the error at the given index or an empty
 * String if the index is out of bound.s
 */
String ScriptParserExpression::getError(int) const {
	return String();
}

/***********************************************************************************
 * ScriptParserConditionalExpression
 ***********************************************************************************/

/*! \fn ScriptParserConditionalExpression::ScriptParserConditionalExpression(const String &condition, const String &if_block, const String &else_block, const StringList &variable_names, bool auto_add_variables, double* variable_array = NULL)
 *
 * Create a ScriptParserConditionalExpression from the given condition
 * expression and the expressions blocks in the if and else block.
 */
ScriptParserConditionalExpression::ScriptParserConditionalExpression(
	const String &condition,
	const String &if_block,
	const String &else_block,
	const StringList &variable_names,
	bool auto_add_variables,
	double* variable_array
) :
	ScriptParserExpression(), condition_(NULL)
{
	if (!condition.isEmpty()) {
		// Create condition
		String equation = "if(" + condition + ", 1., 0.)";
		condition_ = new EquationParser();
		condition_->parse(equation, variable_names, auto_add_variables, variable_array);
		for (int e = 0 ; e < condition_->nbErrors() ; ++e)
			errors_.append(condition_->getError(e));
		// Parse if block
		ScriptParser::breakBlock(if_block, if_expressions_, variable_names, auto_add_variables, errors_, variable_array);
		// Parse else block
		ScriptParser::breakBlock(else_block, else_expressions_, variable_names, auto_add_variables, errors_, variable_array);
	}
}

/*! \fn ScriptParserConditionalExpression::ScriptParserConditionalExpression(const String &condition, const String &if_block, const StringList &variable_names, bool auto_add_variables, double* variable_array = NULL)
 *
 * Create a ScriptParserConditionalExpression from the given condition
 * expression and the expressions block in the if block.
 */
ScriptParserConditionalExpression::ScriptParserConditionalExpression(
	const String &condition,
	const String &if_block,
	const StringList &variable_names,
	bool auto_add_variables,
	double* variable_array
) :
	ScriptParserExpression(), condition_(NULL)
{
	if (!condition.isEmpty()) {
		// Create condition
		String equation = "if(" + condition + ", 1., 0.)";
		condition_ = new EquationParser();
		condition_->parse(equation, variable_names, auto_add_variables, variable_array);
		for (int e = 0 ; e < condition_->nbErrors() ; ++e)
			errors_.append(condition_->getError(e));
		// Parse if block
		ScriptParser::breakBlock(if_block, if_expressions_, variable_names, auto_add_variables, errors_, variable_array);
	}
}

ScriptParserConditionalExpression::~ScriptParserConditionalExpression() {
	delete condition_;
	for (List<ScriptParserExpression*>::iterator it = if_expressions_.begin() ; it != if_expressions_.end() ; ++it)
		delete (*it);
	for (List<ScriptParserExpression*>::iterator it = else_expressions_.begin() ; it != else_expressions_.end() ; ++it)
		delete (*it);
}

/*! \fn int ScriptParserConditionalExpression::nbErrors() const
 *
 * Return the number of errors.
 */
int ScriptParserConditionalExpression::nbErrors() const {
	return errors_.size();
}

/*! \fn String ScriptParserConditionalExpression::getError(int index) const
 *
 * Return the error at the given index.
 */
String ScriptParserConditionalExpression::getError(int index) const {
	if (index < 0 || index >= errors_.size())
		return String();
	return errors_[index];
}

/*! \fn void ScriptParserConditionalExpression::evaluate()
 *
 * Evaluate the condition and depending on the rsult evaluate the expressions
 * in the if block or in the else block.
 */
void ScriptParserConditionalExpression::evaluate() {
	if (condition_ == NULL)
		return;
	if (!MathUtils::isEqual(condition_->evaluate(), 0.)) {
		for (List<ScriptParserExpression*>::iterator it = if_expressions_.begin() ; it != if_expressions_.end() ; ++it)
			(*it)->evaluate();
	} else {
		for (List<ScriptParserExpression*>::iterator it = else_expressions_.begin() ; it != else_expressions_.end() ; ++it)
			(*it)->evaluate();
	}
}

/*! \fn StringList ScriptParserConditionalExpression::variablesName() const
 *
 * Returns the list of variables used in this expression.
 */
StringList ScriptParserConditionalExpression::variablesName() const {
	StringList vars;
	if (condition_ != NULL)
		vars = condition_->variablesName();
	for (int i = 0 ; i < if_expressions_.size() ; ++i) {
		StringList list = if_expressions_[i]->variablesName();
		for (int j = 0 ; j < list.size() ; ++j) {
			if (!vars.contains(list[j]))
				vars.append(list[j]);
		}
	}
	for (int i = 0 ; i < else_expressions_.size() ; ++i) {
		StringList list = else_expressions_[i]->variablesName();
		for (int j = 0 ; j < list.size() ; ++j) {
			if (!vars.contains(list[j]))
				vars.append(list[j]);
		}
	}
	return vars;
}

/***********************************************************************************
 * ScriptParserWhileExpression
 ***********************************************************************************/

/*! \fn ScriptParserWhileExpression::ScriptParserWhileExpression(const String &condition, const String &block, const StringList &variable_names, bool auto_add_variables, double* variable_array = NULL)
 *
 * Create a ScriptParserWhileExpression from the given condition
 * expression and the expressions block in the loop block.
 */
ScriptParserWhileExpression::ScriptParserWhileExpression(
	const String &condition,
	const String &block,
	const StringList &variable_names,
	bool auto_add_variables,
	double* variable_array
) :
	ScriptParserExpression(), condition_(NULL)
{
	if (!condition.isEmpty()) {
		// Create condition
		String equation = "if(" + condition + ", 1., 0.)";
		condition_ = new EquationParser();
		condition_->parse(equation, variable_names, auto_add_variables, variable_array);
		for (int e = 0 ; e < condition_->nbErrors() ; ++e)
			errors_.append(condition_->getError(e));
		// Parse if block
		ScriptParser::breakBlock(block, expressions_, variable_names, auto_add_variables, errors_, variable_array);
	}
}

ScriptParserWhileExpression::~ScriptParserWhileExpression() {
	delete condition_;
	for (List<ScriptParserExpression*>::iterator it = expressions_.begin() ; it != expressions_.end() ; ++it)
		delete (*it);
}

/*! \fn int ScriptParserWhileExpression::nbErrors() const
 *
 * Return the number of errors.
 */
int ScriptParserWhileExpression::nbErrors() const {
	return errors_.size();
}

/*! \fn String ScriptParserWhileExpression::getError(int) const
 *
 * Return the error at the given index.
 */
String ScriptParserWhileExpression::getError(int index) const {
	if (index < 0 || index >= errors_.size())
		return String();
	return errors_[index];
}

/*! \fn void ScriptParserWhileExpression::evaluate()
 *
 * Evaluate the condition and while it is true execute the expressions
 * in the while loop.
 */
void ScriptParserWhileExpression::evaluate() {
	if (condition_ == NULL)
		return;
	while (!MathUtils::isEqual(condition_->evaluate(), 0.)) {
		for (List<ScriptParserExpression*>::iterator it = expressions_.begin() ; it != expressions_.end() ; ++it)
			(*it)->evaluate();
	}
}

/*! \fn StringList ScriptParserWhileExpression::variablesName() const
 *
 * Returns the list of variables used in this expression.
 */
StringList ScriptParserWhileExpression::variablesName() const {
	StringList vars;
	if (condition_ != NULL)
		vars = condition_->variablesName();
	for (int i = 0 ; i < expressions_.size() ; ++i) {
		StringList list = expressions_[i]->variablesName();
		for (int j = 0 ; j < list.size() ; ++j) {
			if (!vars.contains(list[j]))
				vars.append(list[j]);
		}
	}
	return vars;
}

/***********************************************************************************
 * ScriptParserEquationExpression
 ***********************************************************************************/

/*! \fn ScriptParserEquationExpression::ScriptParserEquationExpression(const String &equation, const StringList &variable_names, bool auto_add_variables, double* variable_array = NULL)
 *
 * Build a ScriptParserEquationExpression for the given equation
 * using the given parameters.
 */
ScriptParserEquationExpression::ScriptParserEquationExpression(
	const String &equation,
	const StringList &variable_names,
	bool auto_add_variables,
	double* variable_array
) :
	ScriptParserExpression(), equation_(NULL)
{
	if (!equation.isEmpty()) {
		equation_ = new EquationParser();
		equation_->parse(equation, variable_names, auto_add_variables, variable_array);
	}
}

ScriptParserEquationExpression::~ScriptParserEquationExpression() {
	delete equation_;
}

/*! \fn int ScriptParserEquationExpression::nbErrors() const
 *
 * Return the number of errors when parsing this equation.
 */
int ScriptParserEquationExpression::nbErrors() const {
	if (equation_ == NULL)
		return 0;
	return equation_->nbErrors();
}

/*! \fn String ScriptParserEquationExpression::getError(int) const
 *
 * get the equation parsing errors.
 */
String ScriptParserEquationExpression::getError(int index) const {
	if (equation_ == NULL)
		return String();
	return equation_->getError(index);
}

/*! \fn void ScriptParserEquationExpression::evaluate()
 *
 * Evaluate the result of this equation.
 */
void ScriptParserEquationExpression::evaluate() {
	if (equation_ != NULL)
		equation_->evaluate();
}

/*! \fn StringList ScriptParserEquationExpression::variablesName() const
 *
 * Returns the list of variables used in this expression.
 */
StringList ScriptParserEquationExpression::variablesName() const {
	StringList vars;
	if (equation_ != NULL)
		vars = equation_->variablesName();
	return vars;
}

/***********************************************************************************
 * Debug code
 ***********************************************************************************/

#ifdef PARSER_TREE_DEBUG

EquationParser::ParserTreeNode ScriptParser::getParserTreeDescription() const {
	EquationParser::ParserTreeNode root;
	root.description_ = "Script";
	for (int i = 0 ; i < expressions_.size() ; ++i)
		root.children_ << expressions_[i]->getParserTreeDescription();
	return root;
}

EquationParser::ParserTreeNode ScriptParserConditionalExpression::getParserTreeDescription() const {
	EquationParser::ParserTreeNode if_node;
	if_node.description_ = "If";

	EquationParser::ParserTreeNode cond_node;
	cond_node.description_ = "Condition";
	if (condition_ != NULL) {
		EquationParser::ParserTreeNode if_cond_node = condition_->getParserTreeDescription();
		// What we get above has three children for Condition/Then/Else.
		// We only keep the Condition children part (the Then Else are constant 0 or 1).
		if (!if_cond_node.children_.isEmpty())
			cond_node.children_ << if_cond_node.children_.first().children_.first();
	}
	if_node.children_ << cond_node;

	EquationParser::ParserTreeNode then_node;
	then_node.description_ = "Then";
	for (int i = 0 ; i < if_expressions_.size() ; ++i)
		then_node.children_ << if_expressions_[i]->getParserTreeDescription();
	if_node.children_ << then_node;

	if (!else_expressions_.isEmpty()) {
		EquationParser::ParserTreeNode else_node;
		else_node.description_ = "Else";
		for (int i = 0 ; i < else_expressions_.size() ; ++i)
			else_node.children_ << else_expressions_[i]->getParserTreeDescription();
		if_node.children_ << else_node;
	}

	return if_node;
}

EquationParser::ParserTreeNode ScriptParserWhileExpression::getParserTreeDescription() const {
	EquationParser::ParserTreeNode while_node;
	while_node.description_ = "While loop";

	EquationParser::ParserTreeNode cond_node;
	cond_node.description_ = "Condition";
	if (condition_ != NULL)
		cond_node.children_ << condition_->getParserTreeDescription();
	while_node.children_ << cond_node;

	EquationParser::ParserTreeNode then_node;
	then_node.description_ = "Then";
	for (int i = 0 ; i < expressions_.size() ; ++i)
		then_node.children_ << expressions_[i]->getParserTreeDescription();
	while_node.children_ << then_node;

	return while_node;
}

EquationParser::ParserTreeNode ScriptParserEquationExpression::getParserTreeDescription() const {
	if (equation_ == NULL) {
		EquationParser::ParserTreeNode node;
		node.description_ = "(Empty)";
		return node;
	}
	return equation_->getParserTreeDescription();
}

#endif
