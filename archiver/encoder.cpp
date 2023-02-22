#include "encoder.h"

// NOLINTBEGIN
Encoder::Encoder(std::string input) : input_file_(input) {
}

size_t NumFromBites(const std::vector<bool>& bites) {
    size_t ans = 0;
    for (size_t i = 0; i < bites.size(); ++i) {
        ans = (ans << 1) | bites[i];
    }
    return ans;
}

template <typename T>
void PutEightBitesSymbol(Writer<T>& writer, size_t symbol) {
    for (int i = 7; i >= 0; --i) {
        writer.PutBite(1 & (symbol >> i));
    }
}

template <typename InputType, typename T>
void Terminated(Reader<InputType>& reader, std::shared_ptr<Trie<T>>& cur_node) {
    int tmp_bite = 0;
    while (!(cur_node->IsTerminal())) {
        tmp_bite = reader.GetBite();
        if (tmp_bite == -1) {
            exit(111);
        }
        if (!tmp_bite) {
            cur_node = cur_node->GetLeft();
        } else {
            cur_node = cur_node->GetRight();
        }
    }
}

void Encoder::UnArchive() {
    std::ifstream input_file(input_file_, std::ios_base::binary);
    Reader reader(input_file);
    bool archive_end = false;
    while (!archive_end) {
        std::vector<bool> bits_sequence;
        reader.GetNBites(bits_sequence, 9);
        size_t symbols_cnt = NumFromBites(bits_sequence);

        std::vector<std::pair<size_t, size_t>> symbols_length(symbols_cnt);
        size_t cur_symbol = 0;
        for (size_t i = 0; i < symbols_cnt; ++i) {
            reader.GetNBites(bits_sequence, 9);
            cur_symbol = NumFromBites(bits_sequence);
            symbols_length[i].second = cur_symbol;
        }

        size_t cur_symbols_cnt = 0;
        size_t cur_length_cnt = 0;
        size_t cur_length = 0;
        while (cur_symbols_cnt < symbols_cnt) {
            ++cur_length;
            reader.GetNBites(bits_sequence, 9);
            cur_length_cnt = NumFromBites(bits_sequence);

            for (size_t i = 0; i < cur_length_cnt; ++i) {
                symbols_length[cur_symbols_cnt++].first = cur_length;
            }
        }
        if (symbols_length.empty()) {
            exit(111);
        }
        std::shared_ptr<Trie<size_t>> root = std::make_shared<Trie<size_t>>();
        std::vector<bool> cur_huf_sequence(symbols_length[0].first, false);
        AddSequence(cur_huf_sequence, symbols_length[0].second, root);
        for (size_t i = 1; i < symbols_length.size(); ++i) {
            PlusOne(cur_huf_sequence);
            while (cur_huf_sequence.size() < symbols_length[i].first) {
                cur_huf_sequence.push_back(false);
            }
            AddSequence(cur_huf_sequence, symbols_length[i].second, root);
        }

        std::string output_file_name;
        bool file_end = false;
        std::shared_ptr<Trie<size_t>> cur_node;
        while (!file_end) {
            cur_node = root;
            Terminated(reader, cur_node);
            if ((cur_node->GetElement()) == FILENAME_END) {
                file_end = true;
            } else {
                output_file_name.push_back(static_cast<char>(cur_node->GetElement()));
            }
        }

        std::ofstream output(output_file_name, std::ios_base::binary);
        Writer writer(output);
        file_end = false;
        while (!file_end) {
            cur_node = root;
            Terminated(reader, cur_node);
            if ((cur_node->GetElement()) == ONE_MORE_FILE) {
                file_end = true;
            } else if ((cur_node->GetElement()) == ARCHIVE_END) {
                file_end = true;
                archive_end = true;
            } else {
                PutEightBitesSymbol(writer, (cur_node->GetElement()));
            }
        }
    }
}
// NOLINTEND