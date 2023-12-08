#pragma once

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t
#include <type_traits>
#include <utility>

template <typename T>
struct Slug {
    Slug() = default;
    template <typename U>
    Slug([[maybe_unused]] const Slug<U>& other) requires(std::is_base_of_v<T, U>) {
    }
    template <typename U>
    Slug([[maybe_unused]] Slug<U>&& other) requires(std::is_base_of_v<T, U>) {
    }

    template <typename U>
    Slug& operator=([[maybe_unused]] const Slug& other) requires(std::is_base_of_v<T, U>) {
    }
    template <typename U>
    Slug& operator=([[maybe_unused]] Slug&& other) requires(std::is_base_of_v<T, U>) {
    }

    void operator()(T* ref) {
        delete ref;
    }

    ~Slug() = default;
};

template <typename T>
struct Slug<T[]> {
    Slug() = default;
    template <typename U>
    Slug([[maybe_unused]] const Slug<U>& other) requires(std::is_base_of_v<T, U>) {
    }
    template <typename U>
    Slug([[maybe_unused]] Slug<U>&& other) requires(std::is_base_of_v<T, U>) {
    }

    template <typename U>
    Slug& operator=([[maybe_unused]] const Slug& other) requires(std::is_base_of_v<T, U>) {
    }
    template <typename U>
    Slug& operator=([[maybe_unused]] Slug&& other) requires(std::is_base_of_v<T, U>) {
    }

    void operator()(T* ref) {
        delete[] ref;
    }

    ~Slug() = default;
};

// Primary template
template <typename T, typename Deleter = Slug<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }
    template <typename D2>
    UniquePtr(T* ptr, D2 deleter) : data_(ptr, std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        if (other.data_.IsSecondNotEmpty()) {
            data_.GetSecond() = std::move(other.data_.GetSecond());
        }
    }

    template <typename T2, typename D2>
    requires(std::is_base_of_v<T, T2>) UniquePtr(UniquePtr<T2, D2>&& other)
    noexcept {
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        if (other.data_.IsSecondNotEmpty()) {
            data_.GetSecond() = std::move(other.data_.GetSecond());
        }
    }

    UniquePtr(UniquePtr& other) = delete;
    UniquePtr(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == reinterpret_cast<UniquePtr<T, Deleter>*>(&other)) {
            return *this;
        }
        this->~UniquePtr();
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        if (other.data_.IsSecondNotEmpty()) {
            this->data_.GetSecond() = std::move(other.data_.GetSecond());
        }
        return *this;
    }

    template <typename T2, typename D2>
    requires(std::is_base_of_v<T, T2>) UniquePtr& operator=(UniquePtr<T2, D2>&& other) noexcept {
        if (this == reinterpret_cast<UniquePtr<T, Deleter>*>(&other)) {
            return *this;
        }
        this->~UniquePtr();
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        if (other.data_.IsSecondNotEmpty()) {
            this->data_.GetSecond() = std::move(other.data_.GetSecond());
        }
        return *this;
    }
    UniquePtr& operator=(T* ptr) {
        this->~UniquePtr();
        data_.GetFirst() = ptr;
        return *this;
    }

    UniquePtr& operator=(UniquePtr& other) = delete;
    const UniquePtr& operator=(const UniquePtr& other) = delete;

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        data_.GetSecond()(data_.GetFirst());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        T* old_ptr = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return old_ptr;
    }
    void Reset(T* ptr = nullptr) {
        T* tmp = data_.GetFirst();
        if (tmp != nullptr) {
            data_.GetFirst() = ptr;
            data_.GetSecond()(tmp);
        }
    }
    void Swap(UniquePtr& other) {
        data_.Swap(other.data_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }
    explicit operator bool() const {
        return data_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    template <typename U = T>
    U& operator*() const requires(!std::is_void_v<U> && std::is_same_v<T, U>) {
        return *(data_.GetFirst());
    }
    T* operator->() const {
        return data_.GetFirst();
    }

private:
    template <typename T2, typename D2>
    friend class UniquePtr;

    CompressedPair<T*, Deleter> data_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    explicit UniquePtr(T* ptr = nullptr) : data_(ptr, Deleter()) {
    }
    template <typename D2>
    UniquePtr(T* ptr, D2 deleter) : data_(ptr, std::forward<Deleter>(deleter)) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        if (other.data_.IsSecondNotEmpty()) {
            data_.GetSecond() = std::move(other.data_.GetSecond());
        }
    }

    template <typename T2, typename D2>
    requires(std::is_base_of_v<T, T2>) UniquePtr(UniquePtr<T2, D2>&& other)
    noexcept {
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        if (other.data_.IsSecondNotEmpty()) {
            data_.GetSecond() = std::move(other.data_.GetSecond());
        }
    }

    UniquePtr(UniquePtr& other) = delete;
    UniquePtr(const UniquePtr& other) = delete;

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == reinterpret_cast<UniquePtr<T, Deleter>*>(&other)) {
            return *this;
        }
        this->~UniquePtr();
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        if (other.data_.IsSecondNotEmpty()) {
            this->data_.GetSecond() = std::move(other.data_.GetSecond());
        }
        return *this;
    }

    template <typename T2, typename D2>
    requires(std::is_base_of_v<T, T2>) UniquePtr& operator=(UniquePtr<T2, D2>&& other) noexcept {
        if (this == reinterpret_cast<UniquePtr<T[], Deleter>*>(&other)) {
            return *this;
        }
        this->~UniquePtr();
        data_.GetFirst() = other.data_.GetFirst();
        other.data_.GetFirst() = nullptr;
        if (other.data_.IsSecondNotEmpty()) {
            this->data_.GetSecond() = std::move(other.data_.GetSecond());
        }
        return *this;
    }
    UniquePtr& operator=(T* ptr) {
        this->~UniquePtr();
        data_.GetFirst() = ptr;
        return *this;
    }

    UniquePtr& operator=(UniquePtr& other) = delete;
    const UniquePtr& operator=(const UniquePtr& other) = delete;

    ~UniquePtr() {
        data_.GetSecond()(data_.GetFirst());
    }

    T* Release() {
        T* old_ptr = data_.GetFirst();
        data_.GetFirst() = nullptr;
        return old_ptr;
    }
    void Reset(T* ptr = nullptr) {
        T* tmp = data_.GetFirst();
        if (tmp != nullptr) {
            data_.GetFirst() = ptr;
            data_.GetSecond()(tmp);
        }
    }
    void Swap(UniquePtr& other) {
        data_.Swap(other.data_);
    }

    T* Get() const {
        return data_.GetFirst();
    }
    Deleter& GetDeleter() {
        return data_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return data_.GetSecond();
    }

    explicit operator bool() const {
        return data_.GetFirst() != nullptr;
    }

    T& operator[](size_t ind) {
        return (data_.GetFirst()[ind]);
    }

    const T& operator[](size_t ind) const {
        return (data_.GetFirst()[ind]);
    }

    template <typename U = T>
    U& operator*() const requires(!std::is_void_v<U> && std::is_same_v<T, U>) {
        return *(data_.GetFirst());
    }
    T* operator->() const {
        return data_.GetFirst();
    }

private:
    template <typename T2, typename D2>
    friend class UniquePtr;

    CompressedPair<T*, Deleter> data_;
};
