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

#include "strstream.h"

StrReadStream::StrReadStream(const String& str) :
	string_(str), pos_(0)
{
}

StrReadStream::~StrReadStream() {
}

/*! \fn String StrReadStream::readLine()
 *
 * Read the strin from the current position to either the end of the string or
 * the first end of line character found, whichever comes first. It returns the
 * character read, striped of the end of line character.
 *
 * If atEnd() is true, it returns an empty string.
 */
String StrReadStream::readLine() {
	int size = string_.length();
	if (pos_ >= size)
		return String();

	int end = pos_;
	while (end < size && string_[end] != '\n' && string_[end] != '\r')
		++end;

	String str = string_.mid(pos_, end - 1);

	// skip the end of line and set pos
	if (end < size) {
		++end;
		// handle DOS end of line (CRLF)
		if (end < size && string_[end - 1] == '\r' && string_[end] == '\n')
			++end;
	}
	pos_ = end;

	return str;
}


