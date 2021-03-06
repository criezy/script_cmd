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

#ifndef modules_h
#define modules_h

#include "str.h"
#include <stdio.h>

void runEquationModule();
void runScriptModule(const String& script = String());

void printEquationModuleHelp(int mode);
void printScriptModuleHelp(int mode);
String readLine(const char* prompt, bool strip_eol = true);
String readLine(bool strip_eol, FILE* stream);

#endif // modules_h
