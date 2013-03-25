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

#ifndef list_h
#define list_h

#include <assert.h>

template <class T> class ListSharedData {
public:
	ListSharedData();
	ListSharedData(const ListSharedData<T>&);
	~ListSharedData();

	int capacity_;
	int size_;
	T* storage_;

	int ref_cpt_;
};

template<class T> class List {
public:
	List();
	List(const List<T> &list);
	List(const T *data, int n);

	~List();

	List<T>& operator=(const List<T>&);

	int size() const;
	bool isEmpty() const;

	void clear();

	T& operator[](int);
	const T& operator [](int) const;
	const T& at(int) const;
	T& first();
	const T& first() const;
	T& last();
	const T& last() const;

	int indexOf(const T&) const;
	bool contains(const T&) const;

	List<T>& operator<<(const T& value);
	List<T>& operator<<(const List<T>& list);

	List<T>& operator+=(const T& value);
	List<T>& operator+=(const List<T>& list);

	void append(const T&);
	void prepend(const T&);
	void insert(int i, const T&);
	void replace(int i, const T&);

	T takeAt(int);
	T takeFirst();
	T takeLast();

	void removeAt(int);
	void removeFirst();
	void removaLast();

	bool operator==(const List<T>&) const;
	bool operator!=(const List<T>&) const;

	typedef T *iterator;
	typedef const T *const_iterator;

	iterator begin() {
		ensureUnique();
		return data_ == NULL ? NULL : data_->storage_;
	}
	const_iterator begin() const {
		return data_ == NULL ? NULL : data_->storage_;
	}
	iterator end() {
		ensureUnique();
		return data_ == NULL ? NULL : data_->storage_ + data_->size_;
	}
	const_iterator end() const {
		return data_ == NULL ? NULL : data_->storage_ + data_->size_;
	}

protected:
	void ensureUnique();
	void ensureCapacity(int);

private:
	ListSharedData<T>* data_;
};

#include "list.hpp"


#endif // str_h
