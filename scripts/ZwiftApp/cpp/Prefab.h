#pragma once
struct EntityNode {
};
struct Prefab {
    Prefab(const std::string &) {
        //TODO
    }
    Prefab(const std::string &, const std::vector<EntityNode> &) {
        //TODO
    }
    EntityNode GetEntityNodeWithId(uint32_t) {
        //TODO
    }
    void Spawn(const std::vector<Entity *> &) {
        //TODO
    }
    ~Prefab() {
        //TODO
    }
};
struct PrefabManager {
    static PrefabManager *Instance() {
        //TODO
        return nullptr;
    }
    PrefabManager() {
        //TODO
    }
    void AddEntry(Prefab *) {
        //TODO
    }
    ~PrefabManager() {
        //TODO
    }
    Prefab *CreatePrefab(tinyxml2::XMLElement *, bool) {
        //TODO
        return nullptr;
    }
    void GetEntityTreeOfPrefab(const std::string &) {
        //TODO
    }
    Prefab *GetPrefab(const std::string &) {
        //TODO
    }
    void RemoveEntry(Prefab *) {
        //TODO
    }
    void SpawnPrefab(const std::string &, const VEC3 &) {
        //TODO
    }
};
namespace PrefabFileManager {
    void LoadPrefabsXML(XMLDoc *xdoc);
}