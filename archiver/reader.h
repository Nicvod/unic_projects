#pragma once

#include <vector>
#include <cstdint>
#include <iostream>
// NOLINTBEGIN
template <typename T>
class Reader {
public:
    explicit Reader(T& stream);

    int GetBite();

    bool GetNBites(std::vector<bool>& res, size_t n);

    void ChangeStream(T& stream);

private:
    T* stream_;
    unsigned char group_;
    size_t cur_ind_;  // index of the unread character in group_

    bool Read();
};

template <typename T>
Reader<T>::Reader(T& stream) : stream_(&stream), group_(0), cur_ind_(0){};

template <typename T>
int Reader<T>::GetBite() {
    if (cur_ind_ == 0) {
        if (!Read()) {
            return -1;
        }
    }
    return 1 & (group_ >> (--cur_ind_));
}

template <typename T>
bool Reader<T>::GetNBites(std::vector<bool>& res, size_t n) {
    res.assign(n, false);
    size_t x = 0;
    while (x < n) {
        if (cur_ind_ == 0) {
            if (!Read()) {
                return false;
            }
        }
        res[x++] = 1 & (group_ >> (--cur_ind_));
    }
    return true;
}

template <typename T>
void Reader<T>::ChangeStream(T& stream) {
    stream_ = &stream;
    cur_ind_ = 0;
    group_ = 0;
}

template <typename T>
bool Reader<T>::Read() {
    char tmp = 0;
    if ((*stream_).eof() || (*stream_).bad() || (*stream_).fail()) {
        return false;
    }
    if ((*stream_).read(&tmp, 1)) {
        group_ = tmp;
        cur_ind_ = 8;
        return true;
    } else {
        return false;
    }
}
// NOLINTEND