#include "ZStringUtil.h" //READY for testing
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
    void SplitFilename(const std::string &src, std::string *p1, std::string *p2) {
        for (auto it = src.rbegin(); it != src.rend(); it++) {
            if (*it == '/' || *it == '\\') {
                p1->assign(src.c_str(), &(*it) - src.c_str() + 1);
                p2->assign(src.begin() + p1->length(), src.end());
                return;
            }
        }
        p1->clear();
        *p2 = src;
    }
}

//Unit tests
TEST(SmokeTest, Split) {
    std::string src = "c:\\file";
    auto s = ZStringUtil::Split(src, '\\');
    EXPECT_EQ(2ull, s.size());
    EXPECT_STREQ("c:", s[0].c_str());
    EXPECT_STREQ("file", s[1].c_str());
}
TEST(SmokeTest, SplitNeg) {
    std::string src = "c:\\file";
    auto s = ZStringUtil::Split(src, '?');
    EXPECT_EQ(1ull, s.size());
    EXPECT_EQ(src, s[0]);
}
TEST(SmokeTest, SplitFilename) {
    std::string src = "c:\\path\\file", path, name;
    ZStringUtil::SplitFilename(src, &path, &name);
    EXPECT_STREQ("c:\\path\\", path.c_str());
    EXPECT_STREQ("file", name.c_str());
}
TEST(SmokeTest, SplitFilenameNeg) {
    std::string src = "c:file", path, name;
    ZStringUtil::SplitFilename(src, &path, &name);
    EXPECT_EQ(src, name);
    EXPECT_EQ("", path);
}