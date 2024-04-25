#pragma once

#include<initializer_list>
#include<vector>
#include<functional>
#include<list>
#include <stdexcept>

template<class KeyType, class ValueType, class Hash = std::hash<KeyType> >
class HashMap {
public:
    using IteratorBadType = std::pair<const KeyType, ValueType>;

    explicit HashMap(Hash func = Hash()) : hash_func_(func), begin_(nullptr), end_(nullptr), size_(0) {  // Hash = std::hash<KeyType>
        data_.assign(2, HashElement());
        not_empty_.resize(0);
    }

    template<class T>
    HashMap(T begin, T end, Hash func = Hash())  : hash_func_(func), begin_(nullptr), end_(nullptr), size_(0) {
        data_.assign(2, HashElement());
        not_empty_.resize(0);
        while (begin != end) {
            insert(*begin);
            ++begin;
        }
    }
    HashMap(std::initializer_list<std::pair<KeyType, ValueType>> ini_list, Hash func = Hash()) : hash_func_(func), begin_(nullptr), end_(nullptr), size_(0) {
        data_.assign(2 * ini_list.size() + 1, HashElement());
        not_empty_.resize(0);
        for (auto x : ini_list) {
            insert(x);
        }
    }

    struct HashElement {
        std::pair<KeyType, ValueType> element_;
        size_t dist_;
        bool del_;
        bool empty_;
        HashElement* nxt_;
        HashElement* prv_;

        HashElement() {
            dist_ = 0;
            del_ = true;
            empty_ = true;
            nxt_ = nullptr;
            prv_ = nullptr;
        }
    };

    class iterator {
    public:
        iterator() : current_(nullptr) {};
        explicit iterator(HashElement* iter) : current_(iter) {}
        iterator(iterator& iter) : current_(iter.current_) {}

        IteratorBadType& operator*() {
            return (*operator->());
        }
        IteratorBadType* operator->() const {
            return (IteratorBadType *) (&(current_->element_));
        }

        iterator& operator=(iterator other) {
            current_ = other.current_;
            return *this;
        }

        iterator& operator++() {
            current_ = (*current_).nxt_;
            return *this;
        }
        iterator operator++(int) {
            iterator now(*this);
            current_ = (*current_).nxt_;
            return now;
        }

        bool operator==(const iterator& other) const {
            return (current_ == other.current_);
        }
        bool operator!=(const iterator& other) const {
            return (current_ != other.current_);
        }
    private:
        HashElement* current_;
    };

    class const_iterator {
    public:
        const_iterator() : current_(nullptr) {};
        explicit const_iterator(HashElement* iter) : current_(iter){}
        explicit const_iterator(const HashElement* iter) : current_(iter){}
        explicit const_iterator(iterator& iter) : current_(iter.current_){}
        const_iterator(const_iterator& iter) : current_(iter.current_){}

        const IteratorBadType& operator*() const {
            return (*operator->());
        }
        IteratorBadType* operator->() const {
            return (IteratorBadType *const) (&(current_->element_));
        }

        const_iterator& operator=(iterator other) {
            current_ = other.current_;
            return *this;
        }

        const_iterator& operator++() {
            current_ = (*current_).nxt_;
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator now(*this);
            current_ = (*current_).nxt_;
            return now;
        }

        bool operator==(const const_iterator& other) const {
            return (current_ == other.current_);
        }
        bool operator!=(const const_iterator& other) const {
            return (current_ != other.current_);
        }
    private:
        const HashElement* current_;
    };

    size_t size() const{
        return size_;
    }
    bool empty() const {
        return (size_ == 0);
    }

    Hash hash_function() const {
        return hash_func_;
    }

    void resize() {
        std::vector<std::pair<KeyType, ValueType> > tmp;
        for (auto x : not_empty_) {
            if (!data_[x].del_)
                tmp.push_back(data_[x].element_);
        }
        clear();
        data_.assign(data_.size() * 2, HashElement());
        for (auto x : tmp) {
            insert(x);
        }
    }

    void rehash() {
        std::vector<std::pair<KeyType, ValueType> > tmp;
        for (auto x : not_empty_) {
            if (!data_[x].del_)
                tmp.push_back(data_[x].element_);
        }
        clear();
        for (auto x : tmp) {
            insert(x);
        }
    }

    void pop(size_t hash) {
        auto prv = data_[hash].prv_;
        auto nxt = data_[hash].nxt_;
        if (prv != nullptr) {
            (*prv).nxt_ = nxt;
        }
        else {
            begin_ = nxt;
        }
        if (nxt != nullptr) {
            (*nxt).prv_ = prv;
        }
        else {
            end_ = prv;
        }
        data_[hash].prv_ = nullptr;
        data_[hash].nxt_ = nullptr;
    }

