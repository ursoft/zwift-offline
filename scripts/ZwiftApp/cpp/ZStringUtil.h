#pragma once //READY for testing
namespace ZStringUtil {
    std::vector<std::string> Split(const std::string &src, char delim);
    // NOT used or inlined: ToLower(std::string const &), StripPaddedSpaces(std::string const &), PrependFolder(std::string &, std::string), RemoveSubstring(std::string &, char, char)
    void SplitFilename(const std::string &src, std::string *p1, std::string *p2);
}