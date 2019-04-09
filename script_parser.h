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

#ifndef script_parser_h
#define script_parser_h

#include <stdlib.h>
#include "list.h"
#include "str.h"
#include "strlist.h"
#include "strstream.h"

// The next include is only needed for debugging. Otherwise we could use a forward declaration
#include "equation_parser.h"

class ScriptParserExpression;

/*! \class ScriptParser
 *
 * Parse a script containing equations. The parser breaks the script into
 * expression that are then parsed by the EquationParser. Therefore the
 * individual expressions have to follow the syntax of the EquationParser.
 * However 'if(condition, expression1, expression2)' constructs accepted by the
 * EquationParser are not recognized and should be replaced by 'if (condition)
 * { expression1 } else { expression2 } constructs.
 *
 * A script is a sequence of expression separated by a ';'. It can also contains
 * conditional blocks and comments.
 * \code
    variable1 = variable2 * 1.56325 + 17.4;
    if (variable1 >= 0) {
        variable1 = sqrt(variable1);
    }
    // This is a valid comment.
    # This is another valid comment.
    variable4 = 0;
    if (variable1 > 1) {
        variable3 = 1 / sqrt(variable1 - 1);
        variable4 = variable3 + 1;
    } else if (variable1 > 0) {
        variable3 = 1 / sqrt(variable1);
        variable4 = variable3 - 1;
    } else {
        if (variable2 != 0) {
            variable3 = 1 / variable2;
        }
    }
    while (variable4 != 0.) {
        variable3 = sqrt(variable3);
        variable4 = variable3 - 1;
        if (variable4 < 0) {
            variable4 = 0.;
        }
    }
 * \endcode
 *
 * Example:
 *
 * Consider the following script.
 * \code
    if (z == 0) {
        x = y;
    } else {
        x = y / (2. * z);
    }
    z = z + 1;
 * \endcode
 * And then consider the following code.
 * \code
	StringList var_names;
	var_names << "x" << "y" << "z";
	ScriptParser parser;
	if (parser.parse(script, var_names)) {
		parser.VariablesValue()[1] = 48.;
		parser.VariablesValue()[2] = 0.;
		for (int i = 1 ; i <= 5 ; ++i) {
			float cur_y = parser.VariablesValue()[1];
			float cur_z = parser.VariablesValue()[2];
			parser->evaluate();
			std::cout
				<< "Run " << i << ": x (y = " << cur_y << ", z = "<< cur_z << ") = "
				<< parser.VariablesValue()[0] << std::endl;
		}
	}
 * \endcode
 *
 * The output will be:
 <PRE>
    Run 1: x (y = 48, z = 0) = 48
    Run 2: x (y = 48, z = 1) = 24
    Run 3: x (y = 48, z = 2) = 12
    Run 4: x (y = 48, z = 3) = 8
    Run 5: x (y = 48, z = 4) = 6
 </PRE>
 */

#define ScriptParserMaxNbErrors 50

class ScriptParser {
public:
	ScriptParser();
	~ScriptParser();

	bool parse(const String &script, const StringList &variable_names);

	StringList getVariablesList(const String &script);

	void evaluate(double *var = 0);

	double *VariablesValue();
	const StringList &variablesName() const;

	int nbErrors() const;
	String getError(int) const;
	String getLastError() const;

	static void breakBlock(
		const String &script_block, List<ScriptParserExpression*> &expressions,
		const StringList &variable_names, bool auto_add_variables,
		StringList &errors
	);
	
#ifdef PARSER_TREE_DEBUG
	EquationParser::ParserTreeNode getParserTreeDescription() const;
#endif

protected:
	void clear();
	static bool readCondition(
		String &condition, StrReadStream &stream,
		String &line, int &line_number,
		StringList &errors
	);
	static bool readBlock(
		String &block, StrReadStream &stream,
		int &line_number,
		StringList &errors
	);

private:
	List<ScriptParserExpression*> expressions_;
	// Equation evaluation
	double *args_double_;
	StringList args_names_;
	int nb_args_;
	// Errors
	StringList errors_;
};

/*! \class ScriptParserExpression
 *
 * Internal class used by ScriptParser to store an expression block.
 */
class ScriptParserExpression {
public:
	ScriptParserExpression();
	virtual ~ScriptParserExpression();

	virtual int nbErrors() const;
	virtual String getError(int) const;

	virtual void evaluate(double *var) = 0;

	virtual StringList variablesName() const = 0;

#ifdef PARSER_TREE_DEBUG
	virtual EquationParser::ParserTreeNode getParserTreeDescription() const = 0;
#endif
};

/*! \class ScriptParserConditionalExpression
 *
 * Internal class used by ScriptParser to store an expression block.
 */
class ScriptParserConditionalExpression : public ScriptParserExpression {
public:
	ScriptParserConditionalExpression(
			const String &condition, const String &if_block, const String &else_block,
			const StringList &variable_names, bool auto_add_variables = false
		);
	ScriptParserConditionalExpression(
			const String &condition, const String &if_block,
			const StringList &variable_names, bool auto_add_variables = false
		);
	virtual ~ScriptParserConditionalExpression();

	virtual int nbErrors() const;
	virtual String getError(int) const;

	virtual void evaluate(double *var);

	virtual StringList variablesName() const;

#ifdef PARSER_TREE_DEBUG
	virtual EquationParser::ParserTreeNode getParserTreeDescription() const;
#endif

private:
	EquationParser *condition_;
	List<ScriptParserExpression*> if_expressions_;
	List<ScriptParserExpression*> else_expressions_;
	// Errors
	StringList errors_;
};

/*! \class ScriptParserWhileExpression
 *
 * Internal class used by ScriptParser to store an expression block.
 */
class ScriptParserWhileExpression : public ScriptParserExpression {
public:
	ScriptParserWhileExpression(
		const String &condition,
		const String &block,
		const StringList &variable_names,
		bool auto_add_variables = false
	);
	virtual ~ScriptParserWhileExpression();

	virtual int nbErrors() const;
	virtual String getError(int) const;

	virtual void evaluate(double *var);

	virtual StringList variablesName() const;

#ifdef PARSER_TREE_DEBUG
	virtual EquationParser::ParserTreeNode getParserTreeDescription() const;
#endif

private:
	EquationParser *condition_;
	List<ScriptParserExpression*> expressions_;
	// Errors
	StringList errors_;
};

/*! \class ScriptParserEquationExpression
 *
 * Internal class used by ScriptParser to store an expression block.
 */
class ScriptParserEquationExpression : public ScriptParserExpression {
public:
	ScriptParserEquationExpression(
		const String &equation,
		const StringList &variable_names,
		bool auto_add_variables = false
	);
	virtual ~ScriptParserEquationExpression();

	virtual int nbErrors() const;
	virtual String getError(int) const;

	virtual void evaluate(double *var);

	virtual StringList variablesName() const;

#ifdef PARSER_TREE_DEBUG
	virtual EquationParser::ParserTreeNode getParserTreeDescription() const;
#endif

private:
	EquationParser *equation_;
};

#endif
