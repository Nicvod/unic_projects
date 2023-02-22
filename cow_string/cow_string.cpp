#include "cow_string.h"

CowString::CowString(const std::string_view& other) {
    str_ = new std::string(other);
    cnt_ = new int64_t(1);
}
CowString::CowString(const CowString& other) {
    str_ = other.str_;
    cnt_ = other.cnt_;
    ++(*cnt_);
}
CowString::CowString(CowString&& other) {
    str_ = other.str_;
    cnt_ = other.cnt_;
    other.str_ = nullptr;
    other.cnt_ = nullptr;
}

CowString::Iterator CowString::begin() {
    return Iterator(0, this);
}
CowString::ConstIterator CowString::begin() const {
    return ConstIterator(0, this);
}
CowString::Iterator CowString::end() {
    return Iterator((*str_).size(), this);
}
CowString::ConstIterator CowString::end() const {
    return ConstIterator((*str_).size(), this);
}

char* CowString::GetData() const {
    return str_->data();
}

bool CowString::operator==(const CowString& other) const {
    return (*str_) == (*other.str_);
}
bool CowString::operator==(const std::string_view& other) const {
    return (*str_) == (other);
}
bool CowString::operator!=(const CowString& other) const {
    return (*str_) != *(other.str_);
}
bool CowString::operator!=(const std::string_view& other) const {
    return (*str_) != other;
}

CowString CowString::operator+(const CowString& other) const {
    CowString tmp(*(this->str_));
    *tmp.str_ += *(other.str_);
    return tmp;
}
CowString CowString::operator+(const std::string_view& other) const {
    CowString tmp(*(this->str_));
    *tmp.str_ += other;
    return tmp;
}
CowString& CowString::operator+=(const CowString& other) {
    if ((*cnt_) == 1) {
        *str_ += *other.str_;
        return *this;
    }
    return *this = *this + other;
}
CowString& CowString::operator+=(const std::string_view& other) {
    if ((*cnt_) == 1) {
        *str_ += other;
        return *this;
    }
    return *this = *this + other;
}

CowString& CowString::operator=(const CowString& other) {
    if (this == &other) {
        return *this;
    }
    if (cnt_ != nullptr) {
        --(*cnt_);
        if ((*cnt_) == 0) {
            delete str_;
            delete cnt_;
        }
    }
    str_ = other.str_;
    cnt_ = other.cnt_;
    ++(*cnt_);
    return *this;
}
CowString& CowString::operator=(CowString&& other) {
    if (this == &other) {
        return *this;
    }
    std::swap(cnt_, other.cnt_);
    std::swap(str_, other.str_);
    return *this;
}

CowString::CowStringElement CowString::operator[](size_t position) {
    if (position >= (*str_).size()) {
        throw std::out_of_range ("Bad index");
    }
    return CowStringElement(this, position);
}
char CowString::operator[](size_t position) const {
    if (position >= (*str_).size()) {
        throw std::out_of_range ("Bad index");
    }
    return (*str_)[position];
}
char CowString::At(size_t position) const {
    if (position >= (*str_).size()) {
        throw std::out_of_range ("Bad index");
    }
    return (*str_)[position];
}

CowString::operator std::string_view() const {
    return (*str_);
}

CowString::~CowString() {
    if (cnt_ != nullptr) {
        --(*cnt_);
        if ((*cnt_) == 0) {
            delete str_;
            delete cnt_;
        }
    }
}