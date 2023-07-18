#include "ZwiftApp.h"
std::vector<InstancedObjects *> g_InstancedObjectsLists;
InstancedObjects::InstancedObjects() {
    //TODO
    /*  v2 = operator new(0x1927ui64);
  if ( !v2 )
    invalid_parameter_noinfo_noreturn();
  v3 = &v2->field_0[39] & 0xFFFFFFFFFFFFFFE0ui64;
  *(v3 - 8) = v2;
  *v3 = v3;
  *(v3 + 8) = v3;
  this->m_big = v3;
*/
    /*  v4 = operator new(0x30ui64);
  *v4 = v4;
  v4[1] = v4;
  v4[2] = v4;
  *(v4 + 12) = 257;
  this->m_tree = v4;
*/
    g_InstancedObjectsLists.push_back(this);
}
InstancedObjects::~InstancedObjects() {
    if (!m_field_20) {
        for (auto i = g_InstancedObjectsLists.begin(); i != g_InstancedObjectsLists.end(); i++) {
            if (*i == this) {
                g_InstancedObjectsLists.erase(i);
                break;
            }
        }
    }
    /* TODO tree_dtr(&this[16]);                          // std::__tree<std::__value_type<int,PropType *>
    sub_7FF66CF45AD0(v3, *this);
    v4 = *(*this - 8i64);
    if ((*this - v4 - 8i64) > 0x1F)
        invalid_parameter_noinfo_noreturn();
    j_j_free(v4);*/
}
int g_InstancingMutex = -1;
void INSTANCING_AddInstance(InstancedObjects *, int, MATRIX44, VEC4) {
    //TODO
}
uint32_t INSTANCING_AddMesh(InstancedObjects *a1, const char *a2, uint64_t a3, float a4, BillBoardedSpriteInfo *a5, const VEC4 &a6) {
    //TODO
    return 0;
}
void INSTANCING_ClearInstances() {
    //TODO
}
int g_DithTex = -1;
uint8_t g_bytesDitherTex[64] = {
  255, 127,  31, 159,   7, 135,  39, 167, 191,  63, 223,  95, 199,  71, 231, 103,  47, 175,  15, 143, 55, 183,  23, 151, 239, 111, 207,  79, 247, 119,
  215,  87,  11, 139,  43, 171,   3, 131,  35, 163, 203,  75, 235, 107, 195,  67, 227,  99,  59, 187, 27, 155,  51, 179,  19, 147, 251, 123, 219,  91, 243, 115, 211,  83
};
void GFXAPI_CreateTextureFromR(int hnd, GLsizei w, GLsizei h, const GLvoid *d) {
    auto pGlid = &g_Textures[hnd].m_glid;
    glGenTextures(1, pGlid);
    glBindTexture(GL_TEXTURE_2D, *pGlid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glGetError();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, d);
    auto sq = h * w;
    if (glGetError()) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, d);
        g_VRAMBytes_Textures += 4 * sq;
    } else {
        g_VRAMBytes_Textures += sq;
    }
}
int GFX_CreateTextureFromLuminanceR8(GLsizei w, GLsizei h, const GLvoid *d) {
    auto v3 = g_nTexturesLoaded;
    if (g_nTexturesLoaded >= _countof(g_Textures))
        return -1;
    ++g_nTexturesLoaded;
    if (v3 == -1)
        return -1;
    g_Textures[v3].m_loaded &= ~1u;
    g_Textures[v3].m_bestWidth = w;
    g_Textures[v3].m_bestHeight = h;
    g_Textures[v3].m_align = 2;
    g_Textures[v3].m_field_36_3 = 1;
    g_Textures[v3].m_texState = TS_LOADED;
    GFXAPI_CreateTextureFromR(v3, w, h, d);
    return v3;
}
void INSTANCING_CreateDitherTex() {
    if (g_DithTex == -1) {
        g_DithTex = GFX_CreateTextureFromLuminanceR8(8, 8, g_bytesDitherTex);
        GFX_SetTextureFilter(0, GFF_NEAFEST);
    }
}
void INSTANCING_EndFrame(bool) {
    //empty in Android
}
void INSTANCING_OptimizeGrids() {
    INSTANCING_TakeMutex();
    //TODO
    ZwiftLeaveCriticalSection(g_InstancingMutex);
}
void INSTANCING_RenderAll(InstancedObjects *, GFX_RenderPass, int) {
    //TODO
}
bool INSTANCING_TakeMutex() {
    if (g_InstancingMutex == -1)
        g_InstancingMutex = ZMUTEX_Create("INSTANCINGMUTEX");
    return g_InstancingMutex != -1 && ZwiftEnterCriticalSection(g_InstancingMutex);
}
void INSTANCING_UnloadAll() {
    //TODO
}