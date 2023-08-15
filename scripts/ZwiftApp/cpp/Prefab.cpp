#include "ZwiftApp.h"
void PrefabFileManager::LoadPrefabsXML(XMLDoc *xdoc) {
    auto pfs = xdoc->FindNextElement("PREFABS", true, false);
    if (pfs) {
        auto pf = pfs->FirstChildElement("PREFAB");
        while (pf) {
            PrefabManager::Instance()->CreatePrefab(pf, false);
            pf = pf->NextSiblingElement("PREFAB");
        }
    }
}