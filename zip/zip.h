#pragma once

#include <iterator>

template <typename SequenceElement1, typename SequenceElement2>
class Zipped {
public:
    using LhsValue = typename std::iterator_traits<SequenceElement1>::value_type;
    using RhsValue = typename std::iterator_traits<SequenceElement2>::value_type;

    Zipped(const SequenceElement1& lhs_begin, const SequenceElement1& lhs_end, const SequenceElement2& rhs_begin, const SequenceElement2& rhs_end)
        : lhs_begin_(lhs_begin), rhs_begin_(rhs_begin), lhs_end_(lhs_end), rhs_end_(rhs_end) {
    }

    class ZipIterator {
    public:
        ZipIterator(SequenceElement1 lhs_current, SequenceElement2 rhs_current) {
            lhs_current_ = lhs_current;
            rhs_current_ = rhs_current;
        }

        ZipIterator &operator++() {
            ++lhs_current_;
            ++rhs_current_;
            return *this;
        }
        ZipIterator operator++(int) {
            Zipped::ZipIterator now(lhs_current_, rhs_current_);
            ++lhs_current_;
            ++rhs_current_;
            return now;
        }
        std::pair<LhsValue, RhsValue> operator*() {
            return {*lhs_current_, *rhs_current_};
        }
        bool operator==(const ZipIterator &other) const {
            return ((lhs_current_ == other.lhs_current_) || (rhs_current_ == other.rhs_current_));
        }
        bool operator!=(const ZipIterator &other) const {
            return ((lhs_current_ != other.lhs_current_) && (rhs_current_ != other.rhs_current_));
        }

    private:
        SequenceElement1 lhs_current_;
        SequenceElement2 rhs_current_;
    };

    ZipIterator begin() const {
        return Zipped::ZipIterator(lhs_begin_, rhs_begin_);
    }
    ZipIterator end() const {
        return Zipped::ZipIterator(lhs_end_, rhs_end_);
    }

private:
    SequenceElement1 lhs_begin_;
    SequenceElement2 rhs_begin_;
    SequenceElement1 lhs_end_;
    SequenceElement2 rhs_end_;
};

template <typename Sequence1, typename Sequence2>
auto Zip(const Sequence1 &sequence1, const Sequence2 &sequence2) {
    return Zipped(std::begin(sequence1), std::end(sequence1), std::begin(sequence2), std::end(sequence2));
}