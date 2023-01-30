#include "ZwiftApp.h"

namespace ZStringUtil {
    std::vector<std::string> Split(const std::string &src, char delim) {
        std::stringstream ss(src);
        std::vector<std::string> ret;
        while (ss.good()) {
            std::string substr;
            std::getline(ss, substr, delim);
            ret.push_back(substr);
        }
        return ret;
    }
}