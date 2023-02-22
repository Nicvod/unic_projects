#pragma once

#include <vector>
#include <stdexcept>
// NOLINTBEGIN
// Declaration
template <typename T>
class QueuePriority {
public:
    QueuePriority() = default;
    QueuePriority(std::initializer_list<T> list);

    bool empty() const;   // NOLINT
    size_t size() const;  // NOLINT

    void push(T new_element);  // NOLINT
    T top() const;             // NOLINT
    void pop();                // NOLINT // erase min_element from queue

private:
    std::vector<T> data_;  // left_child_id = (2*index+1); right_child_id = (2*index+2); parent_id = (index - 1) / 2

    size_t ParentInd(size_t cur_ind) const;
    size_t LeftChildInd(size_t cur_ind) const;
    size_t RightChildInd(size_t cur_ind) const;

    bool CheckChildren(
        size_t cur_ind) const;             // Returns true if one of the children is smaller than us, else returns false
    size_t SwapWithChild(size_t cur_ind);  // swap us with a smaller child and returns its index
};

// Realisation
template <typename T>
QueuePriority<T>::QueuePriority(std::initializer_list<T> list) : data_(list) {
    sort(data_.begin(), data_.end());
}

template <typename T>
bool QueuePriority<T>::empty() const {
    return data_.empty();
}

template <typename T>
size_t QueuePriority<T>::size() const {
    return data_.size();
}

template <typename T>
void QueuePriority<T>::push(T new_element) {
    size_t cur_ind = data_.size();
    data_.push_back(new_element);
    while (cur_ind > 0 && new_element < data_[ParentInd(cur_ind)]) {
        std::swap(data_[cur_ind], data_[ParentInd(cur_ind)]);
        cur_ind = ParentInd(cur_ind);
    }
}

template <typename T>
T QueuePriority<T>::top() const {
    if (data_.empty()) {
        throw std::out_of_range("priority_queue is empty");
        ;
    }
    return data_[0];
}

template <typename T>
void QueuePriority<T>::pop() {
    if (data_.empty()) {
        throw std::out_of_range("priority_queue is empty");
    }
    std::swap(data_[0], data_.back());
    data_.pop_back();
    if (!data_.empty()) {
        size_t cur_ind = 0;
        while (CheckChildren(cur_ind)) {
            cur_ind = SwapWithChild(cur_ind);
        }
    }
}

template <typename T>
size_t QueuePriority<T>::ParentInd(size_t cur_ind) const {
    return ((cur_ind - 1) / 2);
}
template <typename T>
size_t QueuePriority<T>::LeftChildInd(size_t cur_ind) const {
    return (cur_ind * 2 + 1);
}
template <typename T>
size_t QueuePriority<T>::RightChildInd(size_t cur_ind) const {
    return (cur_ind * 2 + 2);
}

template <typename T>
bool QueuePriority<T>::CheckChildren(size_t cur_ind) const {
    return ((LeftChildInd(cur_ind) < data_.size() && data_[LeftChildInd(cur_ind)] < data_[cur_ind]) ||
            ((RightChildInd(cur_ind)) < data_.size() && data_[RightChildInd(cur_ind)] < data_[cur_ind]));
}

template <typename T>
size_t QueuePriority<T>::SwapWithChild(size_t cur_ind) {
    if ((RightChildInd(cur_ind)) >= data_.size() || data_[LeftChildInd(cur_ind)] < data_[RightChildInd(cur_ind)]) {
        std::swap(data_[LeftChildInd(cur_ind)], data_[cur_ind]);
        return LeftChildInd(cur_ind);
    } else {
        std::swap(data_[RightChildInd(cur_ind)], data_[cur_ind]);
        return RightChildInd(cur_ind);
    }
}
// NOLINTEND