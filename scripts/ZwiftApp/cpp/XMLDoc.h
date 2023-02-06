#pragma once
struct VEC4 {
    GLfloat d[4];
};
struct VEC3 {
    GLfloat d[3];
};
struct VEC2 {
    GLfloat d[2];
};
class XMLDoc { //1736 bytes
    tinyxml2::XMLDocument m_tiny_doc;
    std::string m_root_name;
    std::map<uint32_t, tinyxml2::XMLElement *> m_map;
    char m_path[MAX_PATH] = {};
    bool m_loadResult = false;
public:
    XMLDoc();
    ~XMLDoc();
    void AddCStr(const char *, const char *);
    void AddString(const char *, const std::string &);
    void AddU32(const char *, uint32_t);
    void AddVEC3(const char *, const VEC3 &);
    void AddVEC4(const char *, const VEC4 &);
    bool BufferLoad(const void *, uint32_t);
    void ClearPath(const char *);
    void CopyXmlToParameter(const tinyxml2::XMLElement *, VEC3 *, const char *);
    void CopyXmlToParameter(const tinyxml2::XMLElement *, bool *, const char *);
    void CopyXmlToParameter(const tinyxml2::XMLElement *, float *, const char *);
    void CopyXmlToParameter(const tinyxml2::XMLElement *, int *, const char *);
    void CopyXmlToParameter(const tinyxml2::XMLElement *, std::string *, const char *);
    void CopyXmlToParameter(const tinyxml2::XMLElement *, std::vector<int> *, const char *);
    void CopyXmlToParameter(const tinyxml2::XMLElement *, std::vector<std::string> *, const char *);
    void CopyXmlToParameter(const tinyxml2::XMLElement *, uint32_t *, const char *);
    tinyxml2::XMLElement *CreateElement(tinyxml2::XMLNode *, const std::vector<std::string> &path);
    tinyxml2::XMLElement *CreateNewElement(tinyxml2::XMLNode *, const std::vector<std::string> &, bool);
    tinyxml2::XMLElement *FindElement(const char *path, bool enableCreate);
    tinyxml2::XMLElement *FindFirstElement(tinyxml2::XMLNode *, int, const std::vector<std::string> &path);
    tinyxml2::XMLElement *FindFirstElement(tinyxml2::XMLNode *, const char *, bool, bool);
    tinyxml2::XMLElement *FindNextElement(const char *path, bool b1, bool enableCreate);
    tinyxml2::XMLElement *FindNextMatchingElement(tinyxml2::XMLNode *from, int pidx, const std::vector<std::string> &path, bool goDown);
    bool GetBool(const char *path, bool def, bool a4);
    const char *GetCStr(const char *, const char *, bool);
    float GetF32(const char *, float, bool);
    float *GetF32Array(const char *, bool);
    std::vector<float> GetF32Vector(const char *, bool);
    int GetS32(const char *, int, bool);
    std::vector<std::string> GetStringArray(const char *, bool);
    bool GetTag(const char *, bool);
    unsigned GetU32(const char *, uint32_t, bool);
    VEC3 GetVEC3(const char *, const VEC3 &, bool);
    VEC3 *GetVEC3Array(const char *, bool);
    VEC4 GetVEC4(const char *, const VEC4 &, bool);
    VEC4 *GetVEC4Array(const char *, bool);
    bool Internal_Load(const char *, long *);
    bool Internal_Load(uint32_t, void *);
    bool Load(const char *, long *);
    tinyxml2::XMLError LoadCompressedXML(const std::string &, tinyxml2::XMLDocument &);
    void Save();
    void SaveAs(const char *, bool);
    void SetBool(const char *, bool, bool);
    void SetCStr(const char *path, const char *text, bool enableCreate);
    void SetF32(const char *, float, bool);
    void SetF32Array(char *, int, const float *, bool);
    void SetF32Vector(char *, const std::vector<float> &, bool);
    void SetS32(const char *, int, bool);
    void SetU32(const char *, uint32_t, bool);
    void SetVEC2(const char *, const VEC2 &, bool);
    void SetVEC2(tinyxml2::XMLElement *, const VEC2 &);
    void SetVEC3(const char *, const VEC3 &, bool);
    void SetVEC3(tinyxml2::XMLElement *, const VEC3 &);
    void SetVEC4(const char *, const VEC4 &, bool);
    void SetVEC4(tinyxml2::XMLElement *, const VEC4 &);
    bool UserLoad(const char *);
    void UserSaveAs(const char *, bool);
};

extern XMLDoc g_UserConfigDoc;