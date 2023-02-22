#pragma once

#include "special_symbols.h"
#include "priority_queue.h"
#include "archivator_funcs.h"
#include "fstream"
#include "mystream.h"
#include "trie.h"
#include <memory>
#include <string>
#include <map>
// NOLINTBEGIN
class Coder {
public:
    Coder(std::vector<std::string> input_files, std::string output);

    void Archive();

    void AddFile(const std::string& str);

private:
    std::vector<std::string> input_files_;
    std::string output_file_;
};

// NOLINTEND