#include "ZwiftApp.h"
void BillboardInfoDatabase::LoadBillboardInfoDatabase() { /*TODO*/ }
BillboardInfoDatabase::~BillboardInfoDatabase() { /*TODO*/ }
BillboardInfoDatabase *BillboardInfoDatabase::Inst() { return &g_BillboardInfo; }
void *BillboardInfoDatabase::GetBBSpriteWrapper(const char *) { /*TODO*/ return nullptr; }
void BillboardInfoDatabase::DestroyBillboardInfoDatabase() { /*TODO*/ }

ShrubHelperInfoDatabase::~ShrubHelperInfoDatabase() {
    //TODO
}
void ShrubHelperInfoDatabase::LoadShrubHelperInfoDatabase() {
    //TODO
}
ShrubHelperInfoDatabase *ShrubHelperInfoDatabase::Inst() { return &g_ShrubHelperInfo; }
const char *ShrubHelperInfoDatabase::GetShrubHelperInfoWithGDEName(const std::string &) {
    //TODO
    return nullptr;
}

void RegionsDatabase::LoadRegionsDatabase() {
    //TODO
}
void RegionsDatabase::ParseRegionAttributes(tinyxml2::XMLElement *, const std::string &) {
    //TODO
}
void RegionsDatabase::LoadLocalRegionTypes() {
    //TODO
}
void RegionsDatabase::GetRegionTypeNameIndex(const char *) {
    //TODO
}
void RegionsDatabase::GetRegionTypeName(uint32_t) {
    //TODO
}
void RegionsDatabase::GetRegionAttribute(const char *) {
    //TODO
}
void RegionsDatabase::DestroyRegionsDatabase() {
    //TODO
}
