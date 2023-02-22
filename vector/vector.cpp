#include "vector.h"

Vector::Iterator::Iterator(Vector::ValueType *pointer) : current_(pointer) {
}
Vector::Iterator::Iterator() : current_(nullptr) {
}

Vector::ValueType &Vector::Iterator::operator*() const {
    return *current_;
}
Vector::ValueType *Vector::Iterator::operator->() const {
    return current_;
}

Vector::Iterator &Vector::Iterator::operator=(Vector::Iterator other) {
    current_ = other.current_;
    return *this;
}

Vector::Iterator &Vector::Iterator::operator++() {
    ++current_;
    return *this;
}
Vector::Iterator Vector::Iterator::operator++(int) {
    Iterator now(*this);
    ++current_;
    return now;
}
Vector::Iterator &Vector::Iterator::operator--() {
    --current_;
    return *this;
}
Vector::Iterator Vector::Iterator::operator--(int) {
    Iterator now(*this);
    --current_;
    return now;
}

Vector::DifferenceType Vector::Iterator::operator-(Vector::Iterator other) {
    return (current_ - other.current_);
}
Vector::Iterator Vector::Iterator::operator+(Vector::DifferenceType shift) {
    Iterator now(*this);
    now.current_ += shift;
    return now;
}
Vector::Iterator Vector::Iterator::operator-(Vector::DifferenceType shift) {
    Iterator now(*this);
    now.current_ -= shift;
    return now;
}
Vector::Iterator &Vector::Iterator::operator+=(Vector::DifferenceType shift) {
    return *this = *this + shift;
}
Vector::Iterator &Vector::Iterator::operator-=(Vector::DifferenceType shift) {
    return *this = *this - shift;
}

bool Vector::Iterator::operator==(const Vector::Iterator &other) const {
    return (current_ == other.current_);
}
bool Vector::Iterator::operator!=(const Vector::Iterator &other) const {
    return (current_ != other.current_);
}
std::strong_ordering Vector::Iterator::operator<=>(const Vector::Iterator &other) const {
    if (current_ == other.current_) {
        return std::strong_ordering::equal;
    } else if (current_ < other.current_) {
        return std::strong_ordering::less;
    } else {
        return std::strong_ordering::greater;
    }
}

Vector::Vector() : size_(0), capacity_(0), data_(nullptr) {
}
Vector::Vector(size_t size) : size_(size), capacity_(size), data_(new ValueType[size]) {
    for (SizeType i = 0; i < size; ++i) {
        data_[i] = 0;
    }
}
Vector::Vector(std::initializer_list<ValueType> list) {
    size_ = list.size();
    capacity_ = list.size();
    data_ = new ValueType[capacity_];
    SizeType ind = 0;
    for (const auto &ini_component : list) {
        data_[ind] = ini_component;
        ++ind;
    }
}
Vector::Vector(const Vector &other) {
    if (*this != other) {
        size_ = other.size_;
        capacity_ = other.size_;
        data_ = new ValueType[capacity_];
        for (SizeType i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }
}

Vector &Vector::operator=(const Vector &other) {
    if (*this == other) {
        return *this;
    }
    delete[] data_;
    size_ = other.size_;
    capacity_ = other.size_;
    data_ = new ValueType[capacity_];
    for (SizeType i = 0; i < size_; ++i) {
        data_[i] = other.data_[i];
    }
    return *this;
}

Vector::~Vector() {
    delete[] data_;
}

Vector::SizeType Vector::Size() const {
    return size_;
}
Vector::SizeType Vector::Capacity() const {
    return capacity_;
}

const Vector::ValueType *Vector::Data() const {
    return data_;
}

Vector::ValueType &Vector::operator[](size_t position) {
    return data_[position];
}
Vector::ValueType Vector::operator[](size_t position) const {
    return data_[position];
}

bool Vector::operator==(const Vector &other) const {
    if (size_ != other.size_) {
        return false;
    }
    for (SizeType i = 0; i < size_; ++i) {
        if (data_[i] != other.data_[i]) {
            return false;
        }
    }
    return true;
}
bool Vector::operator!=(const Vector &other) const {
    return !(*this == other);
}
std::strong_ordering Vector::operator<=>(const Vector &other) const {
    SizeType lhs_ind = 0;
    SizeType rhs_ind = 0;
    while (lhs_ind < size_ && rhs_ind < other.size_) {
        if (data_[lhs_ind] < other.data_[rhs_ind]) {
            return std::strong_ordering::less;
        } else if (data_[lhs_ind] > other.data_[rhs_ind]) {
            return std::strong_ordering::greater;
        }
        ++lhs_ind;
        ++rhs_ind;
    }
    if (size_ == other.size_) {
        return std::strong_ordering::equal;
    } else if (size_ < other.size_) {
        return std::strong_ordering::less;
    } else {
        return std::strong_ordering::greater;
    }
}

void Vector::Reserve(Vector::SizeType new_capacity) {
    if (new_capacity > capacity_) {
        capacity_ = new_capacity;
        ValueType *tmp_data = new ValueType[capacity_];
        for (SizeType i = 0; i < size_; ++i) {
            tmp_data[i] = data_[i];
        }
        delete[] data_;
        data_ = tmp_data;
    }
}
void Vector::Clear() {
    size_ = 0;
}

void Vector::PushBack(const Vector::ValueType &new_element) {
    if (size_ == capacity_) {
        if (capacity_ == 0) {
            capacity_ = 1;
        } else {
            capacity_ *= 2;
        }
        ValueType *tmp_data = new ValueType[capacity_];
        for (SizeType i = 0; i < size_; ++i) {
            tmp_data[i] = data_[i];
        }
        delete[] data_;
        data_ = tmp_data;
    }
    data_[size_] = new_element;
    ++size_;
}
void Vector::PopBack() {
    --size_;
}

void Vector::Swap(Vector &other) {
    std::swap(size_, other.size_);
    std::swap(capacity_, other.capacity_);
    std::swap(data_, other.data_);
}

Vector::Iterator Vector::begin() {
    return Iterator(data_);
}
Vector::Iterator Vector::end() {
    return Iterator(data_ + size_);
}
Vector::Iterator Vector::Begin() {
    return Iterator(data_);
}
Vector::Iterator Vector::End() {
    return Iterator(data_ + size_);
}