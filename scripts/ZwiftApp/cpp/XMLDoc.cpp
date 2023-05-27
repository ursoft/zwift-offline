#include "ZwiftApp.h"
XMLDoc::XMLDoc() {
    //TODO
}
XMLDoc::~XMLDoc() {
    //TODO
}
void XMLDoc::AddCStr(const char *path, const char *value) {
    auto Element = XMLDoc::FindElement(path, false);
    if (!Element) {
        Element = XMLDoc::FindElement(path, true);
        if (Element)
            Element->SetText(value);
        return;
    }
    //TODO
}
void XMLDoc::AddString(const char *path, const std::string &value) {
    AddCStr(path, value.c_str());
}
void XMLDoc::AddU32(const char *, uint32_t) {
    //TODO
}
void XMLDoc::AddVEC3(const char *, const VEC3 &) {
    //TODO
}
void XMLDoc::AddVEC4(const char *, const VEC4 &) {
    //TODO
}
bool XMLDoc::BufferLoad(const void *, uint32_t) {
    //TODO
    return false;
}
void XMLDoc::ClearPath(const char *path) {
    auto e = FindElement(path, false);
    while (e) {
        e->Parent()->DeleteChild(e);
        e = FindElement(path, false);
    }
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, VEC3 *, const char *) {
    //TODO
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, bool *, const char *) {
    //TODO
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, float *, const char *) {
    //TODO
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, int *, const char *) {
    //TODO
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, std::string *, const char *) {
    //TODO
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, std::vector<int> *, const char *) {
    //TODO
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, uint32_t *, const char *) {
    //TODO
}
tinyxml2::XMLElement *XMLDoc::CreateElement(tinyxml2::XMLNode *root, const std::vector<std::string> &path) {
    for (const auto &i : path) {
        auto sib = root->FirstChild();
        for (; sib; sib = sib->NextSibling()) {
            if (i == sib->Value()) break;
        }
        if (sib) {
            root = sib;
        } else {
            auto new_root = root->GetDocument()->NewElement(i.c_str());
            root->InsertEndChild(new_root);
            root = new_root;
        }
    }
    return root->ToElement();
}
tinyxml2::XMLElement *XMLDoc::CreateNewElement(tinyxml2::XMLNode *root, const std::vector<std::string> &path, bool a4) {
    /*for (const auto &i : path) {
        if (a4) {}
        //TODO
    }*/
    return root->ToElement();
}
tinyxml2::XMLElement *XMLDoc::FindFirstElement(tinyxml2::XMLNode *from, int pidx, const std::vector<std::string> &path) {
    if (from == nullptr || pidx < 0 || pidx >= path.size())
        return nullptr;
    auto nv = from->Value();
    if (path[pidx] == nv) {
        if (pidx == path.size() - 1)
            return from->ToElement();
        auto firstChild = from->FirstChild();
        if (firstChild) {
            auto FirstElement = FindFirstElement(firstChild, pidx + 1, path);
            if (FirstElement)
                return FirstElement->ToElement();
        }
    }
    auto next = from->NextSibling();
    if (next) {
        auto FirstElement = FindFirstElement(next, pidx, path);
        if (FirstElement)
            return FirstElement->ToElement();
    }
    return nullptr;
}
tinyxml2::XMLElement *XMLDoc::FindFirstElement(tinyxml2::XMLNode *from, const char *path, bool, bool) {
    //TODO
    return nullptr;
}
tinyxml2::XMLElement *XMLDoc::FindElement(const char *path, bool enableCreate) {
    auto pp = ZStringUtil::Split(path, '\\');
    auto ret = FindFirstElement(m_tiny_doc.FirstChild(), 0, pp);
    if (ret == nullptr && enableCreate)
        ret = CreateElement(&m_tiny_doc, pp);
    return ret;
}
tinyxml2::XMLElement *XMLDoc::FindNextMatchingElement(tinyxml2::XMLNode *from, int pidx, const std::vector<std::string> &path, bool goDown) {
    if (from != nullptr && pidx >= 0 && pidx < path.size()) {
        auto next = from;
        if (!goDown) {
            next = from->NextSibling();
            if (!next) {
                auto parent = from->Parent();
                if (parent) {
                    auto NextMatchingElement = FindNextMatchingElement(parent, pidx - 1, path, false);
                    if (NextMatchingElement)
                        return NextMatchingElement->ToElement();
                }
                return nullptr;
            }
        }
        while (true) {
            if (path[pidx] == next->Value()) {
                if (pidx == path.size() - 1)
                    break;
                auto NextMatchingElement = FindNextMatchingElement(next->FirstChild(), pidx + 1, path, true);
                if (NextMatchingElement)
                    return NextMatchingElement->ToElement();
            }
            next = next->NextSibling();
            if (!next) {
                auto parent = from->Parent();
                if (parent) {
                    auto NextMatchingElement = FindNextMatchingElement(parent, pidx - 1, path, false);
                    if (NextMatchingElement)
                        return NextMatchingElement->ToElement();
                }
                return nullptr;
            }
        }
        return next->ToElement();
    }
    return nullptr;
}
#if 0
static int str_compare(std::string &s1, const char *s2) {
    auto s2_len = strlen(s2), s1_len = (int)s1.length();
    auto min_len = std::min(s1_len, s2_len);
    int ret = memcmp(s1.c_str(), s2, min_len);
    if (!ret) { //eq
        if (s1_len >= s2_len)
            return s1_len > s2_len;
        else
            return -1;
    }
    return ret;
}
#endif
tinyxml2::XMLElement *XMLDoc::FindNextElement(const char *path, bool replaceRoot, bool enableCreate) {
    tinyxml2::XMLElement *ret = nullptr, *nextElement = nullptr;
    std::string          spath(path);
    auto pathCRC = SIG_CalcCaseInsensitiveSignature(path);
    auto pp = ZStringUtil::Split(path, '\\');
    if (!replaceRoot) {
        //if (0 == str_compare(m_root_name, path))
        //    goto LABEL_17;
        if (m_map.contains(pathCRC)) {
//LABEL_17:
            auto from_map = this->m_map[pathCRC];
            nextElement = FindNextMatchingElement(from_map, int(pp.size() - 1), pp, false);
            if (!nextElement) {
                if (!enableCreate)
                    return nullptr; //52
                nextElement = m_tiny_doc.NewElement(from_map->GetText());
                if (from_map->Parent())
                    from_map->Parent()->InsertEndChild(nextElement);
                zassert(nextElement && "XMLDoc::FindNextElement Asserted; nextElement was not created");
            }
            m_map[pathCRC] = nextElement; //goto LABEL50
            return nextElement;
        }
    }
    m_root_name = spath; //originally replace 0...sz
    auto firstChild = this->m_tiny_doc.FirstChild();
    if (!firstChild) { //47
        if (!enableCreate)
            return nullptr;
        nextElement = CreateElement(&m_tiny_doc, pp);
        if (!nextElement)
            return nullptr;
        m_map[pathCRC] = nextElement; //goto50
        return nextElement;
    }
    auto pp_len_m1 = pp.size() - 1;
    tinyxml2::XMLNode *First;
    if (pp[0] != firstChild->Value()) { //64
        //LABEL_64:
        auto next = firstChild->NextSibling();
        if (!next || (First = FindFirstElement(next, 0, pp)) == nullptr) {
            //LABEL_47:
            nextElement = nullptr;
            //LABEL_48:
            if (!enableCreate)
                return nullptr;
            nextElement = CreateElement(&m_tiny_doc, pp);
            if (!nextElement)
                return nullptr;
            m_map[pathCRC] = nextElement; //goto50
            return nextElement;
        }
    } else {
        First = firstChild;
        if (pp_len_m1)
            First = nullptr;
        if (!First) {
            auto v36 = firstChild->FirstChild();
            if (!v36 || (First = FindFirstElement(v36, 1, pp)) == nullptr) {
                //LABEL_64:
                auto next = firstChild->NextSibling();
                if (!next || (First = FindFirstElement(next, 0, pp)) == nullptr) {
                    //LABEL_47:
                    nextElement = nullptr;
                    //LABEL_48:
                    if (!enableCreate)
                        return nullptr;
                    nextElement = CreateElement(&m_tiny_doc, pp);
                    if (!nextElement)
                        return nullptr;
                    m_map[pathCRC] = nextElement; //goto50
                    return nextElement;
                }
            }
        }
    }
    nextElement = First->ToElement();
    if (!nextElement) { //48
        if (!enableCreate)
            return nullptr;
        nextElement = CreateElement(&m_tiny_doc, pp);
        if (!nextElement)
            return nullptr;
        m_map[pathCRC] = nextElement; //goto50
        return nextElement;
    }
//LABEL_50:
    m_map[pathCRC] = nextElement;
//LABEL_51:
    ret = nextElement;
//LABEL_52:
    return ret;
}
bool XMLDoc::GetBool(const char *path, bool def, bool a4) {
    tinyxml2::XMLElement *e = a4 ? FindElement(path, false) : FindNextElement(path, false, false);
    if (!e) return def;
    auto sval = e->GetText();
    return sval && ((sval[0] == '1' && sval[1] == 0) || 0 == strcmp(sval, "true") || 0 == strcmp(sval, "TRUE"));
}
const char *XMLDoc::GetCStr(const char *path, const char *def, bool a4) {
    tinyxml2::XMLElement *e = a4 ? FindElement(path, false) : FindNextElement(path, false, false);
    if (!e) return def;
    return e->GetText();
}
float XMLDoc::GetF32(const char *path, float def, bool a4) {
    tinyxml2::XMLElement *e = a4 ? FindElement(path, false) : FindNextElement(path, false, false);
    if (!e) return def;
    auto sval = e->GetText();
    float ret;
    if (sscanf(sval, "%f", &ret) == 1)
        return ret;
    return def;
}
float *XMLDoc::GetF32Array(const char *, bool) {
    //TODO
    return nullptr;
}
std::vector<float> XMLDoc::GetF32Vector(const char *, bool) {
    std::vector<float> ret;
    //TODO
    return ret;
}
int XMLDoc::GetS32(const char *, int, bool) {
    //TODO
    return 0;
}
std::vector<std::string> XMLDoc::GetStringArray(const char *, bool) {
    std::vector<std::string> ret;
    //TODO
    return ret;
}
bool XMLDoc::GetTag(const char *, bool) {
    //TODO
    return false;
}
unsigned XMLDoc::GetU32(const char *, uint32_t, bool) {
    //TODO
    return 0;
}
VEC3 XMLDoc::GetVEC3(const char *, const VEC3 &, bool) {
    VEC3 ret{};
    //TODO
    return ret;
}
VEC3 *XMLDoc::GetVEC3Array(const char *, bool) {
    //TODO
    return nullptr;
}
VEC4 XMLDoc::GetVEC4(const char *, const VEC4 &, bool) {
    VEC4 ret{};
    //TODO
    return ret;
}
VEC4 *XMLDoc::GetVEC4Array(const char *, bool) {
    //TODO
    return nullptr;
}
bool XMLDoc::Internal_Load(const char *, long *) {
    //TODO
    return false;
}
bool XMLDoc::Internal_Load(uint32_t, void *) {
    //TODO
    return false;
}
bool XMLDoc::Load(const char *, long *) {
    //TODO
    return false;
}
tinyxml2::XMLError XMLDoc::LoadCompressedXML(const std::string &, tinyxml2::XMLDocument &) {
    return tinyxml2::XML_SUCCESS;
}
void XMLDoc::Save() {
    //TODO
}
void XMLDoc::SaveAs(const char *, bool) {
    //TODO
}
void XMLDoc::SetBool(const char *path, bool val, bool enableCreate) {
    auto Element = FindElement(path, enableCreate);
    if (Element)
        Element->SetText(val);
}
void XMLDoc::SetCStr(const char *path, const char *text, bool enableCreate) {
    tinyxml2::XMLElement *ret = enableCreate ? FindElement(path, true) : FindNextElement(path, false, false);
    if (ret)
        ret->SetText(text);
}
void XMLDoc::SetF32(const char *, float, bool) {
    //TODO
}
void XMLDoc::SetF32Array(char *, int, const float *, bool) {
    //TODO
}
void XMLDoc::SetF32Vector(char *, const std::vector<float> &, bool) {
    //TODO
}
void XMLDoc::SetS32(const char *, int, bool) {
    //TODO
}
void XMLDoc::SetU32(const char *, uint32_t, bool) {
    //TODO
}
void XMLDoc::SetVEC2(const char *, const VEC2 &, bool) {
    //TODO
}
void XMLDoc::SetVEC2(tinyxml2::XMLElement *, const VEC2 &) {
    //TODO
}
void XMLDoc::SetVEC3(const char *, const VEC3 &, bool) {
    //TODO
}
void XMLDoc::SetVEC3(tinyxml2::XMLElement *, const VEC3 &) {
    //TODO
}
void XMLDoc::SetVEC4(const char *, const VEC4 &, bool) {
    //TODO
}
void XMLDoc::SetVEC4(tinyxml2::XMLElement *, const VEC4 &) {
    //TODO
}
bool XMLDoc::UserLoad(const char *fileName) {
    auto up = OS_GetUserPath();
    if (up) {
        sprintf_s(m_path, "%s\\Zwift", up);
        CreateDirectoryA(m_path, nullptr);
        sprintf_s(m_path, "%s\\%s", m_path, fileName);
        m_loadResult = (m_tiny_doc.LoadFile(m_path) == 0);
        return m_loadResult;
    }
    return false;
}
void XMLDoc::UserSaveAs(const char *, bool) {
    //TODO
}
