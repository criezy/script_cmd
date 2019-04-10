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

#include "str.h"
#include <ctype.h>
#include <stdio.h>

const char* const String::nullStr = "";

/*! \fn String::String()
 *
 * Construct a new empty string.
 */
String::String() : data_(NULL) {
}

/*! \fn String::String(const char *str)
 *
 * Construct a new string from the given NULL-terminated C string.
 */
String::String(const char *str) : data_(NULL) {
	if (str != NULL)
		initWithCStr(str, strlen(str));
}

/*! \fn String::String(const char *str, int len)
 *
 * Construct a new string containing exactly len characters read from address str.
 */
String::String(const char *str, int len) : data_(NULL) {
	if (str != NULL && len > 0)
		initWithCStr(str, len);
}

/*! \fn String::String(const String &str)
 *
 * Construct a copy of the given string.
 */
String::String(const String &str) : data_(str.data_) {
	if (data_ != NULL)
		++data_->ref_cpt_;
}

/*! \fn String::~String()
 *
 * Destroy this string.
 */
String::~String() {
	clear();
}

/*! \fn void String::initWithCStr(const char *str, int len)
 *
 * Internal function. Initialize this String from the first \p len
 * characters of the given \p str.
 */
void String::initWithCStr(const char *str, int len) {
	data_ = new StringSharedData();
	++data_->ref_cpt_;

	// By default, for the capacity we use the next multiple of 32
	data_->capacity_ = ((len + 32) & ~0x1F);
	data_->size_ = len;
	data_->str_ = new char[data_->capacity_];

	// Copy the string into the storage area
	memcpy(data_->str_, str, len);
	data_->str_[len] = 0;
}

void String::ensureUnique() {
	if (data_ != NULL && data_->ref_cpt_ > 1) {
		--data_->ref_cpt_;
		data_ = new StringSharedData(*data_);
		++data_->ref_cpt_;
	}
}

/*! \fn void String::ensureCapacity(int new_size, bool keep_old)
 *
 * Ensure that enough storage is available to store at least new_size
 * characters plus a null byte. In addition, if we currently share
 * the storage with another string, unshare it, so that we can safely
 * write to the storage.
 */
void String::ensureCapacity(int new_size, bool keep_old) {
	ensureUnique();

	int cur_capacity = data_ == NULL ? 0 : data_->capacity_;
	if (new_size < cur_capacity)
		return;

	// By default, for the capacity we use the next multiple of 32
	int new_capacity = ((new_size + 32) & ~0x1F);
	if (new_capacity < cur_capacity * 2)
		new_capacity = cur_capacity * 2;

	char* new_storage = new char[new_capacity];
	int size = 0;
	if (keep_old && data_ != NULL && data_->size_ > 0) {
		if (data_ != NULL && data_->size_ < new_size)
			size = data_->size_;
		else
			size = new_size;
		memcpy(new_storage, data_->str_, size);
	}
	new_storage[size] = 0;

	if (data_ == NULL) {
		data_ = new StringSharedData();
		++data_->ref_cpt_;
	} else
		delete [] data_->str_;
	data_->size_ = size;
	data_->capacity_ = new_capacity;
	data_->str_ = new_storage;
}

String &String::operator=(const char *str) {
	int len = strlen(str);
	ensureCapacity(len, false);
	if (data_ != NULL && data_->str_ != NULL) {
		data_->size_ = len;
		memmove(data_->str_, str, len + 1);
	}
	return *this;
}

String &String::operator=(const String &str) {
	if (&str == this || str.data_ == data_)
		return *this;

	if (data_ != NULL && --data_->ref_cpt_ == 0)
		delete data_;
	data_ = str.data_;
	if (data_ != NULL)
		++data_->ref_cpt_;

	return *this;
}

String &String::operator+=(const char *str) {
	if (data_ != NULL && data_->str_ <= str && str <= data_->str_ + data_->size_)
		return operator+=(String(str));

	int len = strlen(str);
	if (len > 0) {
		ensureCapacity(length() + len, true);
		memcpy(data_->str_ + data_->size_, str, len + 1);
		data_->size_ += len;
	}
	return *this;
}

