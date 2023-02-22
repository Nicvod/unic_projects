#pragma once

#include <vector>
#include <cstdint>
#include <iostream>
// NOLINTBEGIN
template <typename T>
class Writer {
public:
    explicit Writer(T& stream);

    void PutBite(bool bite);

    void PutNBites(const std::vector<bool>& vec);

    ~Writer();

private:
    T* stream_;
    unsigned char group_;
    size_t cur_ind_;  // index of the not write character in group_

    void Write();
};

template <typename T>
Writer<T>::Writer(T& stream) : stream_(&stream), group_(0), cur_ind_(8){};

template <typename T>
void Writer<T>::PutBite(bool bite) {

    if (cur_ind_ == 0) {
        Write();
    }
    group_ |= (bite << (--cur_ind_));
}

template <typename T>
void Writer<T>::PutNBites(const std::vector<bool>& vec) {
    size_t x = 0;
    while (x < vec.size()) {
        if (cur_ind_ == 0) {
            Write();
        }
        group_ |= (vec[x++] << (--cur_ind_));
    }
}

template <typename T>
Writer<T>::~Writer() {
    if (cur_ind_ != 8) {
        Write();
    }
}

template <typename T>
void Writer<T>::Write() {
    (*stream_) << (group_);
    group_ = 0;
    cur_ind_ = 8;
}
// NOLINTEND