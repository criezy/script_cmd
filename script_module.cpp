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
#include "modules.h"
#include <math.h>
#include <ctype.h>
#include <stdio.h>

void printScriptModuleHelp(int mode) {
	switch (mode) {
	case 1:
	case 2:
	case 3:
		printEquationModuleHelp(mode);
		break;
	case 4:
		printf("A script consists of one or more mathematical expressions separated\n");
		printf("by a ';'. It can also contain conditional statements to control the\n");
		printf("flow of the execution. You can also use variables.\n");
		printf("Example:\n");
		printf("  variable1 = variable2 * 1.56325 + 17.4;\n");
		printf("  if (variable1 >= 0) {\n");
		printf("    variable1 = sqrt(variable1);\n");
		printf("  }\n");
		printf("  // This is a valid comment.\n");
		printf("  # This is another valid comment.\n");
		printf("  variable4 = 0;\n");
		printf("  if (variable1 > 1) {\n");
		printf("    variable3 = 1 / sqrt(variable1 - 1);\n");
		printf("    variable4 = variable3 + 1;\n");
		printf("  } else if (variable1 > 0) {\n");
		printf("    variable3 = 1 / sqrt(variable1);\n");
		printf("    variable4 = variable3 - 1;\n");
		printf("  } else {\n");
		printf("    if (variable2 != 0) {\n");
		printf("      variable3 = 1 / variable2;\n");
		printf("    }\n");
		printf("  }\n");
		printf("  while (variable4 != 0.0) {\n");
		printf("    variable3 = sqrt(variable3);\n");
		printf("    variable4 = variable3 - 1;\n");
		printf("      if (variable4 < 0) {\n");
		printf("    }\n");
		printf("  }\n");
		break;
	default:
		printf("Evaluates C-like script.\n");
		printf("The following commands are recognized:\n");
		printf("  - 'start'     Start defining the script. Type 'end' to finish the script\n");
		printf("                definition. The script will consists of everything you have\n");
		printf("                typed between 'start' and 'end'.\n");
		printf("  - 'script'    Print the previously defined script.\n");
		printf("  - 'variables' Print the list of variables in the previously defined script.\n");
		printf("  - 'run'       Run the previously defined script.\n");
		printf("  - 'help [topic]'   Print this help or help on a specific topic. Topics are:\n");
		printf("                     'constants', 'functions', 'operators' and 'script'.\n");
		printf("  - 'quit' or 'exit' Quit the program.\n");
		break;
	}
}

bool parseScript(
	const String& script,
	ScriptParser& parser,
	StringList& variables,
	double*& var_values
) {
	delete [] var_values;
	var_values = NULL;

	if (script.isEmpty())
		return false;

	variables = parser.getVariablesList(script);
	if (parser.nbErrors() > 0) {
		variables.clear();
		printf("The script contains %d error(s):\n", parser.nbErrors());
		for (int error = 0 ; error < parser.nbErrors() ; ++error)
			printf("  %d: %s\n", error+1, parser.getError(error).c_str());
		return false;
	}

	if (!parser.parse(script, variables)) {
		variables.clear();
		return false;
	}
	if (!variables.isEmpty()) {
		var_values = new double[variables.size()];
		for (int i = 0 ; i < variables.size() ; ++i)
			var_values[i] = 0.0;
	}
	return true;
}

void runScriptModule(const String& s) {
	String script = s;
	ScriptParser parser, parser2;
	StringList variables;
	double* var_values = NULL;
	bool script_edition = false;
	bool script_defined = parseScript(script, parser, variables, var_values);
	printf("Starting script mode.\nType 'help' to get some help.\n");
	while (1) {
		// Script edition
		if (script_edition) {
			String line = readLine(false);
			if (line == "end\n") {
				script_edition = false;
				script_defined = parseScript(script, parser, variables, var_values);
			} else
				script += line;
			continue;
		}

		// Read a line
		printf("> ");
		String line = readLine();

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
			else if (topic == "script")
				t = 4;
			printScriptModuleHelp(t);
			continue;
		}
		
		// start
		if (line == "start") {
			script_edition = true;
			script_defined = false;
			script.clear();
			continue;
		}
		
		// script
		if (line == "script") {
			if (!script_defined) {
				printf("The script is not defined. Type 'start' to define the\n");
				printf("script, and 'end' when you have finished.\n");
			} else
				printf("%s", script.c_str());
			continue;
		}
		
		// variables
		if (line == "variables") {
			if (!script_defined) {
				printf("The script is not defined. Type 'start' to define the\n");
				printf("script, and 'end' when you have finished.\n");
			} else {
				for (int i = 0 ; i < variables.size() ; ++i)
					printf("%s = %.12g\n", variables[i].c_str(), var_values[i]);
			}
			continue;
		}
		
		// run
		if (line == "run") {
			if (!script_defined) {
				printf("The script is not defined. Type 'start' to define the\n");
				printf("script, and 'end' when you have finished.\n");
			} else
				parser.evaluate(var_values);
			continue;
		}
		
		// Treat it as a one-line script
		if (parser2.parse(line, variables))
			parser2.evaluate(var_values);
		else {
			if (parser2.nbErrors() == 0)
				printf("Syntax error...\n");
			else {
				printf("Equation contains %d error(s):\n", parser2.nbErrors());
				for (int error = 0 ; error < parser2.nbErrors() ; ++error)
					printf("  %d: %s\n", error+1, parser2.getError(error).c_str());
			}
		}
	}
	
	delete [] var_values;
}