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

#ifndef str_h
#define str_h

#include <stdarg.h>
#include <string.h>

#ifndef NULL
#define NULL 0
#endif

class StringSharedData {
public:
	StringSharedData() : size_(0), str_(NULL), capacity_(0), ref_cpt_(0) {}
	StringSharedData(const StringSharedData& d) : size_(0), str_(NULL), capacity_(0), ref_cpt_(0) {
		size_ = d.size_;
		capacity_ = d.capacity_;
		if (capacity_ > 0) {
			str_ = new char[capacity_];
			memcpy(str_, d.str_, size_+1);
		}
	}
	~StringSharedData() {delete [] str_;}

	int size_;
	char* str_;
	int capacity_;

	int ref_cpt_;
};

class String {
public:
	String();
	String(const char *str);
	String(const char *str, int len);
	String(const String &str);

	~String();

	String &operator=(const char *str);
	String &operator=(const String &str);

	String &operator+=(const char *str);
	String &operator+=(const String &str);
	String &operator+=(char c);

	bool operator==(const String &x) const;
	bool operator==(const char *x) const;
	bool operator!=(const String &x) const;
	bool operator!=(const char *x) const;

	bool contains(const String &x) const;
	bool contains(const char *x) const;
	bool startsWith(const String&) const;
	bool endsWith(const String&) const;

	const char* const c_str() const;
	int length() const;
	bool isEmpty() const;
	char operator[](int idx) const;

	String left(int to) const;
	String right(int from) const;
	String mid(int from, int to) const;

	int toInt() const;

	void deleteChar(int p);
	void setChar(char c, int p);
	void insertChar(char c, int p);
	void replaceChar(char, char);

	int findSpace(int from = 0) const;
	int findChar(char, int from = 0) const;
	int countChar(char, int from = 0) const;

	void clear();

	void trim();
	String trimmed() const;

	void simplify();
	String simplified() const;
	void simplify(char);
	String simplified(char) const;

	static String format(const char *fmt, ...);
	static String vformat(const char *fmt, va_list args);

protected:
	void ensureUnique();
	void ensureCapacity(int new_size, bool keep_old);
	void initWithCStr(const char *str, int len);

private:
	StringSharedData* data_;

	static const char* const nullStr;
};

String operator+(const String &x, const String &y);
String operator+(const char *x, const String &y);
String operator+(const String &x, const char *y);
String operator+(const String &x, char y);
String operator+(char x, const String &y);

bool operator==(const char *x, const String &y);
bool operator!=(const char *x, const String &y);

inline const char* const String::c_str() const {
	return ((data_ == NULL || data_->str_ == NULL) ? nullStr : data_->str_);
}

inline int String::length() const {
	return (data_ == NULL ? 0 : data_->size_);
}

inline bool String::isEmpty() const {
	return (data_ == NULL || data_->size_ == 0);
}

inline char String::operator[](int idx) const {
	if (idx < 0 || idx >= length())
		return 0;
	return data_->str_[idx];
}

#endif // str_h
