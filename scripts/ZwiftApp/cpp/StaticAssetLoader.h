#pragma once
struct InstancedObjects;
struct GDE_Header_360;
enum PlayerProfileUpdateType { PUT_FULL /*PlayerBikeFullProfileUpdate*/, PUT_LIMITED /*PlayerBikeLimitedProfileUpdate*/, PUT_NETWORKED /*NetworkedBikeProfileUpdate*/, PUT_OTHER /*PlayerProfileUpdate*/ };
struct StaticAssetLoader { //0x2d5 bytes
    virtual ~StaticAssetLoader() {} //vtbl[0]
    virtual int LoadGdeFile(const char *name, bool manyInsts); //vtbl[1]
    virtual int CreateTextureFromTGAFile(const char *fileName); //vtbl[2]
    virtual int CreateShaderFromFile(const char *fileName); //vtbl[3]
    virtual uint32_t AddMesh(InstancedObjects *instObjs, const char *fileName, uint64_t a4); //vtbl[4]
    virtual int SkinGdeFile(const char *fileName); //vtbl[5]
    virtual GDE_Header_360 *GetMesh(int h); //vtbl[6]
    virtual int GetDefaultNormalTexture(Accessory::Type accTy, protobuf::Gender gen); //vtbl[7]
    virtual void LoadWADFile(const char *fileName); //vtbl[8]
    uint32_t m_meshCassette = 0;
    int m_gdeHeadlightcone2 = -1, m_tgaBikeShadow = -1, m_tgaBikeHandCycleShadow = -1, m_shDropShadow = -1, m_gdeRide_On = -1, m_gdeRideOnRainbow = -1, m_gdeRideOnQ = -1, m_gdeRideOnTDZ = -1, m_gdeGroup_Lead = -1,
        m_gdeHighlightBeam = -1, m_gdeWorkoutScreen = -1, m_gdeCassette = -1, m_gdePace_Partner = -1, m_shDefaultParticles = -1, m_tgaCraneLight = -1, m_tgaFlare = -1, m_tgaLeft_signal = -1, m_tgaRight_signal = -1, 
        m_tgaLightcone = -1, m_tgaFognoise = -1, m_tgaNameplate_side = -1, m_texJersey = -1, m_texTankTop = -1, m_texClothe = -1, m_texSocks = -1, m_texShoes = -1, m_texGloves = -1;
    enum HumanKinds { MALE_MASTER_S, MALE_MASTER, MALE_MASTER_BIG, FEMALE_MASTER_S, FEMALE_MASTER, FEMALE_MASTER_BIG, HK_CNT };
    enum SkinIndexing2 { AFR_0, ASIAN_1, EURO_2, SI2_CNT };
    enum SkinIndexing3 { DM01, DM02, DM03, SI3_CNT };
    int m_skinArrayD[HK_CNT][SI2_CNT][SI3_CNT]{};
    int m_skinArrayN[HK_CNT][SI2_CNT]{};
    int m_gdeSkins[HK_CNT]{}, m_midxCyclistKIT[HK_CNT]{}, m_midxRunnerKIT[HK_CNT]{}, m_midxRunnerKITnew[HK_CNT]{}, m_midxRunnerShortsKIT[HK_CNT]{}, m_midxSockKIT[HK_CNT]{}, m_midxRunningShoes[HK_CNT]{};
    int m_midxRunningShoesNew[HK_CNT]{}, m_midxFLESH[HK_CNT]{}, m_midxHAIR[HK_CNT]{}, m_midxThumb[HK_CNT]{}, m_midxGloveKIT[HK_CNT]{}, m_midxBIB[HK_CNT]{};
    bool m_loaded = false;
    void LoadMeshes(protobuf::Gender gen);
    void OnPlayerProfileUpdate(BikeEntity *pBE, PlayerProfileUpdateType put);
};
