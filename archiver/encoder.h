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
class Encoder {
public:
    explicit Encoder(std::string input);

    void UnArchive();

private:
    std::string input_file_;
};
// NOLINTEND