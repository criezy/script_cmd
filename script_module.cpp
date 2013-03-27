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
#include "redirect_output.h"
#include "modules.h"
#include "map.h"
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
		printf("  - 'start [name]'   Start defining a script. Optionaly a name can be given for the script.\n");
		printf("                     Type 'end' to finish the script definition. The script will consists of\n");
		printf("                     everything you have typed between 'start' and 'end'.\n");
		printf("  - 'scripts'        List all the defined scripts.\n");
		printf("  - 'clear [name]'   Clear the script with the given name.\n");
		printf("  - 'script [name]'  Print the previously defined script with the given name.\n");
		printf("  - 'script [name] < file' Initialise the script with the given name using the content of the\n");
		printf("                           given file.\n");
		printf("  - 'script [name] > file' Save the script with the given name to the given file\n");
		printf("  - 'variables'      Print the list of variables in the previously defined scripts.\n");
		printf("  - 'run [name]'     Run the previously defined script with the given name.\n");
		printf("  - 'run [name] > file'    Run the previously defined script with the given name and redirect.\n");
		printf("                           output to file.\n");
		printf("  - 'help [topic]'   Print this help or help on a specific topic. Topics are:\n");
		printf("                     'constants', 'functions', 'operators' and 'script'.\n");
		printf("  - 'quit' or 'exit' Quit the program.\n");
		break;
	}
}

void removeScript(
	const String& name,
	Map<String, String>& scripts,
	StringList& variables,
	double*& var_values
) {
	// Remove this script
	if (!scripts.remove(name))
		return;

	// Update list of variables
	const StringList& names = scripts.keys();
	StringList new_vars;
	ScriptParser parser;
	for (int i = 0 ; i < names.size() ; ++i) {
		StringList script_vars = parser.getVariablesList(scripts[names[i]]);
		for (int v = 0 ; v < script_vars.size() ; ++v) {
			if (!new_vars.contains(script_vars[v]))
				new_vars << script_vars[v];
		}
	}
	if (new_vars.size() == variables.size())
		return; // No variables was removed

	// Update value array
	double* new_values = NULL;
	if (!new_vars.isEmpty()) {
		new_values = new double[new_vars.size()];
		for (int i = 0 ; i < new_vars.size() ; ++i) {
			int index = variables.indexOf(new_vars[i]);
			if (index == -1)
				new_values[i] = 0.0;
			else
				new_values[i] = var_values[index];
		}
	}

	delete [] var_values;
	var_values = new_values;
	variables = new_vars;
}

void addScript(
	const String& script,
	const String& name,
	Map<String, String>& scripts,
	StringList& variables,
	double*& var_values
) {
	if (script.isEmpty()) {
		removeScript(name, scripts, variables, var_values);
		return;
	}

	// Check the script is valid
	ScriptParser parser;
	StringList new_vars = parser.getVariablesList(script);
	if (parser.nbErrors() > 0) {
		printf("The script contains %d error(s):\n", parser.nbErrors());
		for (int error = 0 ; error < parser.nbErrors() ; ++error)
			printf("  %d: %s\n", error+1, parser.getError(error).c_str());
		removeScript(name, scripts, variables, var_values);
		return;
	}

	// Update list of variables
	scripts.remove(name);
	const StringList& names = scripts.keys();
	for (int i = 0 ; i < names.size() ; ++i) {
		StringList script_vars = parser.getVariablesList(scripts[names[i]]);
		for (int v = 0 ; v < script_vars.size() ; ++v) {
			if (!new_vars.contains(script_vars[v]))
				new_vars << script_vars[v];
		}
	}
	scripts[name] = script;

	// Update value array
	double* new_values = NULL;
	if (!new_vars.isEmpty()) {
		new_values = new double[new_vars.size()];
		for (int i = 0 ; i < new_vars.size() ; ++i) {
			int index = variables.indexOf(new_vars[i]);
			if (index == -1)
				new_values[i] = 0.0;
			else
				new_values[i] = var_values[index];
		}
	}

	delete [] var_values;
	var_values = new_values;
	variables = new_vars;
}

String breakLine(const String& line, String& argument, String& input_file, String& output_file) {
	String cmd = line.trimmed();
	argument.clear();
	input_file.clear();
	output_file.clear();
	if (cmd.isEmpty())
		return cmd;

	// Check if we have an input file
	int index = cmd.findChar('<');
	if (index != -1) {
		input_file = cmd.right(index + 1).trimmed();
		cmd = cmd.left(index - 1).trimmed();
	}
	// Check if we have an output file
	index = cmd.findChar('>');
	if (index != -1) {
		output_file = cmd.right(index + 1).trimmed();
		cmd = cmd.left(index - 1).trimmed();
	} else {
		index = input_file.findChar('>');
		if (index != -1) {
			output_file = input_file.right(index + 1).trimmed();
			input_file = input_file.left(index - 1).trimmed();
		}
	}
	// break what remains of the command between a command and an argument if there is a space
	index = 1;
	while (index < cmd.length() && !isspace(cmd[index]))
		++ index;
	if (index < cmd.length()) {
		argument = cmd.right(index+1).trimmed();
		cmd = cmd.left(index-1);
	}
	return cmd;
}

