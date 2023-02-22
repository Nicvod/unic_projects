#include "archivator_funcs.h"

void PlusOne(std::vector<bool>& bites) {
    if (bites.empty()) {
        exit(111);
    }
    size_t cur_ind = bites.size();
    while (cur_ind > 0) {
        if (!bites[--cur_ind]) {
            break;
        }
    }
    if (bites[cur_ind]) {
        cur_ind = bites.size();
        bites.push_back(true);
        while (cur_ind > 0) {
            swap(bites[cur_ind - 1], bites[cur_ind]);
            --cur_ind;
        }
    } else {
        bites[cur_ind] = true;
        ++cur_ind;
        while (cur_ind < bites.size()) {
            bites[cur_ind++] = false;
        }
    }
}

std::string FileFromPath(const std::string& path) {
    int64_t last_slash = -1;
    for (size_t i = 0; i < path.size(); ++i) {
        if (path[i] == '/' || path[i] == '\\') {
            last_slash = static_cast<int64_t>(i);
        }
    }
    return path.substr(last_slash + 1);
}