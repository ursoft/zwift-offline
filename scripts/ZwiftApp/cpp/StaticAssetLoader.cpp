#include "ZwiftApp.h"
int StaticAssetLoader::LoadGdeFile(const char *name, bool manyInsts) { //vtbl[1]
    return LOADER_LoadGdeFile(name, manyInsts);
}
int StaticAssetLoader::CreateTextureFromTGAFile(const char *fileName) { //vtbl[2]
    return GFX_CreateTextureFromTGAFile(fileName, -1, true);
}
int StaticAssetLoader::CreateShaderFromFile(const char *fileName) { //vtbl[3]
    return GFX_CreateShaderFromFile(fileName, -1);
}
uint32_t StaticAssetLoader::AddMesh(InstancedObjects *instObjs, const char *fileName, uint64_t a4) { //vtbl[4]
    VEC4 v5{};
    return INSTANCING_AddMesh(instObjs, fileName, a4, 1.0f, nullptr, v5);
}
int StaticAssetLoader::SkinGdeFile(const char *fileName) { //vtbl[5]
    return LOAD_CHARACTER_SkinGdeFile(fileName);
}
GDE_Header_360 *StaticAssetLoader::GetMesh(int h) { //vtbl[6]
    return GDEMESH_GetMesh(h);
}
int StaticAssetLoader::GetDefaultNormalTexture(Accessory::Type accTy, protobuf::Gender gen) { //vtbl[7]
    switch (accTy) {
    case Accessory::ACT_JERSEY:
        return GFX_CreateTextureFromTGAFile("Humans/Jerseys/Layout_normWithPocket.tga", -1, true);
    case Accessory::ACT_CLOTHE_4: case Accessory::ACT_CLOTHE_5:
        return GFX_CreateTextureFromTGAFile("Humans/Male/Textures/ClotheNM.tga", -1, true);
    case Accessory::ACT_SHOES:
        return GFX_CreateTextureFromTGAFile("Humans/Accessories/Shoes/Textures/ZwiftRunningShoes02_N.tga", -1, true);
    case Accessory::ACT_SOCKS:
        return GFX_CreateTextureFromTGAFile("Humans/Accessories/Socks/Textures/Socks_Normal.tga", -1, true);
    case Accessory::ACT_GLOVES:
        return GFX_CreateTextureFromTGAFile("Humans/Accessories/Gloves/Textures/ZwiftGloves01Nml.tga", -1, true);
    case Accessory::ACT_TANKTOP:
        return GFX_CreateTextureFromTGAFile(gen == protobuf::MALE ? "Humans/Male/Textures/ClotheTanktopNM.tga" : "Humans/Female/Textures/FemaleTanktopNM.tga", -1, true);
    default:
        return -1;
    }
}
void StaticAssetLoader::LoadWADFile(const char *fileName) { //vtbl[8]
    g_WADManager.LoadWADFile(fileName);
}
void StaticAssetLoader::LoadMeshes(protobuf::Gender gen) {
    if (m_gdeHeadlightcone2 == -1)
        m_gdeHeadlightcone2 = LoadGdeFile("data/headlightcone2.gde", false);
    if (m_tgaBikeShadow == -1)
        m_tgaBikeShadow = CreateTextureFromTGAFile("FX/BikeShadow.tga");
    if (m_tgaBikeHandCycleShadow == -1)
        m_tgaBikeHandCycleShadow = CreateTextureFromTGAFile("FX/BikeHandCycleShadow.tga");
    if (m_shDropShadow == -1)
        m_shDropShadow = CreateShaderFromFile("DropShadow");
    if (m_gdeRide_On == -1)
        m_gdeRide_On = LoadGdeFile("data/Pickups/RideOn/Ride_On.gde", false);
    if (m_gdeRideOnRainbow == -1)
        m_gdeRideOnRainbow = LoadGdeFile("data/Pickups/RideOn/RideOnRainbow.gde", false);
    if (m_gdeRideOnQ == -1)
        m_gdeRideOnQ = LoadGdeFile("data/Pickups/RideOn/RideOnQ.gde", false);
    if (m_gdeRideOnTDZ == -1)
        m_gdeRideOnTDZ = LoadGdeFile("data/Pickups/RideOn/RideOnTDZ.gde", false);
    if (m_gdeGroup_Lead == -1)
        m_gdeGroup_Lead = LoadGdeFile("data/Environment/Effects/Group_Ride_Leader/Group_Lead.gde", false);
    if (m_gdeHighlightBeam == -1)
        m_gdeHighlightBeam = LoadGdeFile("data/Environment/Effects/HighlightBeam.gde", false);
    if (m_gdeWorkoutScreen == -1)
        m_gdeWorkoutScreen = LoadGdeFile("data/Humans/Workout_Parts/WorkoutScreen.gde", false);
    if (m_gdeCassette == -1)
        m_gdeCassette = LoadGdeFile("data/bikes/Components/Cassette/Cassette.gde", false);
    m_meshCassette = AddMesh(&g_BikeInstancedObjects, "data/bikes/Components/Cassette/Cassette.gde", 0);
    if (m_gdePace_Partner == -1)
        m_gdePace_Partner = LoadGdeFile("data/Environment/Effects/Group_Ride_Leader/Pace_Partner.gde", false);
    if (m_shDefaultParticles == -1)
        m_shDefaultParticles = CreateShaderFromFile("defaultParticles");
    if (m_tgaCraneLight == -1)
        m_tgaCraneLight = CreateTextureFromTGAFile("Environment/Structures/ConstructionCrane/Textures/CraneLight.tga");
    if (m_tgaFlare == -1)
        m_tgaFlare = CreateTextureFromTGAFile("flare.tga");
    if (m_tgaLeft_signal == -1)
        m_tgaLeft_signal = CreateTextureFromTGAFile("left_signal.tga");
    if (m_tgaRight_signal == -1)
        m_tgaRight_signal = CreateTextureFromTGAFile("right_signal.tga");
    if (m_tgaLightcone == -1) //fixed by Ursoft, was 0
        m_tgaLightcone = CreateTextureFromTGAFile("lightcone.tga");
    if (m_tgaFognoise == -1) {
        m_tgaFognoise = CreateTextureFromTGAFile("fognoise.tga");
        LoadWADFile("assets/Humans/Male/male.wad");
        LoadWADFile("assets/Humans/Male/male_textures.wad");
        LoadWADFile("assets/Humans/Female/female.wad");
        LoadWADFile("assets/Humans/Female/female_textures.wad");
        const char *names[HK_CNT] = {
            "data/Humans/Male/MaleMasterSkinny.gde", "data/Humans/Male/MaleMaster.gde", "data/Humans/Male/MaleMasterBig.gde", "data/Humans/Female/FemaleMasterSkinny.gde",
            "data/Humans/Female/FemaleMaster.gde", "data/Humans/Skeleton/Skeleton.gde"
        };
        for (int v5 = 0; v5 < HK_CNT; v5++) {
            if (m_gdeSkins[v5] == -1)
                m_gdeSkins[v5] = SkinGdeFile(names[v5]);
            m_midxCyclistKIT[v5] = 0;
            m_midxRunnerKIT[v5] = 0;
            m_midxRunnerKITnew[v5] = 0;
            m_midxRunnerShortsKIT[v5] = 0;
            m_midxSockKIT[v5] = 0;
            m_midxRunningShoes[v5] = 0;
            m_midxRunningShoesNew[v5] = 0;
            m_midxFLESH[v5] = 0;
            m_midxHAIR[v5] = 0;
            m_midxGloveKIT[v5] = 0;
            m_midxThumb[v5] = 0; //fixed by Ursoft
            m_midxBIB[v5] = 0;
            auto mesh = GetMesh(m_gdeSkins[v5]);
            if (mesh) {
                for (int midx = 0; midx < (int)mesh->m_materialsCnt; midx++) {
                    auto m_name = mesh->m_materials[midx].m_name;
                    if (!strcmp(m_name, "CyclistKIT"))
                        m_midxCyclistKIT[v5] = midx;
                    else if (!strcmp(m_name, "RunnerKIT"))
                        m_midxRunnerKIT[v5] = midx;
                    else if (!strcmp(m_name, "RunnerKITnew"))
                        m_midxRunnerKITnew[v5] = midx;
                    else if (!strcmp(m_name, "RunnerShortsKIT"))
                        m_midxRunnerShortsKIT[v5] = midx;
                    else if (!strcmp(m_name, "SockKIT"))
                        m_midxSockKIT[v5] = midx;
                    else if (!strcmp(m_name, "RunningShoes"))
                        m_midxRunningShoes[v5] = midx;
                    else if (!strcmp(m_name, "RunningShoesNew"))
                        m_midxRunningShoesNew[v5] = midx;
                    else if (!strcmp(m_name, "GloveKIT"))
                        m_midxGloveKIT[v5] = midx;
                    else if (strstr(m_name, "FLESH"))
                        m_midxFLESH[v5] = midx;
                    else if (strstr(m_name, "HAIR"))
                        m_midxHAIR[v5] = midx;
                    else if (strstr(m_name, "Thumb"))
                        m_midxThumb[v5] = midx;
                    else if (strstr(m_name, "BIB"))
                        m_midxBIB[v5] = midx;
                    if (fabs(mesh->m_materials[midx].m_field_104) > 0.001f)
                        mesh->m_materials[midx].m_field_104 = 0.0f;
                }
            }
        }
        m_texJersey = GetDefaultNormalTexture(Accessory::ACT_JERSEY, gen);
        m_texTankTop = GetDefaultNormalTexture(Accessory::ACT_TANKTOP, gen);
        m_texClothe = GetDefaultNormalTexture(Accessory::ACT_CLOTHE_4, gen);
        m_texSocks = GetDefaultNormalTexture(Accessory::ACT_SOCKS, gen);
        m_texShoes = GetDefaultNormalTexture(Accessory::ACT_SHOES, gen);
        m_texGloves = GetDefaultNormalTexture(Accessory::ACT_GLOVES, gen);
        for (int v5 = 0; v5 < HK_CNT; v5++) {
            if (v5 > 2) { //3,4,5: female
                m_skinArrayD[v5][AFR_0][DM01] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinAfricanDM01.tga");
                m_skinArrayD[v5][AFR_0][DM02] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinAfricanDM02.tga");
                m_skinArrayD[v5][AFR_0][DM03] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinAfricanDM03.tga");
                m_skinArrayN[v5][AFR_0] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinAfricanNM.tga");

                m_skinArrayD[v5][ASIAN_1][DM01] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinAsianDM01.tga");
                m_skinArrayD[v5][ASIAN_1][DM02] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinAsianDM02.tga");
                m_skinArrayD[v5][ASIAN_1][DM03] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinAsianDM03.tga");
                m_skinArrayN[v5][ASIAN_1] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinAsianNM.tga");

                m_skinArrayD[v5][EURO_2][DM01] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinNewDM01.tga");
                m_skinArrayD[v5][EURO_2][DM02] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinNewDM02.tga");
                m_skinArrayD[v5][EURO_2][DM03] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinNewDM03.tga");
                m_skinArrayN[v5][EURO_2] = CreateTextureFromTGAFile("Humans/Female/Textures/FemaleSkinNewNM.tga");
            } else { //0,1,2:male
                m_skinArrayD[v5][AFR_0][DM01] = CreateTextureFromTGAFile("Humans/Male/Textures/MaleAfricanSkinDM01.tga");
                m_skinArrayD[v5][AFR_0][DM02] = CreateTextureFromTGAFile("Humans/Male/Textures/MaleAfricanSkinDM02.tga");
                m_skinArrayD[v5][AFR_0][DM03] = CreateTextureFromTGAFile("Humans/Male/Textures/MaleAfricanSkinDM03.tga");
                m_skinArrayN[v5][AFR_0] = CreateTextureFromTGAFile("Humans/Male/Textures/MaleAfricanSkinNM.tga");

                m_skinArrayD[v5][ASIAN_1][DM01] = CreateTextureFromTGAFile("Humans/Male/Textures/MaleAsianSkinDM01.tga");
                m_skinArrayD[v5][ASIAN_1][DM02] = CreateTextureFromTGAFile("Humans/Male/Textures/MaleAsianSkinDM02.tga");
                m_skinArrayD[v5][ASIAN_1][DM03] = CreateTextureFromTGAFile("Humans/Male/Textures/MaleAsianSkinDM03.tga");
                m_skinArrayN[v5][ASIAN_1] = CreateTextureFromTGAFile("Humans/Male/Textures/MaleAsianSkinNM.tga");

                m_skinArrayD[v5][EURO_2][DM01] = CreateTextureFromTGAFile("Humans/Male/Textures/SkinDM01.tga");
                m_skinArrayD[v5][EURO_2][DM02] = CreateTextureFromTGAFile("Humans/Male/Textures/SkinDM02.tga");
                m_skinArrayD[v5][EURO_2][DM03] = CreateTextureFromTGAFile("Humans/Male/Textures/SkinDM03.tga");
                m_skinArrayN[v5][EURO_2] = CreateTextureFromTGAFile("Humans/Male/Textures/SkinNM.tga");
            }
        }
        m_tgaNameplate_side = CreateTextureFromTGAFile("UI/nameplate_side.tga");
    }
    m_loaded = true;
}
void StaticAssetLoader::OnPlayerProfileUpdate(BikeEntity *pBE, PlayerProfileUpdateType put) {
    if (put != PUT_FULL)
        m_texTankTop = GetDefaultNormalTexture(Accessory::ACT_TANKTOP, pBE->m_profile.is_male() ? protobuf::MALE : protobuf::FEMALE);
}