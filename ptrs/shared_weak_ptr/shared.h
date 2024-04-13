#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <new>
#include <stdexcept>
#include <type_traits>
#include <locale>

class BaseControlBlock {
public:
    BaseControlBlock(size_t strong_cnt = 0, size_t weak_cnt = 0)
        : strong_cnt_(strong_cnt), weak_cnt_(weak_cnt) {
    }
    void IncStrong() {
        ++strong_cnt_;
    }
    void DecStrong() {
        --strong_cnt_;
        if (strong_cnt_ == 0 && weak_cnt_ == 0) {
            delete this;
        } else if (strong_cnt_ == 0) {
            this->Destroy();
        }
    }

    void IncWeak() {
        ++weak_cnt_;
    }
    void DecWeak() {
        --weak_cnt_;
        if (strong_cnt_ == 0 && weak_cnt_ == 0) {
            delete this;
        }
    }

    size_t GetStrongCnt() const {
        return strong_cnt_;
    }
    size_t GetWeakCnt() const {
        return weak_cnt_;
    }

    virtual ~BaseControlBlock() = default;
    virtual void Destroy() {
    }

private:
    size_t strong_cnt_;
    size_t weak_cnt_;
};

template <typename T>
class PointerControlBlock : public BaseControlBlock {
public:
    PointerControlBlock(T* ptr) : BaseControlBlock(1, 0), ptr_(ptr) {
    }

    ~PointerControlBlock() {
        delete ptr_;
        ptr_ = nullptr;
    }

    void Destroy() {
        delete ptr_;
        ptr_ = nullptr;
    }

private:
    T* ptr_;
};

template <typename T>
class PointerControlBlock<T[]> : public BaseControlBlock {
public:
    PointerControlBlock(T* ptr) : BaseControlBlock(1, 0), ptr_(ptr) {
    }

    ~PointerControlBlock() {
        delete[] ptr_;
        ptr_ = nullptr;
    }

    void Destroy() {
        delete[] ptr_;
        ptr_ = nullptr;
    }

private:
    T* ptr_;
};

template <typename T>
class ObjectControlBlock : public BaseControlBlock {
public:
    template <typename... Args>
    ObjectControlBlock(Args&&... args) : BaseControlBlock(1, 0), is_destroyed_(false) {
        ::new (&stor_) T(std::forward<Args>(args)...);
    }

    T* GetObject() {
        return std::launder(reinterpret_cast<T*>(&stor_));
    }

    ~ObjectControlBlock() {
        if (!is_destroyed_) {
            std::destroy_at(std::launder(reinterpret_cast<T*>(&stor_)));
            is_destroyed_ = true;
        }
    }

    void Destroy() {
        if (!is_destroyed_) {
            std::destroy_at(std::launder(reinterpret_cast<T*>(&stor_)));
            is_destroyed_ = true;
        }
    }

private:
    bool is_destroyed_;
    std::aligned_storage_t<sizeof(T), alignof(T)> stor_;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr();
    SharedPtr(std::nullptr_t);
    explicit SharedPtr(T* ptr);

    SharedPtr(BaseControlBlock* block, T* ptr);

    template <typename T2>
    explicit SharedPtr(T2* ptr) : block_(new PointerControlBlock<T2>(ptr)), object_(ptr) {
    }

    SharedPtr(const SharedPtr& other);
    SharedPtr(SharedPtr&& other) noexcept;