String &String::operator+=(const String &str) {
	if (&str == this)
		return operator+=(String(str));

	int len = str.length();
	if (len > 0) {
		ensureCapacity(length() + len, true);
		memcpy(data_->str_ + data_->size_, str.data_->str_, len + 1);
		data_->size_ += len;
	}
	return *this;
}

String &String::operator+=(char c) {
	ensureCapacity(length() + 1, true);

	data_->str_[data_->size_++] = c;
	data_->str_[data_->size_] = 0;

	return *this;
}

bool String::contains(const String &s) const {
	if (s.isEmpty() || s.data_ == data_)
		return true;
	if (isEmpty())
		return false;
	return strstr(data_->str_, s.data_->str_) != NULL;
}

bool String::contains(const char *s) const {
	if (s == NULL || *s == 0)
		return true;
	if (isEmpty())
		return false;
	return strstr(data_->str_, s) != NULL;
}

bool String::startsWith(const String& s) const {
	if (s.isEmpty() || s.data_ == data_)
		return true;
	else if (s.length() > length())
		return false;
	return strncmp(data_->str_, s.data_->str_, s.data_->size_) == 0;
}

bool String::endsWith(const String& s) const {
	if (s.isEmpty() || s.data_ == data_)
		return true;
	else if (s.length() > length())
		return false;
	return strncmp(data_->str_ + data_->size_ - s.data_->size_, s.data_->str_, s.data_->size_) == 0;
}

/*! \fn String String::left(int to) const
 *
 * Return the left part of this string up to and including the
 * character at position \p to. If \p to is negative it is counted
 * leftward from the end of the string. If it is beyond the end of
 * the string it will return the full string.
 */
String String::left(int to) const {
	if (isEmpty())
		return String();

	if (to == -1)
		return String(*this);
	else if (to < 0) {
		to += data_->size_;
		if (to < 0)
			return String();
	} else if (to >= data_->size_ - 1)
		return String(*this);
	return String(data_->str_, to + 1);
}

/*! \fn String String::right(int from) const
 *
 * Return the right part of this string starting at and including the
 * character at position \p from. If \p from is negative it is counted
 * leftward from the end of the string. If it is beyond the end of
 * the string it will return an empty string.
 */
String String::right(int from) const {
	if (isEmpty())
		return String();

	if (from == 0)
		return String(*this);
	else if (from < 0) {
		from += data_->size_;
		if (from <= 0)
			return String(*this);
	} else if (from >= data_->size_)
		return String();
	return String(data_->str_ + from, data_->size_ - from);
}

/*! \fn String String::mid(int from, int to) const
 *
 * Return the middle part of the string starting at \p from and
 * ending at \p to (both included). If \p from and/or \p to is
 * negative it is countedleftward from the end of the string.
 */
String String::mid(int from, int to) const {
	if (isEmpty())
		return String();

	if (from < 0) {
		from += data_->size_;
		if (from < 0)
			from = 0;
	} else if (from >= data_->size_)
		return String();

	if (to < 0) {
		to += data_->size_;
		if (to < 0)
			return String();
	} else if (to >= data_->size_)
		to = data_->size_ - 1;

	if (to < from)
		return String();

	if (from == 0 && to == data_->size_ - 1)
		return String(*this);

	return String(data_->str_ + from, to - from + 1);
}

/*! \fn int String::toInt() const
 *
 * Convert the start of the string to an integer until we reach the end of the string
 * or a non digit (except for the first character that can be '+' or '-').
 */
int String::toInt() const {
	if (isEmpty())
		return 0;

	int pos = 0, sign = 1, value = 0;
	if (data_->str_[0] == '-') {
		sign = -1;
		++pos;
	} else if (data_->str_[0] == '+')
		++pos;
	while (
		pos < data_->size_ &&
		data_->str_[pos] >= '0' && data_->str_[pos] <= '9'
	) {
		value *= 10;
		value += (data_->str_[pos] - '0');
		++pos;
	}
	return value * sign;
}

/*! \fn void String::deleteChar(int p)
 *
 * Remove the character at position p from the string.
 */
void String::deleteChar(int p) {
	if (p < 0 || p >= length())
		return;

	ensureUnique();
	while (p++ < data_->size_)
		data_->str_[p - 1] = data_->str_[p];
	--data_->size_;
}

