#include "ZwiftApp.h"
XMLDoc g_UserConfigDoc;

XMLDoc::XMLDoc() {
    //TODO
}

XMLDoc::~XMLDoc() {
    //TODO
}

void XMLDoc::Save() {
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
        while (1) {
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

tinyxml2::XMLElement *XMLDoc::FindNextElement(const char *path, bool b1, bool enableCreate) {
    std::string spath(path);
    auto pathCRC = SIG_CalcCaseInsensitiveSignature(path);
    auto pp = ZStringUtil::Split(path, '\\');
    if (!b1) {}
    //TODO
    return nullptr;
}

void XMLDoc::SetCStr(const char *path, const char *text, bool enableCreate) {
    tinyxml2::XMLElement *ret = enableCreate ? FindElement(path, true) : FindNextElement(path, false, false);
    if (ret)
        ret->SetText(text);
}