    template <typename T2>
    SharedPtr(const SharedPtr<T2>& other) : block_(other.block_), object_(other.object_) {
        if (block_) {
            block_->IncStrong();
        }
    }
    template <typename T2>
    SharedPtr(SharedPtr<T2>&& other) noexcept : block_(other.block_), object_(other.object_) {
        other.block_ = nullptr;
        other.object_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        block_ = other.block_;
        object_ = ptr;
        if (block_) {
            block_->IncStrong();
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other);
    SharedPtr& operator=(SharedPtr&& other) noexcept;

    template <typename T2>
    SharedPtr& operator=(const SharedPtr<T2>& other) {
        if (block_) {
            block_->DecStrong();
        }
        block_ = other.block_;
        object_ = other.object_;
        if (block_) {
            block_->IncStrong();
        }
        return *this;
    }
    template <typename T2>
    SharedPtr& operator=(SharedPtr<T2>&& other) {
        if (block_) {
            block_->DecStrong();
        }
        block_ = other.block_;
        object_ = other.object_;
        other.block_ = nullptr;
        other.object_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr();

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset();
    void Reset(T* ptr);
    void Swap(SharedPtr& other);

    template <typename T2>
    requires(std::is_base_of_v<T, T2>) void Reset(T2* ptr) {
        if (block_) {
            block_->DecStrong();
        }
        block_ = new PointerControlBlock<T2>(ptr);
        object_ = ptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const;
    T& operator*() const;
    T* operator->() const;
    size_t UseCount() const;
    explicit operator bool() const;

private:
    template <typename T2>
    friend class SharedPtr;

    template <typename T2>
    friend class WeakPtr;

    BaseControlBlock* block_;
    T* object_;
};

template <typename T>
SharedPtr<T>::SharedPtr() : block_(nullptr), object_(nullptr) {
}
template <typename T>
SharedPtr<T>::SharedPtr(std::nullptr_t) : block_(nullptr), object_(nullptr) {
}
template <typename T>
SharedPtr<T>::SharedPtr(T* ptr) : block_(new PointerControlBlock<T>(ptr)), object_(ptr) {
}

template <typename T>
SharedPtr<T>::SharedPtr(const SharedPtr& other) : block_(other.block_), object_(other.object_) {
    if (block_) {
        block_->IncStrong();
    }
}
template <typename T>
SharedPtr<T>::SharedPtr(SharedPtr&& other) noexcept : block_(other.block_), object_(other.object_) {
    other.block_ = nullptr;
    other.object_ = nullptr;
}

template <typename T>
SharedPtr<T>::SharedPtr(BaseControlBlock* block, T* ptr) : block_(block), object_(ptr) {
}

template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(const SharedPtr& other) {
    if (this == &other) {
        return *this;
    }
    if (block_) {
        block_->DecStrong();
    }
    block_ = other.block_;
    object_ = other.object_;
    if (block_) {
        block_->IncStrong();
    }
    return *this;
}
template <typename T>
SharedPtr<T>& SharedPtr<T>::operator=(SharedPtr&& other) noexcept {
    if (this == &other) {
        return *this;
    }
    if (block_) {
        block_->DecStrong();
    }
    block_ = other.block_;
    object_ = other.object_;
    other.block_ = nullptr;
    other.object_ = nullptr;
    return *this;
}

template <typename T>
SharedPtr<T>::~SharedPtr() {
    if (block_) {
        block_->DecStrong();
    }
    block_ = nullptr;
    object_ = nullptr;
}

template <typename T>
void SharedPtr<T>::Reset() {
    if (block_) {
        block_->DecStrong();
    }
    block_ = nullptr;
    object_ = nullptr;
}
template <typename T>
void SharedPtr<T>::Reset(T* ptr) {
    if (block_) {
        block_->DecStrong();
    }
    block_ = new PointerControlBlock<T>(ptr);
    object_ = ptr;
}
template <typename T>
void SharedPtr<T>::Swap(SharedPtr& other) {
    std::swap(block_, other.block_);
    std::swap(object_, other.object_);
}

template <typename T>
T* SharedPtr<T>::Get() const {
    return object_;
}
template <typename T>
T& SharedPtr<T>::operator*() const {
    return *object_;
}
template <typename T>
T* SharedPtr<T>::operator->() const {
    return object_;
}
template <typename T>
size_t SharedPtr<T>::UseCount() const {
    if (block_ == nullptr) {
        return 0;
    }
    return block_->GetStrongCnt();
}
template <typename T>
SharedPtr<T>::operator bool() const {
    return (block_ != nullptr);
}

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return (left.Get() == right.Get());
}

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    ObjectControlBlock<T>* tmp = new ObjectControlBlock<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(tmp, tmp->GetObject());
}

template <typename T>
SharedPtr<T>::SharedPtr(const WeakPtr<T>& other) {
    if (other.Expired()) {
        throw BadWeakPtr();
    }
    block_ = other.block_;
    object_ = other.object_;
    if (block_) {
        block_->IncStrong();
    }
}

// Look for usage examples in tests
// template <typename T>
// class EnableSharedFromThis {
// public:
//     SharedPtr<T> SharedFromThis();
//     SharedPtr<const T> SharedFromThis() const;

//     WeakPtr<T> WeakFromThis() noexcept;
//     WeakPtr<const T> WeakFromThis() const noexcept;
// };