/*! \fn void String::setChar(char c, int p)
 *
 * Set character c at position p, replacing the previous character there.
 */
void String::setChar(char c, int p) {
	if (p < 0 || p >= length())
		return;

	ensureUnique();
	data_->str_[p] = c;
}

/*! \fn void String::insertChar(char c, int p)
 *
 * Insert character c before position p.
 */
void String::insertChar(char c, int p) {
	if (p < 0 || p > length())
		return;

	ensureCapacity(length() + 1, true);
	++data_->size_;
	for (int i = data_->size_ ; i > p ; --i)
		data_->str_[i] = data_->str_[i - 1];
	data_->str_[p] = c;
}

/*! \fn void String::replaceChar(char, char)
 *
 * Replace the first char with the second char in this string.
 */
void String::replaceChar(char c1, char c2) {
	if (isEmpty())
		return;

	ensureUnique();
	for (int i = 0 ; i < data_->size_ ; ++i) {
		if (data_->str_[i] == c1)
			data_->str_[i] = c2;
	}
}

/*! \fn int String::findSpace(int from = 0) const
 *
 * Return the index of the first occurance of a space character (as
 * identified by isspace(char)), starting at the given position in
 * the string. Returns -1 if no space is found in the string at or
 * after the given \p from position.
 */
int String::findSpace(int from) const {
	if (isEmpty())
		return -1;
	
	if (from < 0) {
		from += data_->size_;
		if (from < 0)
			from = 0;
	}
	while (from < data_->size_) {
		if (isspace(data_->str_[from]))
			return from;
		++from;
	}
	return -1;
}

/*! \fn int String::findChar(char, int from = 0) const
 *
 * Return the index of the first occurance of the given char, starting
 * at the given position in the string. Returns -1 if the char is not
 * found in the string at or after the given \p from position.
 */
int String::findChar(char c, int from) const {
	if (isEmpty())
		return -1;

	if (from < 0) {
		from += data_->size_;
		if (from < 0)
			from = 0;
	}
	while (from < data_->size_) {
		if (data_->str_[from] == c)
			return from;
		++from;
	}
	return -1;
}

/*! \fn int String::countChar(char, int from = 0) const
 *
 * Return the number of time the given character occurs in this string at
 * or after the given \p from position.
 */
int String::countChar(char c, int from) const {
	if (isEmpty())
		return 0;

	if (from < 0) {
		from += data_->size_;
		if (from < 0)
			from = 0;
	}
	int cpt = 0;
	while (from < data_->size_) {
		if (data_->str_[from++] == c)
			++cpt;
	}
	return cpt;
}

/*! \fn void String::clear()
 *
 * Clears the string, making it empty.
 */
void String::clear() {
	if (data_ != NULL && (--data_->ref_cpt_) == 0)
		delete data_;
	data_ = NULL;
}

/*! \fn void String::trim()
 *
 * Removes trailing and leading whitespaces. Uses isspace() to decide
 * what is whitespace and what not.
 */
void String::trim() {
	if (isEmpty())
		return;

	ensureUnique();

	// Trim trailing whitespace
	while (data_->size_ >= 1 && isspace(data_->str_[data_->size_ - 1]))
		--data_->size_;
	data_->str_[data_->size_] = 0;

	// Trim leading whitespace
	char *t = data_->str_;
	while (isspace(*t))
		++t;

	if (t != data_->str_) {
		data_->size_ -= (t - data_->str_);
		memmove(data_->str_, t, data_->size_ + 1);
	}
}

/*! \fn String String::trimmed() const
 *
 * Return a copy of this string from which the trailing and
 * leading spaces have been removed.
 */
String String::trimmed() const {
	String str(*this);
	str.trim();
	return str;
}

/*! \fn void String::simplify() {
 *
 * Simplify spaces in this string by replacing any number of consecutive
 * space (identified by isspace()) with a single space.
 */
void String::simplify() {
	if (isEmpty())
		return;

	ensureUnique();

	int i = 0, j = 0;
	while (i < data_->size_) {
		if (!isspace(data_->str_[i]))
			data_->str_[j++] = data_->str_[i++];
		else {
			data_->str_[j++] = data_->str_[i++];
			while (i < data_->size_ && isspace(data_->str_[i]))
				++i;
		}
	}
	data_->str_[j] = 0;
	data_->size_ = j;
}

