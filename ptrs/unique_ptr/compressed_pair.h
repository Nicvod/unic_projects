#pragma once

#include <iostream>
#include <type_traits>
#include <memory>
#include <utility>
#include <mutex>

// Me think, why waste time write lot code, when few code do trick.
template <typename F, typename S,
          bool = std::is_empty_v<F> && !std::is_final_v<F> && !std::is_base_of_v<S, F>,
          bool = std::is_empty_v<S> && !std::is_final_v<S> && !std::is_base_of_v<F, S>>
class CompressedPair {};

template <typename F, typename S>
class CompressedPair<F, S, false, false> {
public:
    CompressedPair() : first_(F()), second_(S()) {
    }
    template <typename FF, typename SS>
    CompressedPair(FF&& first, SS&& second)
        : first_(std::forward<FF>(first)), second_(std::forward<SS>(second)) {
    }

    F& GetFirst() {
        return first_;
    }
    const F& GetFirst() const {
        return first_;
    }

    S& GetSecond() {
        return second_;
    }
    const S& GetSecond() const {
        return second_;
    }

    bool IsFirstNotEmpty() {
        return true;
    }
    bool IsSecondNotEmpty() {
        return true;
    }
    void Swap(CompressedPair& other) {
        std::swap(this->first_, other.first_);
        std::swap(this->second_, other.second_);
    }

private:
    F first_;
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, false, true> : S {
public:
    CompressedPair() : first_(F()) {
    }
    template <typename FF, typename SS>
    CompressedPair(FF&& first, SS&& second) : first_(std::forward<FF>(first)) {  // NOLINT
    }

    F& GetFirst() {
        return first_;
    }
    const F& GetFirst() const {
        return first_;
    }

    S GetSecond() {
        return S();
    }
    const S GetSecond() const {
        return S();
    }
    bool IsFirstNotEmpty() {
        return true;
    }
    bool IsSecondNotEmpty() {
        return false;
    }
    void Swap(CompressedPair& other) {
        std::swap(this->first_, other.first_);
    }

private:
    F first_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, false> : F {
public:
    CompressedPair() : second_(S()) {
    }
    template <typename FF, typename SS>
    CompressedPair(FF&& first, SS&& second) : second_(std::forward<SS>(second)) {  // NOLINT
    }

    F GetFirst() {
        return F();
    }
    const F GetFirst() const {
        return F();
    }

    S& GetSecond() {
        return second_;
    }
    const S& GetSecond() const {
        return second_;
    }

    bool IsFirstNotEmpty() {
        return false;
    }
    bool IsSecondNotEmpty() {
        return true;
    }

    void Swap(CompressedPair& other) {
        std::swap(this->second_, other.second_);
    }

private:
    S second_;
};

template <typename F, typename S>
class CompressedPair<F, S, true, true> : F, S {
public:
    CompressedPair() {
    }
    template <typename FF, typename SS>
    CompressedPair(FF&& first, SS&& second) {  // NOLINT
    }

    F GetFirst() {
        return F();
    }
    const F GetFirst() const {
        return F();
    }

    S GetSecond() {
        return S();
    }
    const S GetSecond() const {
        return S();
    }
    bool IsFirstNotEmpty() {
        return false;
    }
    bool IsSecondNotEmpty() {
        return false;
    }
    void Swap([[maybe_unused]] CompressedPair& other) {
    }
};