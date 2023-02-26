#pragma once
class BillboardInfoDatabase {
public:
    void LoadBillboardInfoDatabase();
    ~BillboardInfoDatabase();
    static BillboardInfoDatabase *Inst();
    void *GetBBSpriteWrapper(const char *);
    void DestroyBillboardInfoDatabase();
};
inline BillboardInfoDatabase g_BillboardInfo;
class ShrubHelperInfoDatabase {
public:
    void LoadShrubHelperInfoDatabase();
    ~ShrubHelperInfoDatabase();
    static ShrubHelperInfoDatabase *Inst();
    const char *GetShrubHelperInfoWithGDEName(const std::string &);
};
inline ShrubHelperInfoDatabase g_ShrubHelperInfo;
class RegionsDatabase {
public:
    static void LoadRegionsDatabase();
    void ParseRegionAttributes(tinyxml2::XMLElement *, const std::string &);
    void LoadLocalRegionTypes();
    void GetRegionTypeNameIndex(const char *);
    void GetRegionTypeName(uint32_t);
    void GetRegionAttribute(const char *);
    void DestroyRegionsDatabase();
};
