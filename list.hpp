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

template <class T> List<T>::List() :
	data_(NULL)
{
}

template <class T> List<T>::List(const List<T> &list) :
	data_(list.data_)
{
	if (data_ != NULL)
		++data_->ref_cpt_;
}

template<class T> List<T>::List(const T* data, int n) :
	data_(NULL)
{
	ensureCapacity(n);
	for (int i = 0 ; i < n ; ++i)
		data_->storage_[i] = data[i];
	data_->size_ = n;
}

template<class T> List<T>::~List() {
	clear();
}

template<class T> List<T>& List<T>::operator=(const List<T>& list) {
	if (&list == this || list.data_ == data_)
		return *this;

	clear();
	data_ = list.data_;
	if (data_ != NULL)
		++data_->ref_cpt_;
	return *this;
}

template<class T> void List<T>::ensureUnique() {
	if (data_ != NULL && data_->ref_cpt_ > 1) {
		--data_->ref_cpt_;
		data_ = new ListSharedData<T>(*data_);
		++data_->ref_cpt_;
	}
}

template<class T> void List<T>::ensureCapacity(int new_size) {
	ensureUnique();
	
	int cur_capacity = data_ == NULL ? 0 : data_->capacity_;
	if (new_size < cur_capacity)
		return;

	int new_capacity = new_size;
	if (new_capacity < cur_capacity * 2)
		new_capacity = cur_capacity * 2;

	T* new_storage = new T[new_capacity];
	int size = 0;
	if (data_ != NULL) {
		if (data_->size_ < new_size)
			size = data_->size_;
		else
			size = new_size;
		for (int i = 0 ; i < size ; ++i)
			new_storage[i] = data_->storage_[i];
		delete [] data_->storage_;
	} else {
		data_ = new ListSharedData<T>();
		++data_->ref_cpt_;
	}
	
	data_->storage_ = new_storage;
	data_->size_ = size;
	data_->capacity_ = new_capacity;
}

template<class T> void List<T>::clear() {
	if (data_ != NULL && --data_->ref_cpt_ == 0)
		delete data_;
	data_ = NULL;
}

template<class T> int List<T>::size() const {
	return (data_ == NULL ? 0 : data_->size_);
}

template<class T> bool List<T>::isEmpty() const {
	return (data_ == NULL || data_->size_ == 0);
}

template<class T> T& List<T>::operator[](int index) {
	assert(index >= 0 && index < size());
	ensureUnique();
	return data_->storage_[index];
}

template<class T> const T& List<T>::operator [](int index) const {
	assert(index >= 0 && index < size());
	return data_->storage_[index];
}

template<class T> const T& List<T>::at(int index) const {
	return operator[](index);
}

template<class T> T& List<T>::first() {
	return operator[](0);
}

template<class T> const T& List<T>::first() const {
	return operator[](0);
}

template<class T> T& List<T>::last() {
	return operator[](size()-1);
}

template<class T> const T& List<T>::last() const {
	return operator[](size()-1);
}

template<class T> int List<T>::indexOf(const T& value) const {
	if (isEmpty())
		return -1;
	for (int i = 0 ; i < size() ; ++i) {
		if (value == data_->storage_[i])
			return i;
	}
	return -1;
}

template<class T> bool List<T>::contains(const T& value) const {
	return indexOf(value) != -1;
}

template<class T> List<T>& List<T>::operator<<(const T& value) {
	ensureCapacity(size() + 1);
	data_->storage_[data_->size_] = value;
	++data_->size_;
	return *this;
}

template<class T> List<T>& List<T>::operator<<(const List<T>& list) {
	if (&list == this)
		return operator<<(List<T>(list));

	ensureCapacity(size() + list.size());
	for (int i = 0 ; i < list.size() ; ++i)
		data_->storage_[data_->size_ + i] = list[i];
	data_->size_ += list.size();
	return *this;
}

template<class T> List<T>& List<T>::operator+=(const T& value) {
	return operator<<(value);
}

template<class T> List<T>& List<T>::operator+=(const List<T>& list) {
	return operator<<(list);
}

template<class T> void List<T>::insert(int index, const T& value) {
	assert(index >= 0 && index <= size());
	ensureCapacity(size() + 1);
	for (int i = data_->size() ; i > index ; --i)
		data_->storage_[i] = data_->storage_[i-1];
	data_->storage_[index] = value;
	++data_->size_;
}

template<class T> void List<T>::append(const T& value) {
	operator<<(value);
}

template<class T> void List<T>::prepend(const T& value) {
	insert(0, value);
}

template<class T> void List<T>::replace(int index, const T& value) {
	assert(index >= 0 && index < size());
	ensureUnique();
	data_->storage_[index] = value;
}

template<class T> T List<T>::takeAt(int index) {
	assert(index >= 0 && index < size());
	ensureUnique();
	T value = data_->storage_[index];
	for (int i = index + 1 ; i < data_->size_ ; ++i)
		data_->storage_[i-1] = data_->storage_[i];
	--data_->size_;
	return value;
}

template<class T> T List<T>::takeFirst() {
	return takeAt(0);
}

template<class T> T List<T>::takeLast() {
	return takeAt(size() - 1);
}

template<class T> void List<T>::removeAt(int index) {
	assert(index >= 0 && index < size());
	ensureUnique();
	for (int i = index + 1 ; i < data_->size_ ; ++i)
		data_->storage_[i-1] = data_->storage_[i];
	--data_->size_;
}

template<class T> void List<T>::removeFirst() {
	removeAt(0);
}

template<class T> void List<T>::removaLast() {
	removeAt(size() - 1);
}

template<class T> bool List<T>::operator==(const List<T>& list) const {
	if (data_ == list.data_)
		return true;

	if (size() != list.size())
		return false;
	for (int i = 0 ; i < size() ; ++i) {
		if (data_->storage_[i] != list[i])
			return false;
	}
	return true;
}

template<class T> bool List<T>::operator!=(const List<T>& list) const {
	return !(*this == list);
}


template<class T> ListSharedData<T>::ListSharedData() :
	capacity_(0), size_(0), storage_(NULL), ref_cpt_(0)
{
}

template<class T> ListSharedData<T>::ListSharedData(const ListSharedData<T>& other) :
capacity_(other.size_), size_(other.size_), storage_(NULL), ref_cpt_(0)
{
	if (size_ > 0) {
		storage_ = new T[size_];
		for (int i = 0 ; i < size_ ; ++i)
			storage_[i] = other.storage_[i];
	}
}

template<class T> ListSharedData<T>::~ListSharedData() {
	delete [] storage_;
}
