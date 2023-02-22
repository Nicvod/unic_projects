#pragma once

#include <string>
#include <vector>
#include <map>
// NOLINTBEGIN
void PlusOne(std::vector<bool>& bites);

template <typename T>
void CanonicalHuffmanCoder(const std::vector<std::pair<size_t, T>>& symbols_lengths,
                           std::map<T, std::vector<bool>>& ans) {
    if (symbols_lengths.empty()) {
        exit(111);
    }
    std::vector<bool> cur_sequence(symbols_lengths[0].first, false);
    ans[symbols_lengths[0].second] = cur_sequence;
    for (size_t i = 1; i < symbols_lengths.size(); ++i) {
        PlusOne(cur_sequence);
        while (cur_sequence.size() < symbols_lengths[i].first) {
            cur_sequence.push_back(false);
        }
        ans[symbols_lengths[i].second] = cur_sequence;
    }
}

std::string FileFromPath(const std::string& path);
// NOLINTEND
