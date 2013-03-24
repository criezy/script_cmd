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

#ifndef strstream_h
#define strstream_h

#include "str.h"

class StrReadStream {
public:
	StrReadStream(const String&);
	~StrReadStream();
	
	String readLine();
	char readChar();
	bool atEnd() const;
	
private:
	String string_;
	int pos_;
};

/*! \fn char StrReadStream::readChar()
 *
 * Return the character at the current position and move to the next char.
 * If atEnd() is true, it returns 0.
 */
inline char StrReadStream::readChar() {
	if (atEnd())
		return 0;
	return string_[pos_++];
}

/*! \fn bool StrReadStream::atEnd() const
 *
 * Return true if we have reached the end of the string.
 */
inline bool StrReadStream::atEnd() const {
	return (pos_ >= string_.length());
}

#endif // strstream_h
