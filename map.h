/*
 * Copyright (C) 2013 Thierry Crozat
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

#include <assert.h>

#ifndef map_h
#define map_h

#include "list.h"

template <class Key, class Value> class Map {
public:
	Map();
	Map(const Map&);
	~Map();

	Map& operator=(const Map&);

	bool contains(const Key&) const;
	const Value& operator[](const Key&) const;
	Value& operator[](const Key&);

	bool isEmpty() const;
	int size() const;
	const List<Key>& keys() const;

	bool remove(const Key&);

	void clear();

private:
	List<Key> keys_;
	List<Value> values_;
};

template <class Key, class Value> Map<Key, Value>::Map() {
}

template <class Key, class Value> Map<Key, Value>::Map(const Map<Key, Value>& other) :
	keys_(other.keys_), values_(other.values)
{
}

template <class Key, class Value> Map<Key, Value>::~Map() {
}

template <class Key, class Value> Map<Key, Value>& Map<Key, Value>::operator=(const Map<Key, Value>& other) {
	if (&other != this) {
		keys_ = other.keys_;
		values_ = other.values_;
	}
	return *this;
}

template <class Key, class Value> bool Map<Key, Value>::contains(const Key& key) const {
	return keys_.contains(key);
}

template <class Key, class Value> const Value& Map<Key, Value>::operator[](const Key& key) const {
	int i = keys_.indexOf(key);
	assert(i != -1);
	return values_[i];
}

template <class Key, class Value> Value& Map<Key, Value>::operator[](const Key& key) {
	int i = keys_.indexOf(key);
	if (i != -1)
		return values_[i];
	keys_ << key;
	values_ << Value();
	return values_.last();
}

template <class Key, class Value> bool Map<Key, Value>::isEmpty() const {
	return keys_.isEmpty();
}

template <class Key, class Value> int Map<Key, Value>::size() const {
	return keys_.size();
}

template <class Key, class Value> const List<Key>& Map<Key, Value>::keys() const {
	return keys_;
}

template <class Key, class Value> bool Map<Key, Value>::remove(const Key& key) {
	int i = keys_.indexOf(key);
	if (i == -1)
		return false;
	keys_.removeAt(i);
	values_.removeAt(i);
	return true;
}

template <class Key, class Value> void Map<Key, Value>::clear() {
	keys_.clear();
	values_.clear();
}

#endif

