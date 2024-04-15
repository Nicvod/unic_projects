#include "archivator.h"
// NOLINTBEGIN
int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "too few arguments";
        return 1;
    }
    // I'll try change parsing
    if (argv[1][1] == 'h') {
        std::cout << "Типо архиватор(нет    :(    )";
    } else if (argv[1][1] == 'd') {
        if (argc < 3) {
            std::cerr << "too few arguments";
            return 1;
        }
        Encoder encoder(argv[2]);
        encoder.UnArchive();
    } else (argv[1][1] == 'c') {
        if (argc < 3) {
            std::cerr << "too few arguments";
            return 1;
        }
        std::vector<std::string> input_files;
        for (int i = 3; i < argc; ++i) {
            input_files.push_back(argv[i]);
        }
        Coder coder(input_files, argv[2]);
        coder.Archive();
    } else {
        std::cerr << "unknown option: " << argv[2] << std::endl;
        return 1;
    }
    return 0;
}
// NOLINTEND
