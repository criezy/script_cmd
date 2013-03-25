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

#include "modules.h"
#include "str.h"
#include <stdio.h>

void printHelp(const char* cmd_name) {
	printf("Usage: %s [options]\n", cmd_name);
	printf("\n");
	printf("When no option is given it starts the program in interactive script mode.\n");
	printf("\n");
	printf("Options can be:\n");
	printf("  --help              Print this help.\n");
	printf("  --simple-mode       Interactive simple equation mode.\n");
	printf("  -e                  Same as --simple-mode\n");
	printf("  --script='command'  Start in script mode and set the script to the given one.\n");
	printf("  -s 'command'        Same as --script='command'\n");
	printf("  --file=path         Start in script mode and load the script from the given file.\n");
	printf("  -f path             Same as --file=path\n");
	printf("\n");
	printf("This program interprets C-like mathematical expressions and prints the result.\n");
	printf("In Script mode, you can specify a multi-line script that contains variables,\n");
	printf("then set the variable values and run the script multiple time (changing the\n");
	printf("variable values between each run if you want to.\n");
	printf("In simple mode, each line you type is interpreted as a simple equation and the\n");
	printf("result is printed when you press return.\n");
	printf("See the README that comes with the software for syntax examples\n");
	printf("\n");
}

int main(int argc, char* argv[])  {
	// When invoqued with no arguments, start interactive script mode
	if (argc == 1) {
		runScriptModule();
		return 0;
	}

	// Check we don't have too many arguments
	if (argc > 3) {
		printf("Unrecognized option.\n");
		printHelp(argv[0]);
		return 1;
	}

	// Parse arguments
	String first_arg(argv[1]);
	String second_arg;
	if (argc == 2) {
		if (first_arg.startsWith("--script=")) {
			second_arg = first_arg.right(9);
			first_arg = "-s";
		} else if (first_arg.startsWith("--file=")) {
			second_arg = first_arg.right(7);
			first_arg = "-f";
		}
		// Strip quotes if needed
		if (
			second_arg.length() > 2 && (
				(second_arg[0] == '\'' && second_arg[second_arg.length()-1] == '\'') ||
				(second_arg[0] == '"' && second_arg[second_arg.length()-1] == '"')
			)
		)
			second_arg = second_arg.mid(1, second_arg.length() - 2);
	} else
		second_arg = argv[2];

	// Help
	if (first_arg == "--help" && second_arg.isEmpty()) {
		printHelp(argv[0]);
		return 0;
	}

	// Simple mode
	if ((first_arg == "-e" || first_arg == "--simple-mode") && second_arg.isEmpty()) {
		runEquationModule();
		return 0;
	}

	// File mode
	if (first_arg == "-f" && !second_arg.isEmpty()) {
		// Load the content of the file into the second argument.
		// And then do as if it was invoqued with '-s'.
		FILE* f = fopen(second_arg.c_str(), "r");
		if (f == NULL) {
			printf("Cannot open file '%s'\n", second_arg.c_str());
			return -1;
		}
		char buffer[256];
		second_arg.clear();
		while (fgets(buffer, 256, f) != NULL)
			second_arg += buffer;
		fclose(f);
		first_arg = "-s";
	}

	// Script mode
	if (first_arg == "-s" && !second_arg.isEmpty()) {
		runScriptModule(second_arg);
		return 0;
	}

	printf("Unrecognized option.\n");
	printHelp(argv[0]);
	return 1;
}
