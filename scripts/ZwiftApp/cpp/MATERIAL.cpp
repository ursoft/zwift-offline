#include "ZwiftApp.h" //READY for testing
VEC4 g_globalSpecularSettings, g_globalFresnelSettings;
float g_defaultSpecularPower, g_FlipbookMaterialTimeOffset;
bool g_UVOffsetOverrideB1, g_UVOffsetOverrideB2;
float g_UVOffsetOverride1, g_UVOffsetOverride2;
float g_UVOffsetOverrideF1, g_UVOffsetOverrideF2;
int CalculateFlipbookTextureIndex(const GDE_flipbookHeader *fbh) {
    if (fbh->m_count) {
        if (fbh->m_count == 1)
            return fbh->m_texIndexes[0];
        auto t = fmodf(g_FlipbookMaterialTimeOffset + g_MaterialTime, fbh->m_totalDuration);
        if (fbh->m_texTimes[0] > t)
            return fbh->m_texIndexes[0];
        auto v5 = fbh->m_count - 1;
        if (t > fbh->m_texTimes[v5])
            return fbh->m_texIndexes[v5];
        for (int i = 1; i < fbh->m_count; i++)
            if (t <= fbh->m_texTimes[i])
                return fbh->m_texIndexes[i - 1];
    }
    return -1;
}
float g_SelfIllumIntensityBias, g_SelfIllumDayNightLerpVal;
struct GDE_MaterialUsageConst0 {
    char field_0[8];
    char *m_name1;
    int m_cond1;
    int16_t m_reg1;
    char field_16[10];
    char *m_name2;
    int m_cond2;
    int16_t m_reg2;
    char field_2E;
    char field_2F;
    VEC4 m_vec;
    void (*m_func)(VEC4 *, VEC4 *, void *);
    char m_type;
    char field_49;
    char field_4A;
    char field_4B;
    int m_field_4C;
};
struct GDE_MaterialUsageConst1 {
    int m_defTex;
    char field4[14];
    int16_t m_texArr7[7];
    char field_20, field_21, field_22, field_23;
    bool m_field_24, field_25, field_26, field_27;
    void (*m_func)(int *, int *, void *);
    char field_30[32];
};
void doMatActivate(GDE_MaterialUsageConst *constParams, uint8_t cntConstParam, void *a3) {
    static_assert(sizeof(GDE_MaterialUsageConst0) == 80);
    static_assert(sizeof(GDE_MaterialUsageConst1) == 80);
    auto ep = constParams + cntConstParam;
    VEC4 vec;
    int tex;
    while (constParams < ep) {
        auto constParams0 = (GDE_MaterialUsageConst0 *)constParams;
        auto constParams1 = (GDE_MaterialUsageConst1 *)constParams;
        switch (constParams->m_type) {
        case 0:
            vec = constParams0->m_vec;
            if (constParams0->m_func)
                constParams0->m_func(&vec, &vec, a3);
            if (constParams0->m_cond1 != -1)
                GFX_SetVrtShaderConstByName_Vec4(GFX_UserRegister{ constParams0->m_reg1, constParams0->m_name1 }, vec);
            if (constParams0->m_cond2 != -1)
                GFX_SetVrtShaderConstByName_Vec4(GFX_UserRegister{ constParams0->m_reg2, constParams0->m_name2 }, vec);
            break;
        case 1:
            tex = constParams1->m_defTex;
            if (constParams1->m_func) {
                int v47 = 0, v45 = 0;
                constParams1->m_func(&v47, &v45, a3);
                if (v47) {
                    if (v47 >= 7)
                        v47 = 7;
                    auto v33 = constParams1->m_texArr7[v47];
                    if (v33 != -1)
                        tex = v33;
                }
            }
            if (tex != -1 && !constParams1->m_field_24)
                GFX_ActivateTexture(tex, -1, nullptr, TWM_REPEAT);
            break;
        default:
            break;
        }
        ++constParams;
    }
}
void MATERIAL_Activate(Material_360 *mat, GDE_MaterialUsage *us, void *a3) {
    if (g_GFX_rp == 3 || g_GFX_rp == 2) {
        GFX_ActivateTexture(mat->m_texGlid[0], 0, nullptr, TWM_REPEAT);
        GFX_UploadShaderVEC4(GSR_23, VEC4{0.0f, mat->m_mbmAlpha ? 0.01f : 0.5f, 0.0f, 0.0f}, mat->m_mbmAlpha ? 0x74657374i64 : 0x6F706171i64);
        return;
    }
    GFXAPI_SetFlipbookTextureIndexOverride(-1); // MATERIAL_ApplyAnimators inlined
    if ((mat->m_bits & 0x10) && mat->m_hasAnimators && mat->m_pAnimators->m_flipbookHeader)
        GFXAPI_SetFlipbookTextureIndexOverride(CalculateFlipbookTextureIndex(mat->m_pAnimators->m_flipbookHeader));
    if (us->m_field_18 && us->m_field_18 <= 0xFFFFFFFE && us->m_cntConstParam) {
        doMatActivate(us->m_pConstantParams, us->m_cntConstParam, a3);
    } else {
        auto v8 = mat->m_texGlid[0];
        if (v8 != -1) {
            if (mat->m_bits & 0x80) {
                v8 = _countof(g_Textures) - 1;
                TEXMGR_OverrideHWHandle(v8, mat->m_ovrTexHandle);
            }
            GFX_ActivateTexture(v8, 0, nullptr, TWM_REPEAT);
        }
        auto v9 = mat->m_texGlid[2];
        if (v9 != -1)
            GFX_ActivateTexture(v9, 7, nullptr, TWM_REPEAT);
        auto v10 = mat->m_texGlid[1];
        if (v10 == -1)
            v10 = g_DefaultNormalMap;
        GFX_ActivateTexture(v10, 1, nullptr, TWM_REPEAT);
        if (mat->m_bits & 0x100) {
            auto v11 = mat->m_texGlid[3];
            if (v11 != -1)
                GFX_ActivateTexture(v11, 2, nullptr, TWM_REPEAT);
            auto v12 = mat->m_texGlid[4];
            if (v12 == -1)
                v12 = g_DefaultNormalMap;
            GFX_ActivateTexture(v12, 3, nullptr, TWM_REPEAT);
        }
    }
    ActivateVideoMaterial(mat->m_name);
    auto v17 = 0i64;
    VEC4 vec;
    if (g_UVOffsetOverrideB1) {
        vec = { g_UVOffsetOverrideB2 ? fmodf(g_MaterialTime * mat->m_field_104 + g_UVOffsetOverrideF1, 1.0f) : g_UVOffsetOverrideF1,
                g_UVOffsetOverrideB2 ? fmodf(g_MaterialTime * mat->m_field_108 + g_UVOffsetOverrideF2, 1.0f) : g_UVOffsetOverrideF2,
                g_UVOffsetOverride1,
                g_UVOffsetOverride2 };
    } else {
        vec = { fmodf(mat->m_field_104 * g_MaterialTime, 1.0f), fmodf(mat->m_field_108 * g_MaterialTime, 1.0f), 0.0f, 0.0f };
        if (mat->m_field_104 == 0.0f && mat->m_field_108 == 0.0f)
            v17 = 0x696E6974i64;
    }
    GFX_UploadShaderVEC4(GSR_7, vec, v17);
    if (mat->m_specularPower >= 0.01f)
        g_globalSpecularSettings.m_data[0] = mat->m_specularPower;
    else
        g_globalSpecularSettings.m_data[0] = g_defaultSpecularPower;
    GFX_UploadShaderVEC4(GSR_22, g_globalSpecularSettings, 0i64);
    GFX_UploadShaderVEC4(GSR_21, g_globalFresnelSettings, 0i64);
    auto v21 = 0x696E6974i64;
    VEC4 v26{};
    if (mat->m_bits & 0x40) {
        v21 = 0i64;
        v26.m_data[0] = (1.0f - g_SelfIllumDayNightLerpVal) * mat->m_field_11C + g_SelfIllumDayNightLerpVal * mat->m_field_118 + g_SelfIllumIntensityBias;
    }
    if (mat->m_mbmAlpha) {
        v26.m_data[1] = 0.01f;
        GFX_SetAlphaBlendEnable(true);
        if (v21 == 0x696E6974)
            v21 = 1952805748i64;
    } else {
        GFX_SetAlphaBlendEnable(false);
        v26.m_data[1] = 0.5f;
        if (v21 == 0x696E6974)
            v21 = 0x6F706171i64;
    }
    GFX_UploadShaderVEC4(GSR_23, v26, v21);
    GFX_SetBlendFunc(GBO_FUNC_ADD, GB_SRC_ALPHA, (mat->m_mbmAlpha == MBM_3) ? GB_TRUE : GB_ONE_MINUS_SRC_ALPHA);
}
int MATERIAL_GetDefaultNormalMap() {
    return g_DefaultNormalMap;
}
float MATERIAL_GetFlipbookTimeOffset() {
    return g_FlipbookMaterialTimeOffset;
}
void MATERIAL_Init() {
    g_DefaultNormalMap = GFX_CreateTextureFromTGAFile("defaultNormalMap.tga", -1, true);
}
void MATERIAL_Reset() {
    //A: empty
}
void MATERIAL_SetFlipbookTimeOffset(float v) {
    g_FlipbookMaterialTimeOffset = v;
}
void MATERIAL_SetGlobalLighting(const VEC4 &spec, const VEC4 &fresnel) {
    g_globalSpecularSettings = spec;
    g_defaultSpecularPower = g_globalSpecularSettings.m_data[0];
    g_globalFresnelSettings = fresnel;
}
void MATERIAL_SetUVOffsetOverride(bool a1, float a2, float a3, bool a4) {
    g_UVOffsetOverride1 = 0.0f;
    g_UVOffsetOverride2 = 0.0f;
    g_UVOffsetOverrideB1 = a1;
    g_UVOffsetOverrideB2 = a4;
    g_UVOffsetOverrideF1 = a2;
    g_UVOffsetOverrideF2 = a3;
}
void MATERIAL_Update(float dt) {
    UpdateVideoMaterials();
    g_MaterialTime += dt;
}