#include "ZwiftApp.h"
GNView::GNView() {
    //TODO
}
GNView::~GNView() {
    //TODO auto vector dtr (A: field_98..A0)
}
void GNView::Begin(const GNViewBeginParams &) {
    //TODO A:
    /*  GNScene::BeginView((GNScene *)ZSystem<GNScene>::s_pSystem, a1);
  if ( (int)GFX_GetTier() < 1 )
    return 0xFFFFFFFFLL;
  *(int32x2_t *)&a1->field_80 = vshrn_n_s64(vaddw_u32(vdupq_n_s64(0xFuLL), *(uint32x2_t *)(a2 + 144)), 4uLL);
  CoordinateMap = (float *)GFX_GetCoordinateMap();*/
    //TODO: huge
}
void GNView::Initialize(const GNViewInitializeParams &ip) {
    //TODO A:
    /*
  this->field_0 = ip->field_0;
  v3 = ip->field_10;
  v4 = *(_QWORD *)&ip->field_8;
  this->field_10 = v3;
  *(_QWORD *)&this->field_8 = v4;
  v5 = v3;
  if ( (int)GFX_GetTier() <= 0 )
  {
    *(_QWORD *)&this->field_88 = -1LL;
    *(_DWORD *)&this->field_90 = -1;
  }
  else
  {
    v6 = 4 * ((unsigned __int64)(v5 + 31) >> 5) * (unsigned int)(HIDWORD(v4) * v4) + 62400;
    v10 = v6;
    v11 = 0LL;
    *(_DWORD *)&this->field_88 = GFX_CreateBuffer(&v10);
    v10 = v6;
    v11 = 0LL;
    *(_DWORD *)&this->field_8C = GFX_CreateBuffer(&v10);
    v10 = v6;
    v11 = 0LL;
    *(_DWORD *)&this->field_90 = GFX_CreateBuffer(&v10);
    v7 = (char *)this->field_98;
    v8 = (char *)(this->field_A0 - (void *)v7);
    if ( v6 <= (unsigned __int64)v8 )
    {
      if ( v6 < (unsigned __int64)v8 )
        this->field_A0 = &v7[v6];
    }
    else
    {
      std::vector<unsigned char>::__append((__int64)&this->field_98, v6 - (_QWORD)v8);
    }
    */
}
void GNView::RenderCubeMap(RenderTarget *, const GNViewBeginParams &) {
    //TODO A:
    /*  if ( !rt || *((_BYTE *)rt + 25) != 1 )
    return 0xFFFFFFFFLL;
  g_bReflectionRendering = 1;
  g_GFX_rp = 4;
  GFX_SetShaderColorSpace();
  v6 = (ZwiftAssert *)GFX_MatrixMode(0LL);
  GFX_PushMatrix(v6);
  v7 = (ZwiftAssert *)GFX_MatrixMode(1LL);
  GFX_PushMatrix(v7);
  v8 = (ZwiftAssert *)GFX_MatrixMode(2LL);
  GFX_PushMatrix(v8);
  GFX_LoadIdentity();
  GFX_Perspective(1.5708, 1.0, *(float *)&this->field_18, *(float *)&this->field_1C);
  v51[0] = *(_OWORD *)params;
  v51[1] = *((_OWORD *)params + 1);
  v51[2] = *((_OWORD *)params + 2);
  v51[3] = *((_OWORD *)params + 3);
  v52[0] = *((_OWORD *)params + 4);
  v52[1] = *((_OWORD *)params + 5);
  v52[2] = *((_OWORD *)params + 6);
  v9 = *((_OWORD *)params + 8);
  v52[3] = *((_OWORD *)params + 7);
  v53 = v9;
  v10 = *((_OWORD *)params + 11);
  v11 = *((_OWORD *)params + 9);
  v55 = *((_OWORD *)params + 10);
  v56 = v10;
  v54 = v11;
  v12 = *((_OWORD *)params + 15);
  v14 = *((_OWORD *)params + 12);
  v13 = *((_OWORD *)params + 13);
  v59 = *((_OWORD *)params + 14);
  v60 = v12;
  v57 = v14;
  v58 = v13;
  v61 = *((_OWORD *)params + 16);
  v62 = *((_OWORD *)params + 17);
  v63 = *((_OWORD *)params + 18);
  v64 = *((_OWORD *)params + 19);
  v65 = *((_OWORD *)params + 20);
  v66 = *((_OWORD *)params + 21);
  v67 = *((_OWORD *)params + 22);
  v68 = *((_OWORD *)params + 23);
  v69 = *((_OWORD *)params + 24);
  v70 = *((_OWORD *)params + 25);
  v71 = *((_OWORD *)params + 26);
  v72 = *((_OWORD *)params + 27);
  v73 = *((_OWORD *)params + 28);
  v74 = *((_OWORD *)params + 29);
  v75 = *((_OWORD *)params + 30);
  v15 = *((_OWORD *)params + 32);
  v76 = *((_OWORD *)params + 31);
  v77 = v15;
  v16 = *((_OWORD *)params + 35);
  v17 = *((_OWORD *)params + 33);
  v79 = *((_OWORD *)params + 34);
  v80 = v16;
  v78 = v17;
  v18 = *((_OWORD *)params + 38);
  v19 = *((_DWORD *)params + 156);
  v20 = *((_OWORD *)params + 36);
  *(_OWORD *)v82 = *((_OWORD *)params + 37);
  *(_OWORD *)&v82[16] = v18;
  v83 = v19;
  v81 = v20;
  v54 = *((unsigned __int64 *)rt + 1);
  v55 = xmmword_1DA3200;
  *(_QWORD *)&v82[12] = *(_QWORD *)&this->field_18;
  GFX_MatrixMode(2LL);
  GFX_StoreMatrix(v52);
  v21 = *(_DWORD *)&this->field_70;
  v22 = 0LL;
  v23 = (float *)&dword_1E2D338;
  v24 = (float *)&dword_1E2D2F0;
  *(_QWORD *)&v53 = *(_QWORD *)&this->field_68;
  *((_QWORD *)&v53 + 1) = v21 | 0x3F80000000000000LL;
  do
  {
    VRAM_RenderTo(rt, 61LL, 0LL, 0, 0, 0, v22);
    GFX_MatrixMode(0LL);
    GFX_LoadIdentity();
    GFX_MatrixMode(1LL);
    GFX_LoadIdentity();
    v25 = *(float *)&this->field_68;
    v26 = *(float *)&this->field_6C;
    *(float *)&v27 = *v23;
    *(float *)&v28 = v25 + *(v24 - 2);
    *(float *)&v29 = v26 + *(v24 - 1);
    GFX_LookAt(
      v25,
      v26,
      *(float *)&this->field_70,
      v28,
      v29,
      *(float *)&this->field_70 + *v24,
      v27,
      v38,
      v30,
      v31,
      v32,
      v33,
      v34,
      v35,
      v36,
      v37,
      *((_QWORD *)v23 - 1),
      *v23);
    GFX_StoreMatrix(v51);
    GFX_MatrixMode(0LL);
    GNView::Begin(this, (__int64)v51);
    g_GFX_rp = 0;
    if ( (v82[24] & 1) != 0 )
    {
      v39 = fmodf(*(float *)&g_DayTimeVisualOffset + (float)((float)(*(float *)&g_WorldTime * 0.125) * 24.0), 24.0);
      Sky::Render((ZwiftAssert *)&this->field_68, 0, v39 / 24.0);
    }
    SetupLightmaps(1);
    GFX_UpdateMatrices(0, v40, v41, v42, v43, v44);
    GFX_SetDepthWrite(1);
    GFX_SetDepthTestEnable(1);
    v46 = v82[24];
    if ( (v82[24] & 4) != 0 )
    {
      v45 = (ZwiftAssert *)GameWorld::RenderRoad((GameWorld *)g_pGameWorld);
      v46 = v82[24];
      if ( (v82[24] & 8) == 0 )
      {
LABEL_9:
        if ( (v46 & 2) != 0 )
          goto LABEL_14;
        goto LABEL_10;
      }
    }
    else if ( (v82[24] & 8) == 0 )
    {
      goto LABEL_9;
    }
    v45 = INSTANCING_RenderAll((__int64)g_WorldProps, 0, -1);
    v46 = v82[24];
    if ( (v82[24] & 2) != 0 )
    {
LABEL_14:
      SetupTerrainShaders(v45);
      GFX_SetCullMode(1LL);
      HeightMapManager::Render((HeightMapManager *)g_pHeightMapManager, -1);
      if ( (v82[24] & 0x10) == 0 )
        goto LABEL_4;
      goto LABEL_15;
    }
LABEL_10:
    if ( (v46 & 0x10) == 0 )
      goto LABEL_4;
LABEL_15:
    g_bSkinMeshMaterialCullingHack = 1;
    GameWorld::RenderEntities((GameWorld *)g_pGameWorld);
    g_GFX_rp = 1;
    GameWorld::RenderEntities((GameWorld *)g_pGameWorld);
    g_bSkinMeshMaterialCullingHack = 0;
LABEL_4:
    GNView::End(this);
    ++v22;
    v23 += 3;
    v24 += 3;
  }
  while ( v22 != 6 );
  v48 = GFX_MatrixMode(0LL);
  GFX_PopMatrix(v48);
  v49 = GFX_MatrixMode(1LL);
  GFX_PopMatrix(v49);
  v50 = GFX_MatrixMode(2LL);
  GFX_PopMatrix(v50);
  result = 0LL;
  g_bReflectionRendering = 0;
  return result;*/
}
void GNView::End() {
    g_GNSceneSystem.EndView(this);
}
