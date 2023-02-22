#pragma once

#include <cstdint>
#include <string_view>
#include <string>
#include <stdexcept>

class CowString {
public:
    explicit CowString(const std::string_view& other);
    CowString(const CowString& other);
    CowString(CowString&& other);

    class CowStringElement {
    public:
        CowStringElement(CowString* str, size_t ind) : str_(str), ind_(ind) {
        }

        CowStringElement& operator=(char other) {
            if (other == (*(str_->str_))[ind_]) {
                return *this;
            }
            if (*(str_->cnt_) != 1) {
                --(*(str_->cnt_));
                str_->str_ = new std::string(*(str_->str_));
                str_->cnt_ = new int64_t(1);
            }
            (*(str_->str_))[ind_] = other;
            return *this;
        }

        operator char() const {
            return (*(str_->str_))[ind_];
        }
    private:
        CowString* str_;
        size_t ind_;
    };

    class Iterator {
    public:
        CowStringElement operator*() const {
            return CowStringElement(str_, ind_);
        }
        Iterator(size_t ind, CowString* str) : ind_(ind), str_(str) {
        }
        Iterator& operator++() {
            ++ind_;
            return *this;
        }
        Iterator operator++(int) {
            Iterator now(ind_, str_);
            ++ind_;
            return now;
        }
        Iterator& operator--() {
            --ind_;
            return *this;
        }
        Iterator operator--(int) {
            Iterator now(ind_, str_);
            --ind_;
            return now;
        }

        bool operator==(const Iterator& other) const {
            return (ind_ == other.ind_);
        }
        bool operator!=(const Iterator& other) const {
            return ind_ != other.ind_;
        }

    private:
        size_t ind_;
        CowString* str_;
    };

    class ConstIterator {
    public:
        const char& operator*() const {
            return (*(str_->str_))[ind_];
        }
        ConstIterator(size_t ind, const CowString* str) : ind_(ind), str_(str) {
        }
        ConstIterator& operator++() {
            ++ind_;
            return *this;
        }
        ConstIterator operator++(int) {
            ConstIterator now(ind_, str_);
            ++ind_;
            return now;
        }
        ConstIterator& operator--() {
            --ind_;
            return *this;
        }
        ConstIterator operator--(int) {
            ConstIterator now(ind_, str_);
            --ind_;
            return now;
        }

        bool operator==(const ConstIterator& other) const {
            return (ind_ == other.ind_);
        }
        bool operator!=(const ConstIterator& other) const {
            return ind_ != other.ind_;
        }
    private:
        size_t ind_;
        const CowString* str_;
    };

    char* GetData() const;

    Iterator begin();
    Iterator end();
    ConstIterator begin() const;
    ConstIterator end() const;

    bool operator==(const CowString& other) const;
    bool operator==(const std::string_view& other) const;
    bool operator!=(const CowString& other) const;
    bool operator!=(const std::string_view& other) const;

    CowString operator+(const CowString& other) const;
    CowString operator+(const std::string_view& other) const;
    CowString& operator+=(const CowString& other);
    CowString& operator+=(const std::string_view& other);

    CowString& operator=(const CowString& other);
    CowString& operator=(CowString&& str);

    CowStringElement operator[](size_t position);
    char operator[](size_t position) const;
    char At(size_t position) const;

    operator std::string_view() const;

    ~CowString();

private:
    std::string* str_;
    int64_t* cnt_;
    friend std::ostream& operator<<(std::ostream& stream, const CowString& str) {
        return stream << (*str.str_);
    }
    friend Iterator;
    friend ConstIterator;
};