/*! \fn String String::simplified() const
 *
 * Return a copy of this string in which spaces have been simplified by
 * replacing any number of consecutive space (identified by isspace())
 * with a single space.
 */
String String::simplified() const {
	String str(*this);
	str.simplify();
	return str;
}

/*! \fn void String::simplify(char c) {
 *
 * Simplify the given character in this string by replacing any number of consecutive
 * appearance with a single appearance.
 */
void String::simplify(char c) {
	if (isEmpty())
		return;

	ensureUnique();

	int i = 0, j = 0;
	while (i < data_->size_) {
		if (data_->str_[i] != c)
			data_->str_[j++] = data_->str_[i++];
		else {
			data_->str_[j++] = data_->str_[i++];
			while (i < data_->size_ && data_->str_[i] == c)
				++i;
		}
	}
	data_->str_[j] = 0;
	data_->size_ = j;
}

/*! \fn String String::simplified(char) const
 *
 * Return a copy of this string in which the given character has been simplified by
 * replacing any number of consecutive appearance with a single appearance.
 */
String String::simplified(char c) const {
	String str(*this);
	str.simplify(c);
	return str;
}

/*! \fn String String::format(const char *fmt, ...)
 *
 * Print formatted data into a String object. Similar to sprintf,
 * except that it stores the result in (variably sized) String
 * instead of a fixed size buffer.
 */
String String::format(const char *fmt, ...) {
	String output;

	va_list va;
	va_start(va, fmt);
	output = String::vformat(fmt, va);
	va_end(va);

	return output;
}

/*! \fn String String::vformat(const char *fmt, va_list args)
 *
 * Print formatted data into a String object. Similar to vsprintf,
 * except that it stores the result in (variably sized) String
 * instead of a fixed size buffer.
 */
String String::vformat(const char *fmt, va_list args) {
	String output;
	int guess_size = (strlen(fmt) + 10) / 2;

	// MSVC and IRIX don't return the size the full string would take up.
	// MSVC returns -1, IRIX returns the number of characters actually written,
	// which is at the most the size of the buffer minus one, as the string is
	// truncated to fit.

	// We assume MSVC failed to output the correct, null-terminated string
	// if the return value is either -1 or size.
	// For IRIX, because we lack a better mechanism, we assume failure
	// if the return value equals size - 1.
	// The downside to this is that whenever we try to format a string where the
	// size is 1 below the built-in capacity, the size is needlessly increased.

	// Try increasing the size of the string until it fits.
	int needed_len = 0;
	do {
		guess_size *= 2;
		output.ensureCapacity(guess_size - 1, false);
		guess_size = output.data_->capacity_;

		va_list va;
		va_copy(va, args);
		needed_len = vsnprintf(output.data_->str_, guess_size, fmt, va);
		va_end(va);
	} while (needed_len == -1 || needed_len >= guess_size - 1);
	output.data_->size_ = needed_len;

	return output;
}

bool String::operator==(const String &s) const {
	if (data_ == s.data_ || (s.isEmpty() && isEmpty()))
		return true;
	if (length() != s.length())
		return false;

	return (strcmp(data_->str_, s.data_->str_) == 0);
}

bool String::operator==(const char *s) const {
	if (s == NULL)
		return (length() == 0);
	if (isEmpty())
		return (*s == 0);

	return (strcmp(data_->str_, s) == 0);
}

bool String::operator!=(const String &s) const {
	return !(operator==(s));
}

bool String::operator !=(const char *s) const {
	return !(operator==(s));
}

bool operator==(const char* y, const String &x) {
	return (x == y);
}

bool operator!=(const char* y, const String &x) {
	return x != y;
}

String operator+(const String &x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(const char *x, const String &y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(const String &x, const char *y) {
	String temp(x);
	temp += y;
	return temp;
}

String operator+(char x, const String &y) {
	String temp(&x, 1);
	temp += y;
	return temp;
}

String operator+(const String &x, char y) {
	String temp(x);
	temp += y;
	return temp;
}