    void push(size_t hash) {
        if (end_ == nullptr) {
            begin_ = &(data_[hash]);
        }
        else {
            (*end_).nxt_ = &(data_[hash]);
        }
        data_[hash].prv_ = end_;
        data_[hash].nxt_ = nullptr;
        end_ = &(data_[hash]);
    }

    void insert(std::pair<KeyType, ValueType> new_element) {
        bool iter = find_del(new_element.first);
        if (iter) {
            size_t hash = hash_func_(new_element.first);
            if (hash >= data_.size()) hash %= data_.size();
            while ((data_[hash].element_.first == new_element.first) == false) {
                ++hash;
                if (hash >= data_.size()) {
                    hash -= data_.size();
                }
            }
            if (data_[hash].del_ == true) {
                data_[hash].element_.second = new_element.second;
                data_[hash].del_ = false;
                push(hash);
                ++size_;
            }
            return;
        }
        ++size_;
        size_t hash = hash_func_(new_element.first);
        if (hash >= data_.size()) hash %= data_.size();
        size_t dist = 0;
        bool del = false;
        while (!data_[hash].empty_) {
            if (dist > data_[hash].dist_) {
                std::swap(dist, data_[hash].dist_);
                std::swap(new_element, data_[hash].element_);
                std::swap(del, data_[hash].del_);
                if (del != data_[hash].del_) {
                    if (!del) {
                        pop(hash);
                    }
                    else {
                        push(hash);
                    }
                }
            }
            ++hash;
            if (hash >= data_.size()) {
                hash -= data_.size();
            }
            ++dist;
        }
        data_[hash].del_ = del;
        data_[hash].element_ = new_element;
        data_[hash].dist_ = dist;
        data_[hash].empty_ = false;
        if (!del) push(hash);
        not_empty_.push_back(hash);
        if (not_empty_.size()*10 >= data_.size()*9) {
            resize();
        }
    }
    void erase(KeyType key) {
        size_t hash = hash_func_(key);
        if (hash >= data_.size()) hash %= data_.size();
        while (!data_[hash].empty_) {
            if (key == data_[hash].element_.first) {
                if (!data_[hash].del_) {
                    pop(hash);
                    data_[hash].del_ = true;
                    data_[hash].nxt_ = nullptr;
                    data_[hash].prv_ = nullptr;
                    --size_;
                }
                break;
            }
            ++hash;
            if (hash >= data_.size()) {
                hash -= data_.size();
            }
        }

        if (not_empty_.size()/4 > size_) {
            rehash();
        }
    }

    iterator find(KeyType key) {
        size_t hash = hash_func_(key);
        hash %= data_.size();
        while (!data_[hash].empty_) {
            if (data_[hash].element_.first == key) {
                if (data_[hash].del_ == false) {
                    return iterator(&data_[hash]);
                }
                else {
                    return end();
                }
            }
            ++hash;
            if (hash >= data_.size()) {
                hash -= data_.size();
            }
        }
        return end();
    }
    const_iterator find(KeyType key) const {
        size_t hash = hash_func_(key);
        hash %= data_.size();
        while (!data_[hash].empty_) {
            if (data_[hash].element_.first == key) {
                if (data_[hash].del_ == false) {
                    return const_iterator(&(data_[hash]));
                }
                else {
                    return end();
                }
            }
            ++hash;
            if (hash >= data_.size()) {
                hash -= data_.size();
            }
        }
        return end();
    }

    bool find_del(KeyType key) {
        size_t hash = hash_func_(key);
        hash %= data_.size();
        while (!data_[hash].empty_) {
            if (data_[hash].element_.first == key) {
                return true;
            }
            ++hash;
            if (hash >= data_.size()) {
                hash -= data_.size();
            }
        }
        return false;
    }

    ValueType& operator[](KeyType key) {
        iterator iter = find(key);
        if (iter == end()) {
            insert({key, ValueType()});
            iter = find(key);
        }
        return (iter->second);
    }
    const ValueType& at(KeyType key) const {
        const_iterator iter = find(key);
        if (iter == end()) {
            throw std::out_of_range("Don't find your key :(");
        }
        return const_cast<const ValueType&> (iter->second);
    }

    void clear() {
        for (auto ind : not_empty_) {
            data_[ind].empty_ = true;
            data_[ind].del_ = true;
            data_[ind].dist_ = 0;
            data_[ind].nxt_ = nullptr;
            data_[ind].prv_ = nullptr;
        }
        not_empty_.clear();
        begin_ = nullptr;
        end_ = nullptr;
        size_ = 0;
    }



    iterator begin() {
        return iterator(begin_);
    }
    iterator end() {
        return iterator();
    }
    const_iterator begin() const {
        return const_iterator(begin_);
    }
    const_iterator end() const {
        return const_iterator();
    }
private:
    std::vector<HashElement> data_;
    std::vector<size_t> not_empty_; // all not empty indexes
    Hash hash_func_;
    HashElement* begin_; // first undead element
    HashElement* end_; // last undead element
    size_t size_;
};
