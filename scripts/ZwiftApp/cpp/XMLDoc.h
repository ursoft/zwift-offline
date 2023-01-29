#pragma once

class XMLDoc { //1736 bytes
    tinyxml2::XMLDocument m_tiny_doc;
    std::map<unsigned, tinyxml2::XMLElement *> m_map;
public:
    XMLDoc();
    ~XMLDoc();
    void Save();
    tinyxml2::XMLElement *FindElement(const char *path, bool enableCreate);
    tinyxml2::XMLElement *FindFirstElement(tinyxml2::XMLNode *, int, const std::vector<std::string> &path);
    tinyxml2::XMLElement *FindNextElement(const char *path, bool b1, bool enableCreate);
    tinyxml2::XMLElement *FindNextMatchingElement(tinyxml2::XMLNode *from, int pidx, const std::vector<std::string> &path, bool goDown);
    tinyxml2::XMLElement *CreateElement(tinyxml2::XMLNode *, const std::vector<std::string> &path);
    void SetCStr(const char *path, const char *text, bool enableCreate);
};

extern XMLDoc g_UserConfigDoc;