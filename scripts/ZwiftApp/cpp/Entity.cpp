#include "ZwiftApp.h"
EntityManager EntityManager::g_entityMgr;
#if 0 //not found
bool EntityFileManager::LoadLightXMLFile(const char *) {
    XMLDoc v5;
    sub_DA9E70(v14, v6, v7, v8, this, "lights.xml");
    v9 = (EntityFileManager *)(*(__int64(__fastcall **)(XMLDoc *, char *, _QWORD))(*(_QWORD *)v5 + 16LL))(v5, v14, 0LL);
    if (((unsigned __int8)v9 & 1) != 0)
    {
        EntityFileManager::ParseEntityFile(v9, (char *)v5, a2, v10);
        (*(void(__fastcall **)(XMLDoc *))(*(_QWORD *)v5 + 8LL))(v5);
        return (char *)1;
    } else
    {
        ZwiftAssert::IsBeingDebugged_(v9);
        v12 = ZwiftAssert::BeforeAbort_(
            "false && \"GameWorld::Load failed; XMLDoc cannot load lights.xml\"",
            "/Users/jenkins/git/deploy-macos-agent-persistent/Projects/ZwiftApp/CODE/EntityFileManager.cpp",
            (unsigned int)&qword_170 + 3,
            0);
        v11 = 0;
        if (v12)
        {
            ZwiftAssert::Abort_((ZwiftAssert *)v12);
            return 0;
        }
    }
    return (char *)v11;
}
#endif
void EntityFileManager::ParseEntityFile(XMLDoc *xent) {
    auto ent = xent->FindElement("world\\entities\\ent", false);
    while (ent) {
        Entity *eo = nullptr;
        if (!EntityFactory::CreateEntityObject(ent, &eo))
            break;
        EntityManager::GetInst()->AddEntity(eo);
        ent = ent->NextSiblingElement("ent");
    }
}
void EntityManager::AddEntity(Entity *) {
    //TODO
}
void EntityManager::Render() {
    //TODO
}
void EntityManager::Render2D() {
    //TODO
}
void EntityManager::Update(float dt) { //inlined into GameWorld::Update
    //TODO
}
void EntityManager::ClearAllRenderLists() { //inlined into onGameWorldDtr
    //TODO
}
void EntityManager::Destroy() { //inlined into onGameWorldDtr
    //TODO
}
Entity *EntityFactory::Create(Entity::EType, tinyxml2::XMLElement *src) {
    //TODO
    return nullptr;
}
bool EntityFactory::CreateEntityObject(Entity::EType, Entity **) {
    //TODO
    return true;
}
bool EntityFactory::CreateEntityObject(tinyxml2::XMLElement *, Entity **) {
    //TODO
    return true;
}
void EntityFactory::Destroy() {
    //TODO
}
EntityFactory::EntityFactory() {
    //TODO
}
EntityFactory *EntityFactory::GetInst() {
    //TODO
    return nullptr;
}
EntityFactory::~EntityFactory() {
    //TODO
}
