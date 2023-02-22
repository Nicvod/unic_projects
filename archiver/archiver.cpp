#include "archivator.h"
// NOLINTBEGIN
int main(int argc, char** argv) {

    // I'll try change parsing
    if (argv[1][1] == 'h') {
        std::cout << "Типо архиватор(нет    :(    )";
    } else if (argv[1][1] == 'd') {
        Encoder encoder(argv[2]);
        encoder.UnArchive();
    } else {
        std::vector<std::string> input_files;
        for (int i = 3; i < argc; ++i) {
            input_files.push_back(argv[i]);
        }
        Coder coder(input_files, argv[2]);
        coder.Archive();
    }
    return 0;
}
// NOLINTEND