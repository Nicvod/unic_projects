#include "coder.h"
// NOLINTBEGIN
Coder::Coder(std::vector<std::string> input_files, std::string output)
    : input_files_(input_files), output_file_(output) {
}

void CountSymbolsInFile(const std::string& input_file, std::map<size_t, uint64_t>& symbols_count) {
    std::ifstream input(input_file, std::ios_base::binary);
    Reader reader(input);
    bool file_ended = false;
    int cur_bite = 0;
    size_t cur_symbol = 0;
    while (true) {
        cur_symbol = 0;
        for (int i = 0; i < 8; ++i) {
            cur_bite = reader.GetBite();
            if (cur_bite < 0) {
                file_ended = true;
                break;
            }
            cur_symbol = (cur_symbol << 1) | cur_bite;
        }
        if (file_ended) {
            break;
        }
        ++symbols_count[cur_symbol];
    }
    ++symbols_count[ARCHIVE_END];  // hard code :(
    ++symbols_count[ONE_MORE_FILE];
    ++symbols_count[FILENAME_END];
    std::string file_name = FileFromPath(input_file);
    for (unsigned char symbol : file_name) {
        ++symbols_count[static_cast<size_t>(symbol)];
    }
}

void CreateHuffQueue(const std::map<size_t, uint64_t>& symbols_count,
                     QueuePriority<std::pair<size_t, std::shared_ptr<Trie<size_t>>>>& huff_queue) {
    for (const auto& [symbol, cnt] : symbols_count) {
        huff_queue.push({cnt, std::make_shared<Trie<size_t>>(symbol)});
    }
    while (huff_queue.size() != 1) {
        auto tmp1 = huff_queue.top();
        huff_queue.pop();
        auto tmp2 = huff_queue.top();
        huff_queue.pop();
        huff_queue.push({tmp1.first + tmp2.first, std::make_shared<Trie<size_t>>(tmp1.second, tmp2.second)});
    }
}

template <typename T>
void WriteNineBitsInInt(Writer<T>& writer, size_t num) {
    for (int i = 8; i >= 0; --i) {
        writer.PutBite(1 & (num >> i));
    }
}

template <typename T>
void WriteFile(Writer<T>& writer, const std::string& input_file,
               const std::map<size_t, std::vector<bool>>& symbols_codes,
               const std::vector<std::pair<size_t, size_t>>& symbols_length) {
    WriteNineBitsInInt(writer, symbols_codes.size());
    std::vector<size_t> cnt_symbols_with_length(symbols_length.size() + 1, 0);
    size_t max_length = 0;
    for (const auto& [length, symbol] : symbols_length) {
        WriteNineBitsInInt(writer, symbol);
        cnt_symbols_with_length[length]++;
        if (max_length < length) {
            max_length = length;
        }
    }

    for (size_t i = 1; i <= max_length; ++i) {
        WriteNineBitsInInt(writer, cnt_symbols_with_length[i]);
    }

    std::string file_name = FileFromPath(input_file);
    for (unsigned char x : file_name) {
        writer.PutNBites(symbols_codes.at(x));
    }

    writer.PutNBites(symbols_codes.at(FILENAME_END));

    std::ifstream input(input_file, std::ios_base::binary);
    Reader reader(input);
    bool file_ended = false;
    int cur_bite = 0;
    size_t cur_symbol = 0;
    while (true) {
        cur_symbol = 0;
        for (int i = 0; i < 8; ++i) {
            cur_bite = reader.GetBite();
            if (cur_bite < 0) {
                file_ended = true;
                break;
            }
            cur_symbol = (cur_symbol << 1) | cur_bite;
        }
        if (file_ended) {
            break;
        }
        writer.PutNBites(symbols_codes.at(cur_symbol));
    }
}

void Coder::Archive() {
    std::ofstream output(output_file_, std::ios_base::binary);
    Writer writer(output);

    for (size_t i = 0; i < input_files_.size(); ++i) {
        const std::string& cur_file = input_files_[i];
        std::map<size_t, uint64_t> symbols_count;
        CountSymbolsInFile(cur_file, symbols_count);

        QueuePriority<std::pair<size_t, std::shared_ptr<Trie<size_t>>>> huff_queue;
        CreateHuffQueue(symbols_count, huff_queue);

        std::vector<std::pair<size_t, size_t>> symbols_length;
        GetTerminalLens(huff_queue.top().second, symbols_length);

        sort(symbols_length.begin(), symbols_length.end());
        std::map<size_t, std::vector<bool>> symbols_codes;
        CanonicalHuffmanCoder(symbols_length, symbols_codes);

        WriteFile(writer, cur_file, symbols_codes, symbols_length);
        if (i + 1 != input_files_.size()) {
            writer.PutNBites(symbols_codes.at(ONE_MORE_FILE));
        } else {
            writer.PutNBites(symbols_codes.at(ARCHIVE_END));
        }
    }
}

void Coder::AddFile(const std::string& str) {
    input_files_.push_back(str);
}
// NOLINTEND