void runScriptModule(const String& s) {
	Map<String, String> scripts;
	ScriptParser parser;
	StringList variables;
	String cur_script, cur_name, input_file, output_file;
	double* var_values = NULL;
	if (!s.isEmpty())
		addScript(s, String(), scripts, variables, var_values);
	bool script_edition = false;
	printf("Starting script mode.\nType 'help' to get some help.\n");
	while (1) {
		// Script edition
		if (script_edition) {
			String line = readLine(false);
			if (line == "end\n") {
				script_edition = false;
				addScript(cur_script, cur_name, scripts, variables, var_values);
			} else
				cur_script += line;
			continue;
		}

		// Read a line
		printf("> ");
		String line = readLine();
		String cmd = breakLine(line, cur_name, input_file, output_file);

		if (cmd.isEmpty())
			continue;

		// Check if it is exit
		if (cmd == "exit" || cmd == "quit")
			break;

		// Check if it is help
		if (cmd == "help") {
			int t = 0;
			if (cur_name == "constants")
				t = 1;
			else if (cur_name == "functions")
				t = 2;
			else if (cur_name == "operators")
				t = 3;
			else if (cur_name == "script")
				t = 4;
			printScriptModuleHelp(t);
			continue;
		}

		// start
		if (cmd == "start") {
			script_edition = true;
			cur_script.clear();
			continue;
		}

		// scripts
		if (cmd == "scripts") {
			if (scripts.isEmpty()) {
				printf("No script is currently defined.\n");
				printf("Type 'start [name]' to define a script and 'end' when you have finished.\n");
			} else {
				const StringList& names = scripts.keys();
				printf("There are %d scripts defined:\n", names.size());
				for (int i = 0 ; i < names.size() ; ++i)
					printf("   %s\n", names[i].c_str());
			}
			continue;
		}

		// clear
		if (cmd == "clear") {
			removeScript(cur_name, scripts, variables, var_values);
			continue;
		}

		// script
		if (cmd == "script") {
			if (!input_file.isEmpty()) {
				FILE* file = fopen(input_file.c_str(), "r");
				if (file == NULL)
					printf("Cannot open file '%s'\n", input_file.c_str());
				else {
					char buffer[256];
					cur_script.clear();
					while (fgets(buffer, 256, file) != NULL)
						cur_script += buffer;
					fclose(file);
					addScript(cur_script, cur_name, scripts, variables, var_values);
				}
			}
			if (!scripts.contains(cur_name)) {
				printf("The script '%s' is not defined.\n", cur_name.c_str());
				printf("Type 'scripts' to get a list of defined scripts.\n");
			} else {
				if (!output_file.isEmpty()) {
					FILE* file = fopen(output_file.c_str(), "w");
					if (file == NULL)
						printf("Failed to save script to file '%s'.\n", output_file.c_str());
					else {
						fprintf(file, "%s", scripts[cur_name].c_str());
						fclose(file);
					}
				} else
					printf("%s", scripts[cur_name].c_str());
			}
			continue;
		}

		// variables
		if (cmd == "variables") {
			if (scripts.isEmpty()) {
				printf("No script is currently defined.\n");
				printf("Type 'start [name]' to define a script and 'end' when you have finished.\n");
			} else {
				for (int i = 0 ; i < variables.size() ; ++i)
					printf("%s = %.12g\n", variables[i].c_str(), var_values[i]);
			}
			continue;
		}

		// run
		if (cur_name.isEmpty() && scripts.contains(cmd)) {
			// Only the name of a script was typed. Run it.
			cur_name = cmd;
			cmd = "run";
		}
		if (cmd == "run") {
			if (!scripts.contains(cur_name)) {
				printf("The script '%s' is not defined.\n", cur_name.c_str());
				printf("Type 'scripts' to get a list of defined scripts.\n");
			} else {
				bool redirected = false;
				if (!output_file.isEmpty())
					redirected = redirect_output(output_file);
				parser.parse(scripts[cur_name], variables);
				parser.evaluate(var_values);
				if (redirected)
					close_redirect_output();
			}
			continue;
		}

		// Treat it as a one-line script
		if (parser.parse(line, variables))
			parser.evaluate(var_values);
		else {
			if (parser.nbErrors() == 0)
				printf("Syntax error...\n");
			else {
				printf("Equation contains %d error(s):\n", parser.nbErrors());
				for (int error = 0 ; error < parser.nbErrors() ; ++error)
					printf("  %d: %s\n", error+1, parser.getError(error).c_str());
			}
		}
	}

	delete [] var_values;
}
