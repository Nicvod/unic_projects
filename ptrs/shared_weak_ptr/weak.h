#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

#include <cstddef>
#include <type_traits>

template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr();

    WeakPtr(const WeakPtr& other);
    WeakPtr(WeakPtr&& other);

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other);
    WeakPtr& operator=(WeakPtr&& other);

    WeakPtr& operator=(const SharedPtr<T>& other) {
        if (block_) {
            block_->DecWeak();
        }
        block_ = other.block_;
        object_ = other.object_;
        if (block_) {
            block_->IncWeak();
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset();
    void Swap(WeakPtr& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const;
    bool Expired() const;
    SharedPtr<T> Lock() const;

private:
    template <typename T2>
    friend class SharedPtr;

    BaseControlBlock* block_;
    T* object_;
};

template <typename T>
WeakPtr<T>::WeakPtr() : block_(nullptr), object_(nullptr) {
}

template <typename T>
WeakPtr<T>::WeakPtr(const WeakPtr& other) : block_(other.block_), object_(other.object_) {
    if (block_) {
        block_->IncWeak();
    }
}
template <typename T>
WeakPtr<T>::WeakPtr(WeakPtr&& other) : block_(other.block_), object_(other.object_) {
    other.block_ = nullptr;
    other.object_ = nullptr;
}

template <typename T>
WeakPtr<T>::WeakPtr(const SharedPtr<T>& other) : block_(other.block_), object_(other.object_) {
    if (block_) {
        block_->IncWeak();
    }
}

template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(const WeakPtr& other) {
    if (this == &other) {
        return *this;
    }
    if (block_) {
        block_->DecWeak();
    }
    block_ = other.block_;
    object_ = other.object_;
    if (block_) {
        block_->IncWeak();
    }
    return *this;
}
template <typename T>
WeakPtr<T>& WeakPtr<T>::operator=(WeakPtr&& other) {
    if (this == &other) {
        return *this;
    }
    if (block_) {
        block_->DecWeak();
    }
    block_ = other.block_;
    object_ = other.object_;
    other.block_ = nullptr;
    other.object_ = nullptr;
    return *this;
}

template <typename T>
WeakPtr<T>::~WeakPtr() {
    if (block_) {
        block_->DecWeak();
    }
    block_ = nullptr;
    object_ = nullptr;
}

template <typename T>
void WeakPtr<T>::Reset() {
    if (block_) {
        block_->DecWeak();
    }
    block_ = nullptr;
    object_ = nullptr;
}
template <typename T>
void WeakPtr<T>::Swap(WeakPtr& other) {
    std::swap(block_, other.block_);
    std::swap(object_, other.object_);
}

template <typename T>
size_t WeakPtr<T>::UseCount() const {
    if (!block_) {
        return 0;
    }
    return block_->GetStrongCnt();
}
template <typename T>
bool WeakPtr<T>::Expired() const {
    return (UseCount() == 0);
}
template <typename T>
SharedPtr<T> WeakPtr<T>::Lock() const {
    return Expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
}
