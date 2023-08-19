//UT Coverage: 99%, 636/639, ENOUGH (3 indulgenced) READY for testing
#include "CRC.h"
#include "Logging.h"
#include "ZStringUtil.h"
#include "WADManager.h"
#include "OS.h"
#include "MAT.h"
#include "XMLDoc.h"
XMLDoc::XMLDoc() {
    //empty
}
XMLDoc::~XMLDoc() {
    if (m_wadCopy)
        free(m_wadCopy);
}
#if 0
void XMLDoc::AddCStr(const char *path, const char *value) {
    //unused
    assert(false);
}
void XMLDoc::AddString(const char *path, const std::string &value) {
    AddCStr(path, value.c_str());
}
void XMLDoc::AddU32(const char *, uint32_t) {
    //unused
    assert(false);
}
void XMLDoc::AddVEC3(const char *, const VEC3 &) {
    //unused
    assert(false);
}
void XMLDoc::AddVEC4(const char *, const VEC4 &) {
    //unused
    assert(false);
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, VEC3 *, const char *) {
    //unused
    assert(false);
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, bool *, const char *) {
    //unused
    assert(false);
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, float *, const char *) {
    //unused
    assert(false);
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, int *, const char *) {
    //unused
    assert(false);
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, std::string *, const char *) {
    //unused
    assert(false);
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *, uint32_t *, const char *) {
    //unused
    assert(false);
}
std::vector<float> XMLDoc::GetF32Vector(const char *, bool) {
    std::vector<float> ret;
    //unused
    assert(false);
    return ret;
}
VEC4 XMLDoc::GetVEC4(const char *, const VEC4 &, bool) {
    VEC4 ret{};
    //unused
    assert(false);
    return ret;
}
VEC4 *XMLDoc::GetVEC4Array(const char *, bool) {
    //unused
    assert(false);
    return nullptr;
}
bool XMLDoc::Internal_Load(const char *, long *) {
    //unused
    assert(false);
    return false;
}
bool XMLDoc::Internal_Load(uint32_t, void *) {
    //unused
    assert(false);
    return false;
}
void XMLDoc::SetVEC2(const char *ename, const VEC2 &val) {
    //unused
    assert(false);
}
void XMLDoc::SetVEC2(tinyxml2::XMLElement *e, const VEC2 &val) {
    //unused
    assert(false);
}
void XMLDoc::SetVEC3(const char *ename, const VEC3 &val) {
    //unused
    assert(false);
}
void XMLDoc::SetVEC3(tinyxml2::XMLElement *e, const VEC3 &val) {
    //unused
    assert(false);
}
void XMLDoc::SetVEC4(const char *ename, const VEC4 &val) {
    //unused
    assert(false);
}
void XMLDoc::SetVEC4(tinyxml2::XMLElement *e, const VEC4 &) {
    //unused
    assert(false);
}
void XMLDoc::SetF32Array(const char *ename, int sz, const float *vals) {
    //unused
    assert(false);
}
void XMLDoc::SetF32Vector(const char *ename, const std::vector<float> &vals) {
    //unused
    assert(false);
}
tinyxml2::XMLElement *XMLDoc::CreateNewElement(tinyxml2::XMLNode *root, const std::vector<std::string> &path, bool a4) {
    //unused
    assert(false);
    return root->ToElement();
}
tinyxml2::XMLElement *XMLDoc::FindFirstElement(tinyxml2::XMLNode *from, const char *path, bool, bool) {
    //unused
    assert(false);
    return nullptr;
}
tinyxml2::XMLError XMLDoc::LoadCompressedXML(const std::string &, tinyxml2::XMLDocument &) {
    //unused
    assert(false);
    return tinyxml2::XML_SUCCESS;
}
#endif
void XMLDoc::GetVEC3Array(tinyxml2::XMLElement *el, const char *chNameFormat, std::vector<VEC3> *pRet) {
    char name[128];
    pRet->reserve(128);
    for (int i = 0; ; i++) {
        sprintf_s(name, chNameFormat, i);
        auto ch = el->FirstChildElement(name);
        if (!ch)
            break;
        VEC3 item{};
        sscanf_s(ch->GetText(), "%f,%f,%f", item.m_data, item.m_data + 1, item.m_data + 2);
        pRet->push_back(item);
    }
}
void XMLDoc::GetVEC2(tinyxml2::XMLElement *el, VEC2 *pRet, const VEC2 &def) {
    if (!el || 2 != sscanf_s(el->GetText(), "%f,%f", pRet->m_data, pRet->m_data + 1))
        *pRet = def;
}
void XMLDoc::GetVEC3(tinyxml2::XMLElement *el, VEC3 *pRet, const VEC3 &def) {
    if (!el || 3 != sscanf_s(el->GetText(), "%f,%f,%f", pRet->m_data, pRet->m_data + 1, pRet->m_data + 2))
        *pRet = def;
}
void XMLDoc::GetVEC4(const tinyxml2::XMLAttribute *at, VEC4 *pRet, const VEC4 &def, const char *fmt) {
    if (!at || 4 != sscanf_s(at->Value(), fmt, pRet->m_data, pRet->m_data + 1, pRet->m_data + 2, pRet->m_data + 3))
        *pRet = def;
}
bool XMLDoc::BufferLoad(const void *src, uint32_t sz) {
    if (m_wadCopy)
        free(m_wadCopy);
    m_wadCopy = (char *)malloc(sz + 1);
    memmove(m_wadCopy, src, sz);
    m_wadCopy[sz] = 0;
    if (m_tiny_doc.Parse(m_wadCopy, sz)) {
        m_loadResult = false;
        free(m_wadCopy);
        m_wadCopy = nullptr;
        Log("*** Failed to process XML document from buffer!");
    } else {
        m_loadResult = true;
    }
    strcpy(m_path, "UNNAMED");
    m_version = 0;
    return m_loadResult;
}
void XMLDoc::ClearPath(const char *path) {
    auto e = FindElement(path, false);
    while (e) {
        e->Parent()->DeleteChild(e);
        e = FindElement(path, false);
    }
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *el, std::vector<int> *dest, const char *name) {
    auto e = el->FirstChildElement(name);
    if (e) {
        auto t = e->GetText();
        if (t && *t) {
            auto sdest = ZStringUtil::Split(t, ',');
            std::transform(sdest.begin(), sdest.end(), std::back_inserter(*dest),
                [](const std::string &str) { return std::stoi(str); });
        }
    }
}
void XMLDoc::CopyXmlToParameter(const tinyxml2::XMLElement *el, std::vector<std::string> *dest, const char *name) {
    auto e = el->FirstChildElement(name);
    if (e) {
        auto t = e->GetText();
        if (t && *t)
            *dest = ZStringUtil::Split(t, ',');
    }
}
tinyxml2::XMLElement *XMLDoc::GetElt(const char *ename, bool distinctMode, bool enableCreate) {
    if (distinctMode)
        return FindElement(ename, enableCreate);
    else
        return FindNextElement(ename, false, enableCreate);
}
bool XMLDoc::GetBool(const char *path, bool def, bool distinctMode) {
    auto e = GetElt(path, distinctMode, false);
    if (!e) return def;
    auto sval = e->GetText();
    return sval && ((sval[0] == '1' && sval[1] == 0) || 0 == strcmp(sval, "true") || 0 == strcmp(sval, "TRUE"));
}
const char *XMLDoc::GetCStr(const char *path, const char *def, bool distinctMode) {
    auto e = GetElt(path, distinctMode, false);
    if (!e) return def;
    return e->GetText();
}
float XMLDoc::GetF32(const char *path, float def, bool distinctMode) {
    auto e = GetElt(path, distinctMode, false);
    if (e) {
        auto sval = e->GetText();
        float ret;
        if (sscanf(sval, "%f", &ret) == 1)
            return ret;
    }
    return def;
}
uint32_t XMLDoc::GetU32(const char *path, uint32_t def) {
    auto e = FindElement(path, false);
    if (e) {
        auto sval = e->GetText();
        uint32_t ret;
        if (sscanf(sval, "%u", &ret) == 1)
            return ret;
    }
    return def;
}
int XMLDoc::GetS32(const char *path, int def, bool distinctMode) {
    auto e = FindElement(path, false);
    if (e) {
        auto sval = e->GetText();
        int ret;
        if (sscanf(sval, "%d", &ret) == 1)
            return ret;
    }
    return def;
}
float *XMLDoc::GetF32Array(const char *ename, bool distinctMode) {
    float *ret = nullptr;
    auto e = GetElt(ename, distinctMode, false);
    if (e) {
        auto s = e->GetText();
        if (s) {
            auto v19 = ZStringUtil::Split(s, ',');
            ret = new float[v19.size()];
            int i = 0;
            for (auto &is : v19) {
                ret[i] = 0.0f;
                sscanf(is.c_str(), "%f", ret + i++);
            }
        }
    }
    return ret;
}
tinyxml2::XMLElement *XMLDoc::CreateElement(tinyxml2::XMLNode *root, const std::vector<std::string> &path) {
    for (const auto &i : path) {
        auto sib = root->FirstChild();
        for (; sib; sib = sib->NextSibling()) {
            if (i == sib->Value()) 
                break;
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
static int str_compare(std::string &s1, const char *s2) {
    auto s2_len = (int)strlen(s2), s1_len = (int)s1.length();
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
tinyxml2::XMLElement *XMLDoc::FindNextElement(const char *path, bool fromBegin, bool enableCreate) {
    tinyxml2::XMLElement *ret = nullptr, *nextElement = nullptr;
    std::string          spath(path);
    auto pathCRC = SIG_CalcCaseInsensitiveSignature(path);
    auto pp = ZStringUtil::Split(path, '\\');
    if (!fromBegin) {
        if ((*path && 0 == str_compare(m_root_name, path)) || m_map.contains(pathCRC)) {
            auto from_map = this->m_map[pathCRC];
            nextElement = FindNextMatchingElement(from_map, int(pp.size() - 1), pp, false);
            if (!nextElement) {
                if (!enableCreate)
                    return nullptr; //52
                nextElement = m_tiny_doc.NewElement(from_map->Value());
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
            return nullptr; //UT indulgence
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
    }
//LABEL_50:
    m_map[pathCRC] = nextElement;
//LABEL_51:
    ret = nextElement;
//LABEL_52:
    return ret;
}
std::vector<std::string> XMLDoc::GetStringArray(const char *ename, bool distinctMode) {
    std::vector<std::string> ret;
    auto e = GetElt(ename, distinctMode, false);
    while (e) {
        ret.push_back(e->GetText());
        e = e->NextSiblingElement();
    }
    return ret;
}
bool XMLDoc::Load(const char *aName, time_t *aTime) {
    auto ret = false;
    std::string saName(aName);
    int offset = 0;
    if (saName.starts_with("data/"))
        offset = 5;
    time_t wadTouchTime = 0;
    auto WadFileHeaderByItemName = g_WADManager.GetWadFileHeaderByItemName(aName + offset, WAD_ASSET_TYPE::GLOBAL, &wadTouchTime);
    struct _stat64i32 v63{};
    if (_stat64i32(saName.c_str(), &v63) /*no such file*/ && offset == 0) {
        saName = "data/" + saName; //ZStringUtil::PrependFolder
        if (_stat64i32(saName.c_str(), &v63) /*no such file*/ && !WadFileHeaderByItemName)
            return ret;
    }
    if (v63.st_mtime <= wadTouchTime && WadFileHeaderByItemName) { //select wad by time as newer
        m_path[0] = 0;
        m_wadCopy = (char *)malloc(WadFileHeaderByItemName->m_fileLength + 1);
        memmove(m_wadCopy, WadFileHeaderByItemName->FirstChar(), WadFileHeaderByItemName->m_fileLength);
        *(m_wadCopy + WadFileHeaderByItemName->m_fileLength) = 0;
        if (m_tiny_doc.Parse(m_wadCopy, WadFileHeaderByItemName->m_fileLength)) {
            m_loadResult = false;
            Log("*** had WAD file header, but failed to process XML document.");
            return ret;
        }
        m_loadResult = true;
        ret = true;
        if (aTime)
            *aTime = wadTouchTime;
    } else {
        strncpy_s(m_path, saName.c_str(), saName.length());
        auto err = m_tiny_doc.LoadFile(m_path);
        if (err == tinyxml2::XML_SUCCESS) {
            ret = m_loadResult = true;
        } else {
            ret = m_loadResult = false;
            Log("XML Error %d loading loose document %s", err, m_path);
        }
        if (aTime)
            *aTime = v63.st_mtime;
    }
    if (ret && m_tiny_doc.LastChild()) {
        auto v = m_tiny_doc.LastChild()->FirstChildElement("VERSION");
        if (v)
            m_version = v->IntText(0);
    }
    return ret;
}
int XMLDoc::Save() {
    return SaveAs(m_path, false);
}
void XMLDoc::SetU32(const char *ename, uint32_t val) {
    auto el = FindElement(ename, true);
    if (el)
        el->SetText(val);
}
int XMLDoc::SaveAs(const char *path, bool addVersion) {
    errno = 0;
    auto v6 = fopen(path, "w");
    if (v6) {
        if (addVersion) {
            m_tiny_doc.InsertFirstChild(m_tiny_doc.NewDeclaration());
            auto v = m_tiny_doc.NewElement("VERSION");
            v->SetText(m_version);
            m_tiny_doc.LastChild()->InsertEndChild(v);
        }
        tinyxml2::XMLPrinter v12(v6);
        m_tiny_doc.Print(&v12);
        fclose(v6);
    } else {
        Log("XMLDoc::SaveAs  FAILED TO OPEN filename = %s", path);
    }
    return errno;
}
void XMLDoc::SetBool(const char *path, bool val) {
    auto Element = FindElement(path, true);
    if (Element)
        Element->SetText(val ? "1" : "0");
}
void XMLDoc::SetCStr(const char *path, const char *text) {
    tinyxml2::XMLElement *ret = FindElement(path, true);
    if (ret)
        ret->SetText(text);
}
void XMLDoc::SetF32(const char *ename, float val) {
    auto el = FindElement(ename, true);
    if (el)
        el->SetText(val);
}
void XMLDoc::SetS32(const char *ename, int val) {
    auto el = FindElement(ename, true);
    if (el)
        el->SetText(val);
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
    return false; //UT indulgence
}
int XMLDoc::UserSaveAs(const char *fileName, bool addDec) {
    errno = 0;
    auto up = OS_GetUserPath();
    if (up) {
        sprintf_s(m_path, "%s\\Zwift", up);
        CreateDirectoryA(m_path, nullptr);
        sprintf_s(m_path, "%s\\%s", m_path, fileName);
        auto v7 = fopen(m_path, "w");
        if (v7) {
            if (addDec)
                m_tiny_doc.InsertFirstChild(m_tiny_doc.NewDeclaration());
            tinyxml2::XMLPrinter v10(v7);
            m_tiny_doc.Print(&v10);
            fclose(v7);
        }
    }
    return errno;
}

TEST(SmokeTestXML, NonDistinct) {
    XMLDoc x;
    auto g = "<g><c/><a>1</a><b>2</b><a>3</a><b><c>3</c></b></g><x2>2</x2><x2>3</x2><x2><x3/></x2><x2/><x2>4</x2><g><b>1</b></g><g><a>10</a></g>";
    EXPECT_TRUE(x.BufferLoad(g, (uint32_t)strlen(g)));
    int sum = 0;
    for (auto i = x.FindNextElement("g\\a", true, false); i; i = x.GetElt("g\\a", false, false))
        sum += i->IntText();
    EXPECT_EQ(14, sum);
    sum = 0;
    for (auto i = x.FindNextElement("g\\b", true, false); i; i = x.GetElt("g\\b", false, false))
        sum += i->IntText();
    EXPECT_EQ(3, sum);
    sum = 0;
    for (auto i = x.FindNextElement("g\\b\\no", true, false); i; i = x.GetElt("g\\b\\no", false, false))
        sum += i->IntText(); //UT indulgence
    EXPECT_EQ(0, sum);
    sum = 0;
    for (auto i = x.FindNextElement("g\\a", true, false); i; i = x.GetElt("g\\a", false, false))
        sum += i->IntText();
    EXPECT_EQ(14, sum);
    sum = 0;
    for (auto i = x.FindNextElement("x2", true, false); i; i = x.GetElt("x2", false, false))
        sum += i->IntText();
    EXPECT_EQ(9, sum);

    int sum1 = 0, sum2 = 0;
    for (auto i = x.FindNextElement("g\\a", true, false), j = x.FindNextElement("x2", true, false); i || j; i = x.GetElt("g\\a", false, false), j = x.GetElt("x2", false, false)) {
        if (i) sum1 += i->IntText();
        if (j) sum2 += j->IntText();
    }
    EXPECT_EQ(14, sum1);
    EXPECT_EQ(9, sum2);
    auto newa = x.FindNextElement("g\\a", false, true);
    EXPECT_NE(nullptr, newa);
    newa->SetText(100);
    sum = 0;
    for (auto i = x.FindNextElement("g\\a", true, false); i; i = x.GetElt("g\\a", false, false))
        sum += i->IntText();
    EXPECT_EQ(114, sum);
    tinyxml2::XMLPrinter p(nullptr, true);
    x.m_tiny_doc.Print(&p);
    auto c = p.CStr();
    EXPECT_STREQ("<g><c/><a>1</a><b>2</b><a>3</a><b><c>3</c></b></g><x2>2</x2><x2>3</x2><x2><x3/></x2><x2/><x2>4</x2><g><b>1</b></g><g><a>10</a><a>100</a></g>", c);
    newa = x.FindNextElement("g\\a", true, true);
    EXPECT_NE(nullptr, newa);
    newa->SetText(1000);
    sum = 0;
    for (auto i = x.FindNextElement("g\\a", true, false); i; i = x.GetElt("g\\a", false, false))
        sum += i->IntText();
    EXPECT_EQ(1113, sum);
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><c/><a>1000</a><b>2</b><a>3</a><b><c>3</c></b></g><x2>2</x2><x2>3</x2><x2><x3/></x2><x2/><x2>4</x2><g><b>1</b></g><g><a>10</a><a>100</a></g>", c);
    newa = x.FindNextElement("g\\e", true, true);
    EXPECT_NE(nullptr, newa);
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><c/><a>1000</a><b>2</b><a>3</a><b><c>3</c></b><e/></g><x2>2</x2><x2>3</x2><x2><x3/></x2><x2/><x2>4</x2><g><b>1</b></g><g><a>10</a><a>100</a></g>", c);
    newa = x.FindNextElement("h", true, true);
    EXPECT_NE(nullptr, newa);
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><c/><a>1000</a><b>2</b><a>3</a><b><c>3</c></b><e/></g><x2>2</x2><x2>3</x2><x2><x3/></x2><x2/><x2>4</x2><g><b>1</b></g><g><a>10</a><a>100</a></g><h/>", c);

    sum1 = 0, sum2 = 0;
    for (auto i = x.FindNextElement("g\\b\\c", true, false), j = x.FindNextElement("g\\b", true, false); i || j; i = x.GetElt("g\\b\\c", false, false), j = x.GetElt("g\\b", false, false)) {
        if (i) sum1 += i->IntText();
        if (j) sum2 += j->IntText();
    }
    EXPECT_EQ(3, sum1);
    EXPECT_EQ(3, sum2);
    EXPECT_EQ(nullptr, x.FindNextElement("no", true, false));

    XMLDoc y;
    EXPECT_EQ(nullptr, y.FindNextElement("g\\b\\no", true, false));
    auto e = "<??>";
    EXPECT_TRUE(y.BufferLoad(e, (uint32_t)strlen(e)));
    EXPECT_EQ(nullptr, y.FindNextElement("", true, false));
    EXPECT_EQ(nullptr, y.FindNextElement("", true, true));
    EXPECT_EQ(nullptr, y.FindNextElement("", false, true));

    XMLDoc z;
    EXPECT_NE(nullptr, z.FindNextElement("", true, true));
    z.m_tiny_doc.FirstChild()->InsertFirstChild(z.m_tiny_doc.NewDeclaration("")); //too dirty, need nicer way to cover those lines
    EXPECT_EQ(nullptr, z.FindNextElement("\\", true, true));
    p.ClearBuffer();
    z.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<><??></>", c);

    XMLDoc fc;
    fc.m_tiny_doc.InsertFirstChild(fc.m_tiny_doc.NewElement("q"));
    fc.m_tiny_doc.InsertEndChild(fc.m_tiny_doc.NewElement("w"));
    fc.m_tiny_doc.FirstChild()->NextSibling()->InsertFirstChild(fc.m_tiny_doc.NewDeclaration("")); //too dirty, need nicer way to cover those lines
    EXPECT_EQ(nullptr, fc.FindNextElement("w\\", true, true));
}
TEST(SmokeTestXML, DISABLED_LoadBadXmlFromWad) {
    g_WADManager.LoadWADFile("assets\\Worlds\\worlds_bad.wad");
    XMLDoc x;
    time_t t{};
    EXPECT_FALSE(x.Load("data/Worlds/propgeomrandomizeinfo.xml", &t));
    EXPECT_EQ(0, t);
    g_WADManager.DeleteAllWadFiles();
}
TEST(SmokeTestXML, LoadWadParams) {
    g_WADManager.LoadWADFile("assets\\Worlds\\worlds.wad");
    XMLDoc x;
    time_t t{};
    EXPECT_TRUE(x.Load("data/Worlds/propgeomrandomizeinfo.xml", &t));
    EXPECT_GE(t, 1667957809);
    EXPECT_TRUE(x.Load("Worlds/propgeomrandomizeinfo.xml", &t));
    EXPECT_GE(t, 1667957809);
    std::vector<int> vecInt;
    std::vector<std::string> vecStr;
    auto i = x.FindElement("GEOMSETS\\GEOMSET\\MODELS\\MODEL", false);
    EXPECT_NE(nullptr, i);
    x.CopyXmlToParameter(i, &vecStr, "GDEINCLUDE");
    EXPECT_EQ(4, vecStr.size());
    EXPECT_EQ("Student"s, vecStr[0]);
    EXPECT_EQ("JapanMaleCountry"s, vecStr[3]);
    auto j = x.FindElement("GEOMSETS\\GEOMSET\\MODELS\\MODEL\\RANDOMSETS\\SET", false);
    EXPECT_NE(nullptr, j);
    x.CopyXmlToParameter(j, &vecInt, "GROUPS");
    EXPECT_EQ(6, vecInt.size());
    EXPECT_EQ(2, vecInt[0]);
    EXPECT_EQ(7, vecInt[5]);
    g_WADManager.DeleteAllWadFiles();
}
TEST(SmokeTestXML, SaveLoadFiles) {
    XMLDoc x;
    EXPECT_FALSE(x.UserLoad("knowndevices-notexist.xml"));
    auto t = GetTickCount();
    std::string name_v = std::string(x.m_path) + "_v";
    EXPECT_TRUE(x.UserLoad("knowndevices.xml"));
    auto sa = x.GetStringArray("DEVICES\\DEVICE", true);
    EXPECT_LT(0, sa.size());
    x.SetU32("DEVICES\\ticks", t);
    EXPECT_EQ(0, x.UserSaveAs("knowndevices_t.xml", true));
    EXPECT_TRUE(x.UserLoad("knowndevices_t.xml"));
    EXPECT_EQ(t, x.GetU32("DEVICES\\ticks", t + 1));
    EXPECT_TRUE(x.UserLoad("knowndevices.xml"));
    x.m_version = (int)t;
    EXPECT_EQ(0, x.SaveAs(name_v.c_str(), true));
    x.m_version++;
    time_t ti{};
    EXPECT_TRUE(x.Load(name_v.c_str(), &ti));
    EXPECT_GE(ti, 1687631446);
    EXPECT_EQ((int)t, x.m_version);
    EXPECT_TRUE(x.UserLoad("MapSchedule_v2.xml"));
    EXPECT_EQ((int)t, x.m_version);
    EXPECT_TRUE(x.Load(x.m_path, nullptr));
    EXPECT_EQ(1, x.m_version);
    x.m_path[0] = '*';
    EXPECT_EQ(2, x.Save());
    EXPECT_FALSE(x.Load(x.m_path, nullptr));
    x.SetCStr("1\\2", "some_val"); //for now, two high-level nodes is not enough to be bad for tinyxml2 - so we made 1 (digit tag name)
    EXPECT_EQ(0, x.UserSaveAs("MapSchedule_v2.bad", false));
    EXPECT_FALSE(x.Load(x.m_path, nullptr));
}
TEST(SmokeTestXML, Distinct) {
    XMLDoc x;
    EXPECT_EQ(nullptr, x.GetElt("", true, false));
    std::vector<std::string> dummy{"1"};
    EXPECT_EQ(nullptr, x.FindFirstElement(nullptr, 0, dummy));
    EXPECT_FALSE(x.BufferLoad("<bad", 4));
    auto g = "<g><a/><b><c>text</c></b><d/></g>";
    EXPECT_TRUE(x.BufferLoad(g, (uint32_t)strlen(g)));
    EXPECT_EQ(nullptr, x.FindFirstElement(x.m_tiny_doc.FirstChild(), -1, dummy));
    EXPECT_EQ(nullptr, x.FindFirstElement(x.m_tiny_doc.FirstChild(), 2, dummy));
    EXPECT_TRUE(x.m_loadResult);
    EXPECT_STREQ("UNNAMED", x.m_path);
    EXPECT_EQ(0, x.m_version);
    EXPECT_TRUE(x.GetBool("not\\exists", true, true));
    EXPECT_FALSE(x.GetBool("not\\exists", false, true));
    EXPECT_FALSE(x.GetBool("g\\b\\c", true, true));
    x.ClearPath("g\\b");
    tinyxml2::XMLPrinter p(nullptr, true);
    x.m_tiny_doc.Print(&p);
    auto c = p.CStr();
    EXPECT_STREQ("<g><a/><d/></g>", c);
    EXPECT_TRUE(x.BufferLoad(c, (uint32_t)strlen(c)));
    p.ClearBuffer();
    x.ClearPath("g\\d");
    x.SetCStr("g\\a", "123");
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><a>123</a></g>", c);
    EXPECT_FALSE(x.GetBool("g\\a", true, true));
    x.SetCStr("g\\a", "1");
    EXPECT_EQ(1u, x.GetU32("g\\a", 2));
    EXPECT_EQ(2u, x.GetU32("g\\not", 2));
    EXPECT_EQ(1, x.GetS32("g\\a", -2, true));
    EXPECT_EQ(-2, x.GetS32("g\\not", -2, true));
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><a>1</a></g>", c);
    EXPECT_TRUE(x.GetBool("g\\a", false, true));
    x.SetS32("g\\a", -2);
    EXPECT_EQ(-2, x.GetS32("g\\a", -3, true));
    EXPECT_EQ(4294967294u, x.GetU32("g\\a", 3));
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><a>-2</a></g>", c);
    x.SetU32("g\\a", 4294967293u);
    EXPECT_EQ(-3, x.GetS32("g\\a", -3, true));
    EXPECT_EQ(4294967293u, x.GetU32("g\\a", 3));
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><a>4294967293</a></g>", c);
    x.SetF32("g\\a", -1.234f);
    EXPECT_FLOAT_EQ(-1.234f, x.GetF32("g\\a", 1.234f, true));
    EXPECT_FLOAT_EQ(1.234f, x.GetF32("g\\not", 1.234f, true));
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><a>-1.234</a></g>", c);
    EXPECT_EQ(nullptr, x.GetF32Array("g\\not", true));
    auto fa = x.GetF32Array("g\\a", true);
    EXPECT_NE(nullptr, fa);
    if (fa) {
        EXPECT_FLOAT_EQ(-1.234f, fa[0]);
        delete[] fa;
    }
    EXPECT_EQ(nullptr, x.GetF32Array("g", true));
    fa = x.GetF32Array("g", true);
    x.SetCStr("g\\a", "true,1");
    fa = x.GetF32Array("g\\a", true);
    EXPECT_NE(nullptr, fa);
    if (fa) {
        EXPECT_FLOAT_EQ(0.0f, fa[0]);
        EXPECT_FLOAT_EQ(1.0f, fa[1]);
        delete[] fa;
    }
    EXPECT_STREQ("true,1", x.GetCStr("g\\a", "def", true));
    EXPECT_STREQ("def", x.GetCStr("g\\not", "def", true));
    EXPECT_FALSE(x.GetBool("g\\a", true, true));
    x.SetCStr("g\\a", "true");
    EXPECT_TRUE(x.GetBool("g\\a", false, true));
    x.SetCStr("g\\a", "TRUE");
    EXPECT_TRUE(x.GetBool("g\\a", false, true));
    x.SetCStr("g\\a", "TRUE1");
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><a>TRUE1</a></g>", c);
    EXPECT_FALSE(x.GetBool("g\\a", true, true));
    x.SetBool("g\\a", true);
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><a>1</a></g>", c);
    EXPECT_TRUE(x.GetBool("g\\a", false, true));
    x.SetBool("g\\a", false);
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><a>0</a></g>", c);
    EXPECT_FALSE(x.GetBool("g\\a", true, true));
    x.SetCStr("not\\exists", "some_val");
    p.ClearBuffer();
    x.m_tiny_doc.Print(&p);
    c = p.CStr();
    EXPECT_STREQ("<g><a>0</a></g><not><exists>some_val</exists></not>", c); //QUEST ill-formed xml, is it OK?
}