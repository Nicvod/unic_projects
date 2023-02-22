#pragma once

#include <memory>
#include <algorithm>
#include <map>
// NOLINTBEGIN
template <typename T>
class Trie {
public:
    Trie();

    explicit Trie(T element);

    Trie(std::shared_ptr<Trie> &lhs, std::shared_ptr<Trie> &rhs);

    bool operator<(const Trie other) const;

    std::shared_ptr<Trie> GetLeft() const;

    std::shared_ptr<Trie> GetRight() const;

    T GetElement() const;

    bool IsTerminal() const;

private:
    bool terminal_vertex_;
    std::shared_ptr<Trie> left_child_;
    std::shared_ptr<Trie> right_child_;
    T element_;

    friend std::ostream &operator<<(std::ostream &stream, std::shared_ptr<Trie> ptr) {
        return stream << (ptr->element_);
    }

    friend void GetTerminalLens(
        std::shared_ptr<Trie> root,
        std::vector<std::pair<size_t, T>>
            &ans) {  // returns vector of pairs {deep_of_terminal_node, shared_ptr{terminal_node}}
        std::vector<std::pair<std::shared_ptr<Trie>, size_t>> stack;  // dfs stack
        stack.push_back({root, 0});
        std::shared_ptr<Trie> cur_node;
        size_t cur_deep = 0;
        while (!stack.empty()) {  // dfs
            cur_node = stack.back().first;
            cur_deep = stack.back().second;
            stack.pop_back();
            if (cur_node->terminal_vertex_) {
                ans.push_back({cur_deep, cur_node->element_});
                continue;
            }
            stack.push_back({cur_node->left_child_, cur_deep + 1});
            stack.push_back({cur_node->right_child_, cur_deep + 1});
        }
    }

    friend bool operator<(const std::pair<size_t, std::shared_ptr<Trie<T>>> &lhs,
                          const std::pair<size_t, std::shared_ptr<Trie<T>>> &rhs) {
        return (std::make_pair(lhs.first, lhs.second->element_) < std::make_pair(rhs.first, rhs.second->element_));
    }

    friend void AddSequence(const std::vector<bool> &ar, T symbol, std::shared_ptr<Trie<T>> cur_node) {
        for (size_t i = 0; i < ar.size(); ++i) {
            if (!ar[i]) {
                if (cur_node->left_child_ == nullptr) {
                    cur_node->left_child_ = std::make_shared<Trie<T>>();
                }
                cur_node = cur_node->left_child_;
                if (i + 1 == ar.size()) {
                    cur_node->element_ = symbol;
                    cur_node->terminal_vertex_ = true;
                } else {
                    cur_node->terminal_vertex_ = false;
                }
            } else {
                if (cur_node->right_child_ == nullptr) {
                    cur_node->right_child_ = std::make_shared<Trie<T>>();
                }
                cur_node = cur_node->right_child_;
                if (i + 1 == ar.size()) {
                    cur_node->element_ = symbol;
                    cur_node->terminal_vertex_ = true;
                } else {
                    cur_node->terminal_vertex_ = false;
                }
            }
        }
    }
};

template <typename T>
Trie<T>::Trie() : terminal_vertex_(false), left_child_(nullptr), right_child_(nullptr), element_(0) {
}

template <typename T>
Trie<T>::Trie(T element) : terminal_vertex_(true), left_child_(nullptr), right_child_(nullptr), element_(element) {
}

template <typename T>
Trie<T>::Trie(std::shared_ptr<Trie<T>> &lhs, std::shared_ptr<Trie<T>> &rhs)
    : terminal_vertex_(false), left_child_(lhs), right_child_(rhs), element_(std::min(lhs->element_, rhs->element_)) {
}

template <typename T>
bool Trie<T>::operator<(const Trie<T> other) const {
    return element_ < other.element_;
}

template <typename T>
bool Trie<T>::IsTerminal() const {
    return terminal_vertex_;
}

template <typename T>
std::shared_ptr<Trie<T>> Trie<T>::GetLeft() const {
    return left_child_;
}

template <typename T>
std::shared_ptr<Trie<T>> Trie<T>::GetRight() const {
    return right_child_;
}

template <typename T>
T Trie<T>::GetElement() const {
    return element_;
}
// NOLINTEND