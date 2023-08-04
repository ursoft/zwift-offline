#include "ZwiftApp.h"
void SetupGameCameras() {
    auto mainBike = BikeManager::Instance()->m_mainBike;
    g_FollowCam = g_CameraManager.CreateCamera(mainBike, CT_BIKE);
    g_FollowCam->m_bikeLocalPos = VEC3{ -150.0f, 200.0f, 0.0f };
    g_FollowCam->m_field_74 = VEC3{ 200.0f, 150.0f, 0.0f };
    g_FollowCam->m_field_B0 = 0.1f;
    g_FollowCam->m_field_B4 = 0.5f;
    g_FollowCam->m_isElastic = true; //Camera::SetElasticity(1)
    g_CameraManager.m_needSort = true;
    g_SideCam = g_CameraManager.CreateCamera(mainBike, CT_BIKE);
    g_SideCam->m_bikeLocalPos = VEC3{ 200.0f, 100.0f, -200.0f };
    g_SideCam->m_field_74 = VEC3{ 50.0f, 80.0f, 100.0f };
    g_SideCam->m_field_B0 = 0.1f;
    g_SideCam->m_field_B4 = 0.5f;
    g_SideCam->m_isElastic = true;
    g_SideCam->m_field_100 = 23.0f;
    g_SideCam->m_field_104 = 46.0f;
    g_pTitleCamera = g_CameraManager.CreateCamera(nullptr, CT_TITLE);
    g_pTitleCamera->m_field_B0 = 1.002f;
    g_pTitleCamera->m_field_B4 = 0.25f;
    g_HeadCam = g_CameraManager.CreateCamera(mainBike, CT_BIKE);
    g_HeadCam->m_bikeLocalPos = VEC3{ 45.0f, 150.0f, 0.0f };
    g_HeadCam->m_field_74 = VEC3{ 200.0f, 140.0f, 0.0f };
    g_HeadCam->m_field_B0 = 0.03f;
    g_HeadCam->m_field_B4 = 0.5f;
    g_DollyCam = g_CameraManager.CreateCamera(mainBike, CT_BIGBRO);
    g_DollyCam->m_bikeLocalPos = VEC3{ 2000.0f, 80.0f, 425.0f };
    g_DollyCam->m_field_74 = VEC3{ 0.0f, 110.0f, 0.0f };
    g_DollyCam->m_localVec = VEC3{ 200.0f, 0.0f, 0.0f };
    g_HeliCam = g_CameraManager.CreateCamera(mainBike, CT_BIGBRO);
    g_HeliCam->m_bikeLocalPos = VEC3{ 32000.0f, 20000.0f, 4000.0f };
    g_HeliCam->m_field_74 = VEC3{ 40.0f, 100.0f, 0.0f };
    g_HeliCam->m_field_80 = 2.0f;
    g_HeliCam->m_field_84 = 2.0f;
    g_HeliCam->m_field_B0 = 1.1f;
    g_HeliCam->m_field_B4 = 0.4f;
    g_HeliCam->m_field_88 = 1000.0f;
    g_HeliCam->m_field_BC = 1.0f;
    g_HeliCam->m_field_C0 = true;
    g_OrbitCam = g_CameraManager.CreateCamera(mainBike, CT_ORBIT);
    g_OrbitCam->m_bikeLocalPos = VEC3{ 500.0f, 0.0f, 0.0f };
    g_OrbitCam->m_field_74 = VEC3{ 0.0f, 110.0f, 0.0f };
    g_CloseCam = g_CameraManager.CreateCamera(mainBike, CT_BIKE);
    g_CloseCam->m_bikeLocalPos = VEC3{ -100.0f, 180.0f, 0.0f };
    g_CloseCam->m_field_74 = VEC3{ 15.0f, 170.0f, 0.0f };
    g_CloseCam->m_field_B0 = 0.08f;
    g_CloseCam->m_field_B4 = 0.5f;
    g_CloseCam->m_isElastic = true;
    g_WheelCam = g_CameraManager.CreateCamera(mainBike, CT_BIKE);
    g_WheelCam->m_bikeLocalPos = VEC3{ -92.0f, 70.0f, 35.0f };
    g_WheelCam->m_field_74 = VEC3{ 200.0f, 120.0f, 25.0f };
    g_WheelCam->m_field_80 = 90.0f;
    g_WheelCam->m_field_84 = 90.0f;
    g_WheelCam->m_isElastic = true;
    g_WheelCam->m_field_100 = 23.0f;
    g_WheelCam->m_field_104 = 46.0f;
    g_LeadCam = g_CameraManager.CreateCamera(mainBike, CT_BIKE);
    g_LeadCam->m_bikeLocalPos = VEC3{ 500.0f, 170.0f, 0.0f };
    g_LeadCam->m_field_74 = VEC3{ -80.0f, 150.0f, 0.0f };
    g_LeadCam->m_field_B0 = 0.2f;
    g_LeadCam->m_field_B4 = 0.5f;
    g_LeadCam->m_isElastic = true;
    g_LeadCam->m_field_D0 = 4000.0f;
    g_LeadCam->m_field_100 = 30.0f;
    g_LeadCam->m_field_104 = 46.0f;
    g_ClassicCam = g_CameraManager.CreateCamera(mainBike, CT_BIKE);
    g_ClassicCam->m_bikeLocalPos = VEC3{ -400.0f, 1000.0f, 0.0f };
    g_ClassicCam->m_field_74 = VEC3{ 60.0f, 0.0f, 0.0f };
    g_ClassicCam->m_field_B0 = 0.08f;
    g_ClassicCam->m_field_B4 = 0.5f;
    g_FreeCam = g_CameraManager.CreateCamera(nullptr, CT_FREE);
}
Camera *CameraManager::GetSelectedCamera() {
    return m_cameras[m_selIdx];
}
Camera *CameraManager::CreateCamera(Entity *e, CAMTYPE ty) {
    auto ret = new Camera(e, ty);
    m_cameras.push_back(ret);
    return ret;
}
CameraManager::~CameraManager() {
    for (auto c : m_cameras)
        delete c;
}
VEC3 CameraManager::GetCameraPos() {
    bool oculus = g_tweakArray[TWI_BENABLEOCULUS].IntValue() == 1;
    if (oculus) {
        auto c = GetSelectedCamera();
        if (c) {
            auto e = c->GetLookAtEntity();
            if (e)
                return e->m_pos;
        }
    }
    return m_pos;
}
int CameraManager::FindCamera(Camera *cam) {
    for (int idx = 0; idx < m_cameras.size(); idx++)
        if (m_cameras[idx] == cam)
            return idx;
    return 0; //QUEST: why not -1
}
void CameraManager::CutToCamera(Camera *cam, float cut) {
    auto idx = FindCamera(cam);
    if (m_selIdx != idx) {
        AUDIO_SetVariable("camera_type", (float)idx);
        m_prevSelIdx = m_selIdx;
        m_selIdx = idx;
        m_cut = cut;
        m_cutInv = (cut == 0.0f) ? 1.0f : 0.0f;
        cam->m_bCut = true;
        cam->m_field_B8 = 0.0f;
        if (g_tweakArray[TWI_BENABLEOCULUS].IntValue() == 1)
            m_pos = cam->m_pos;
    }
    cam->m_field_80 = cam->m_field_84;
}
bool SortElasticFirst(Camera *a1, Camera *a2) {
    return a1->m_isElastic && !a2->m_isElastic;
}
void CameraManager::Update(float dt) {
    if (m_disabledElast) {
        if (m_disabledElastVal)
            m_disabledElastVal--;
        else
            m_disabledElast = false;
    }
    if (m_needSort) {
        std::ranges::sort(m_cameras, SortElasticFirst);
        m_needSort = false;
    }
    int v8 = 0;
    for (auto i : m_cameras) {
        zassert(i && "Camera is set to nullptr!");
        if (!i->m_isElastic)
            break;
        auto v14 = (i == GetSelectedCamera()) ? 1 / 60.0f : 1 / 30.0f;
        auto v15 = int(dt * v14);
        float v17;
        if (dt - v15 * v14 == 0.0f)
            v17 = 0.0f;
        else
            v17 = 1.0f;
        i->m_field_EC = i->m_bikeWorldPos;
        for (int s = 0; s < v15; s++)
            i->Update(v14, s / (v15 + v17));
        i->Update(dt - v14 * v15, 1.0f);
        v8++;
    }
    auto sc = GetSelectedCamera();
    if (v8 <= m_selIdx)
        sc->Update(dt, 1.0f);
    if (m_cutInv >= 1.0f) {
        m_pos = sc->m_pos;
        m_field_50 = sc->m_field_34;
    } else {
        m_cutInv = dt / m_cut + m_cutInv;
        auto prev_cam = m_cameras[m_prevSelIdx];
        m_pos.m_data[0] = (sc->m_pos.m_data[0] - prev_cam->m_pos.m_data[0]) * m_cutInv + prev_cam->m_pos.m_data[0];
        m_pos.m_data[1] = (sc->m_pos.m_data[1] - prev_cam->m_pos.m_data[1]) * m_cutInv + prev_cam->m_pos.m_data[1];
        m_pos.m_data[2] = (sc->m_pos.m_data[2] - prev_cam->m_pos.m_data[2]) * m_cutInv + prev_cam->m_pos.m_data[2];
        m_field_50.m_data[0] = (sc->m_field_34.m_data[0] - prev_cam->m_field_34.m_data[0]) * m_cutInv + prev_cam->m_field_34.m_data[0];
        m_field_50.m_data[1] = (sc->m_field_34.m_data[1] - prev_cam->m_field_34.m_data[1]) * m_cutInv + prev_cam->m_field_34.m_data[1];
        m_field_50.m_data[2] = (sc->m_field_34.m_data[2] - prev_cam->m_field_34.m_data[2]) * m_cutInv + prev_cam->m_field_34.m_data[2];
    }
    auto v71 = sc->m_field_34 - sc->m_pos;
    auto v53 = v71.len();
    if (v53 != 0.0f && v53 != 1.0f) {
        auto v54 = 1.0f / v53;
        v71.m_data[2] *= v54;
        v71.m_data[1] *= v54;
        v71.m_data[0] *= v54;
    }
    bool oculus = g_tweakArray[TWI_BENABLEOCULUS].IntValue() == 1;
    if (oculus)
        v71 = VEC3{ 1.0f, 0.0f, 0.0f };
    AUDIO_SetListenerPosition(1, sc->m_pos, v71);
    m_tmpCamVal -= dt;
}
void CameraManager_Update(float dt) {
    if (!g_DesiredCam)
        g_DesiredCam = g_FollowCam;
    if (g_friendsListGUIObj) {
        static Camera *g_camArray[] = { g_FollowCam, g_SideCam, g_DollyCam, g_HeliCam, g_WheelCam, g_LeadCam };
        static float g_accTime;
        static int g_randCamIdx;
        bool v5 = false;
        auto SubgroupEvent = GroupEvents::FindSubgroupEvent(g_GroupEventsActive_CurrentEventId);
        if (SubgroupEvent && SubgroupEvent->m_field_198 == 7)
            v5 = true;
        else if (g_currentPrivateEvent)
            v5 = g_currentPrivateEvent->m_field_1C4 == 4;
        if ((g_friendsListGUIObj->m_field_428 || v5) && !g_friendsListGUIObj->m_changeCamera) {
            g_accTime += dt;
            if (g_accTime > 7.0f) {
                g_accTime -= 7.0f;
                while (true) {
                    int v10 = rand() % _countof(g_camArray);
                    if (v10 != g_randCamIdx) {
                        g_randCamIdx = v10;
                        break;
                    }
                }
                g_DesiredCam = g_camArray[g_randCamIdx];
                if (g_DesiredCam == g_HeliCam) {
                    g_DesiredCam->m_field_80 = 1.0f;
                    g_DesiredCam->m_field_84 = 1.0f;
                    g_DesiredCam->m_field_BC = 2.0f;
                }
            }
        }
        if (g_friendsListGUIObj->m_fanView) {
            g_accTime = 0.0f;
            g_friendsListGUIObj->m_fanView = false;
            g_friendsListGUIObj->m_changeCamera = false;
            g_DesiredCam = g_FollowCam;
        }
    }
    bool v12 = false;
    bool camChanged = false;
    VEC3 io{}, pos{};
    auto pSK = g_CameraManager.GetSelectedCamera();
    if (g_CameraManager.m_tmpCam && g_CameraManager.m_tmpCamVal > 0.0f) {
        v12 = true;
        g_CameraManager.m_tmpCam->SetLookAtEntity(pSK->GetLookAtEntity());
        g_CameraManager.CutToCamera(g_CameraManager.m_tmpCam, 0.0f);
        pSK = g_CameraManager.GetSelectedCamera();
    } else {
        if (g_DesiredCam == g_DollyCam || g_DesiredCam == g_FollowCam) {
            auto &ents = EntityManager::GetInst()->m_ents_photocam;
            if (!ents.empty()) {
                std::vector<PhotoCamEntity *> v192(ents);
                std::vector<PhotoCamEntity *> v191;
                bool breaked = false;
                for (auto v24 : v192) {
                    auto v25 = (BikeEntity *)g_DollyCam->GetLookAtEntity();
                    if (v25) {
                        //PhotoCamEntity::Available(PhotoCamEntity *this, BikeEntity*) inlined
                        auto &p = v24->m_field_310 ? v24->m_field_314 : v24->m_pos;
                        auto v28 = (v25->GetPosition() - p).len();
                        if (v28 < v24->m_field_2B8.m_data[1] && (
                            v24->m_field_30C == -1 || (v24->m_field_30C == 1 && v25->m_field_8B8) || (v24->m_field_30C == 0 && !v25->m_field_8B8)
                            )) {
                            if (v24->m_field_2D2) {
                                if (!v24->m_field_2D3)
                                    v24->m_camera2 = g_FollowCam;
                                v24->m_field_2D3 = v24->m_field_2D4 = true;
                            }
                        } else {
                            if (!v24->m_field_2D2 || !v24->m_field_2D3)
                                continue;
                            v24->m_field_2D4 = false;
                        }
                        if (!v24->m_field_2D2) {
                            if (g_DesiredCam == g_DollyCam) {
                                v191.push_back(v24);
                                v24->m_camera1->SetLookAtEntity(g_DollyCam->GetLookAtEntity());
                            }
                            continue;
                        }
                        v12 = true;
                        auto c = v24->m_camera1;
                        c->SetLookAtEntity(g_DollyCam->GetLookAtEntity());
                        g_CameraManager.CutToCamera(c, 0.0f);
                        pSK = g_CameraManager.GetSelectedCamera();
                        v24->Update(0.0f /*QUEST: not used or why 0*/);
                        breaked = true;
                        break;
                    }
                }
                if (breaked == false) {
                    v12 = io.m_data[0] != 0.0f || io.m_data[1] != 0.0f;
                    auto v37 = v191.size();
                    if (v37) {
                        auto v38 = (g_CachedWorldTime / 15'000) % v37;
                        v12 = true;
                        g_CameraManager.CutToCamera(v191[v38]->m_camera1, 0.0f);
                        pSK = g_CameraManager.GetSelectedCamera();
                        v191[v38]->Update(0.0f /*QUEST: not used or why 0*/);
                    }
                }
                camChanged = false;
            }
        }
        if (!v12) {
            if (pSK != g_DesiredCam) {
                auto v41 = 0.0f;
                if ((pSK == g_CloseCam && (g_DesiredCam == g_WheelCam || g_DesiredCam == g_SideCam || g_DesiredCam == g_FollowCam)) ||
                    (pSK == g_SideCam && (g_DesiredCam == g_CloseCam || g_DesiredCam == g_FollowCam || g_DesiredCam == g_LeadCam)) ||
                    (pSK == g_FollowCam && (g_DesiredCam == g_CloseCam || g_DesiredCam == g_SideCam))) {
                    AUDIO_Event("Play_SFX_GAM_CameraAngleChange", 1, false);
                    v41 = 0.5f;
                }
                g_CameraManager.CutToCamera(g_DesiredCam, v41);
                pSK = g_CameraManager.GetSelectedCamera();
                camChanged = true;
            }
        }
    }
    if (pSK == g_DollyCam) {
        io.m_data[1] = io.m_data[2] = 0.0f;
        auto v45 = (BikeEntity *)g_DollyCam->GetLookAtEntity();
        if (camChanged || v45) {
            if (camChanged || (g_DollyCam->m_pos - v45->GetPosition()).lenSquared() > 16'000'000.0f) {
                io.m_data[0] = g_DollyCam->m_bikeLocalPos.m_data[0];
                g_DollyCam->BikeLocalToWorldPos(&io);
                g_DollyCam->m_pos = io;
                g_DollyCam->ResetMovement();
                if (v45) {
                    uint32_t v55 = 0;
                    if (v45->m_road)
                        v55 = v45->m_road->m_segmentId;
                    auto v57 = g_pRoadManager->GetRoadSegment(v55);
                    if (v57) {
                        auto pt = v57->FindClosestPointOnRoad(g_DollyCam->m_pos, 0.0, 10);
                        float dir = -1.0f;
                        if (v45->UpdateAnimation(dt))
                            dir = 1.0f;
                        auto v194 = v57->CalculateRoadPositionAtTime(pt, false);
                        auto rw = v57->GetRoadWidth(pt, true, nullptr, nullptr, nullptr);
                        auto maneur = (rw * 0.5f - 80.0f) * dir;
                        VEC3 v196;
                        v57->CalculateRoadPositionByDist(pt, v45->m_field_8B8 ? 1000.0 : -1000.0, &v196);
                        pos = v196 - v194;
                        auto sq = pos.lenSquared();
                        float v77, v79;
                        if (sq >= 1.0f) {
                            auto v85 = 1.0f / sqrtf(sq);
                            v77 = -v85 * pos.m_data[2];
                            v79 = v85 * pos.m_data[0];
                        } else {
                            v57->CalculateRoadPositionByDist(pt, v45->m_field_8B8 ? -1000.0 : 1000.0, &v196);
                            pos = v196 - v194;
                            auto v71 = pos.len();
                            if (v71 == 0.0f) {
                                v77 = pos.m_data[2];
                                v79 = -pos.m_data[0];
                            } else {
                                auto v73 = 1.0f / v71;
                                v77 = v73 * pos.m_data[2];
                                v79 = -v73 * pos.m_data[0];
                            }
                        }
                        g_DollyCam->m_pos.m_data[0] = v77 * maneur + v196.m_data[0];
                        g_DollyCam->m_pos.m_data[1] = v196.m_data[1] + 120.0f;
                        g_DollyCam->m_pos.m_data[2] = v79 * maneur + v196.m_data[2];
                    }
                }
            } else if (camChanged || v45) {
                uint32_t v90 = 0;
                if (v45->m_road)
                    v90 = v45->m_road->m_segmentId;
                auto v91 = g_pRoadManager->GetRoadSegment(v90);
                if (v91) {
                    auto pt = v91->FindClosestPointOnRoad(g_DollyCam->m_pos, 0.0, 10);
                    pos = g_DollyCam->m_pos;
                    float dir = -1.0f;
                    if (v45->UpdateAnimation(dt))
                        dir = 1.0f;
                    auto v194 = v91->CalculateRoadPositionAtTime(pt, false);
                    auto rw = v91->GetRoadWidth(pt, true, nullptr, nullptr, nullptr);
                    auto maneur = (rw * 0.5f - 80.0f) * dir;
                    VEC3 v196;
                    v91->CalculateRoadPositionByDist(pt, v45->m_field_8B8 ? 1000.0 : -1000.0, &v196);
                    pos = v196 - v194;
                    auto sq = pos.lenSquared();
                    float v77, v79;
                    if (sq >= 1.0f) {
                        auto v85 = 1.0f / sqrtf(sq);
                        v77 = -v85 * pos.m_data[2];
                        v79 = v85 * pos.m_data[0];
                    } else {
                        v91->CalculateRoadPositionByDist(pt, v45->m_field_8B8 ? -1000.0 : 1000.0, &v196);
                        pos = v196 - v194;
                        auto v71 = pos.len();
                        if (v71 == 0.0f) {
                            v77 = pos.m_data[2];
                            v79 = -pos.m_data[0];
                        } else {
                            auto v73 = 1.0f / v71;
                            v77 = v73 * pos.m_data[2];
                            v79 = -v73 * pos.m_data[0];
                        }
                    }
                    auto v124_ = VEC3{ v77 * maneur + v196.m_data[0], v196.m_data[1] + 120.0f, v79 * maneur + v196.m_data[2] } - g_DollyCam->m_pos;
                    auto sq124 = v124_.lenSquared();
                    if (sq124 <= 0.0f) {
                        g_DollyCam->m_localVec = {};
                        g_DollyCam->ResetMovement();
                        pos.m_data[0] = v45->m_field_528.m_data[0] * 10.0f;
                        pos.m_data[1] = v45->m_field_528.m_data[1] * 10.0f;
                        pos.m_data[2] = v45->m_field_528.m_data[2] * 10.0f;
                        auto v149 = pos.lenSquared();
                        if (v149 > 0.0f) {
                            auto speed = v45->m_bc->GetSpeed();
                            g_DollyCam->m_pos.m_data[0] += pos.m_data[0] * speed * 14.0f * dt;
                            g_DollyCam->m_pos.m_data[1] += pos.m_data[1] * speed * 14.0f * dt;
                            g_DollyCam->m_pos.m_data[2] += pos.m_data[2] * speed * 14.0f * dt;
                        }
                    } else {
                        sq124 = 1.0f / sqrtf(sq124);
                        auto speed = v45->m_bc->GetSpeed();
                        g_DollyCam->m_localVec = {};
                        g_DollyCam->ResetMovement();
                        g_DollyCam->m_pos.m_data[0] += speed * v124_.m_data[0] * 14.0f * dt * sq124;
                        g_DollyCam->m_pos.m_data[1] += speed * v124_.m_data[1] * 14.0f * dt * sq124;
                        g_DollyCam->m_pos.m_data[2] += speed * v124_.m_data[2] * 14.0f * dt * sq124;
                    }
                }
            }
        }
        if (!v12) {
            g_DollyCam->m_field_80 = g_DollyCam->m_field_84 = JoyY2() * dt + g_DollyCam->m_field_80;
            g_CameraManager.CutToCamera(g_DollyCam, 0.0f);
        }
    } else if (pSK == g_HeliCam) {
        if (camChanged) {
            pos = g_HeliCam->m_bikeLocalPos;
            g_HeliCam->BikeLocalToWorldPos(&pos);
            g_HeliCam->m_pos = pos;
            g_HeliCam->ResetMovement();
            g_HeliCam->m_field_80 = 1.0f;
            g_HeliCam->m_field_84 = 1.0f;
        }
        static float g_accTime2;
        g_accTime2 += dt;
        if (g_accTime2 > 3600.0f)
            g_accTime2 -= 360.0f;
        auto wid = g_pGameWorld->WorldID();
        if (wid == WID_RICHMOND || wid == WID_LONDON || wid == WID_INNSBRUCK || wid == WID_NYC) {
            auto v174 = g_HeliCam->GetLookAtEntity();
            if (v174) {
                auto v175 = v174->GetPosition();
                g_HeliCam->m_pos.m_data[0] = 9000.0f + v175.m_data[0];
                g_HeliCam->m_pos.m_data[1] = 20000.0f + v175.m_data[1];
                g_HeliCam->m_pos.m_data[2] = 5000.0f + v175.m_data[2];
            }
            g_HeliCam->m_field_84 = g_HeliCam->m_field_80 = 4.5f - sinf(g_accTime2 * 0.1f) * 1.5f;
        } else {
            g_HeliCam->m_field_84 = g_HeliCam->m_field_80 = 13.0f - sinf(g_accTime2 * 0.1f) * 12.0f;
        }
        g_HeliCam->m_field_BC = 0.0f;
    } else {
        pSK = g_CameraManager.GetSelectedCamera();
        if (pSK && pSK->GetLookAtEntity()) {
            auto bc = ((BikeEntity *)pSK->GetLookAtEntity())->m_bc;
            g_FollowCam->m_field_80 = g_FollowCam->m_field_84 = bc->GetSpeed() * 0.15534274f + 65.0f;
        }
    }
    if (g_CameraManager.GetSelectedCamera() == g_FreeCam)
        dt = 0.016666668f;
    g_CameraManager.Update(dt);
}
bool g_enableJoystick = true; //QUEST originally false - let's test it
float JoyY2() {
    int cnt;
    if (g_enableJoystick) {
        auto JoystickAxes = glfwGetJoystickAxes(-1, &cnt);
        if (cnt > 4) {
            auto v1 = JoystickAxes[3]; //GLFW_GAMEPAD_AXIS_RIGHT_Y ?
            if (v1 < -0.176f)
                return v1 * 1.2135923f + 0.21359225f;
            if (v1 > 0.176f)
                return (v1 - 0.176f) * 1.2135923f;
        }
    }
    return 0.0f;
}
float JoyTrg1() {
    int cnt;
    if (g_enableJoystick) {
        auto JoystickAxes = glfwGetJoystickAxes(-1, &cnt);
        if (cnt > 4) {
            auto v1 = JoystickAxes[4]; //GLFW_GAMEPAD_AXIS_LEFT_TRIGGER ?
            if (v1 < -0.176f)
                return -v1 * 1.2135923f - 0.21359225f;
            if (v1 > 0.176f)
                return (0.176f - v1) * 1.2135923f;
        }
    }
    return 0.0f;
}
float JoyX2() {
    int cnt;
    if (g_enableJoystick) {
        auto JoystickAxes = glfwGetJoystickAxes(-1, &cnt);
        if (cnt > 4) {
            auto v1 = JoystickAxes[2]; //GLFW_GAMEPAD_AXIS_RIGHT_X ?
            if (v1 < -0.176f)
                return -1.2135923f * (v1 + 0.176f);
        }
    }
    return 0.0f;
}
Camera::Camera(Entity *e, CAMTYPE ty) : m_type(ty) {
    if (e) {
        if (e->m_entityType == Entity::ET_BIKE) {
            m_playerId = 0;
            if (!e->m_field_C98)
                m_playerId = e->m_playerIdTx;
        } else {
            zassert(0);
        }
    }
}
Entity *Camera::GetLookAtEntity() {
    return BikeManager::Instance()->FindBikeWithNetworkID(m_playerId, true);
}
void Camera::SetLookAtEntity(Entity *e) {
    zassert(e->m_entityType == Entity::ET_BIKE);
    auto be = (BikeEntity *)e;
    m_playerId = be->m_playerIdTx;
    m_field_80 = m_field_84;
    if (m_isElastic) {
        m_field_CC = 10.0f;
        auto speed = be->m_bc->GetSpeed();
        m_pos = m_bikeLocalPos;
        BikeLocalToWorldPos(&m_pos);
        m_bikeWorldPos = m_pos;
        m_field_D4 = VEC3{};
        m_field_E0 = VEC3{ speed * be->m_heading.m_cos * 30.0f, speed * be->m_heading.m_heading2 * 30.0f, speed * be->m_heading.m_sin * 30.0f };
    }
    m_field_B8 = 0.0f;
}
void Camera::BikeLocalToWorldPos(VEC3 *io) {
    auto be = BikeManager::Instance()->FindBikeWithNetworkID(m_playerId, true);
    if (be) {
        VEC4 dest{ io->m_data[0], io->m_data[1], io->m_data[2], 1.0f };
        MAT_MulVecXYZW(&dest, dest, be->m_matrix);
        io->m_data[0] = dest.m_data[0];
        io->m_data[2] = dest.m_data[2];
        io->m_data[1] = dest.m_data[1];
    }
}
struct RegionEntityBaseCommon {
    bool IsInRegion(const VEC3 &) {
        //TODO
        return true;
    }
    /*RegionEntityBaseCommon::~RegionEntityBaseCommon()
RegionEntityBaseCommon::UseRadius(void)
RegionEntityBaseCommon::UpdateTransform(void)
RegionEntityBaseCommon::RegionEntityBaseCommon(Entity::EType)
RegionEntityBaseCommon::IsGlobal(void)
RegionEntityBaseCommon::InitializeEnd(void)
RegionEntityBaseCommon::GetRadius(void)
RegionEntityBaseCommon::GetAABB(void)*/
};
struct FogRegionEntity : public RegionEntityBaseCommon {
    int m_field_2B4 = 0; //TODO:enum
    /*FogRegionEntity::~FogRegionEntity()
FogRegionEntity::Update(float)
FogRegionEntity::TransitionTimeOffset(float,float,float &,float)
FogRegionEntity::TransitionRoadWetness(float,float)
FogRegionEntity::StopApplyingTime(float)
FogRegionEntity::StopApplyingRoadWetness(float)
FogRegionEntity::RampUpTimeCommon(float,float,float &,float,std::list<FogRegionEntity*> &,bool &,float)
FogRegionEntity::RampUpRoadWetness(float)
FogRegionEntity::RampUpGlobalTime(float)
FogRegionEntity::RampUpFogOnlyTime(float)
FogRegionEntity::RampUpEffect(std::list<FogRegionEntity*> &,bool &,std::function<bool ()(void)>)
FogRegionEntity::RampDownTimeCommon(float,float &,float,std::list<FogRegionEntity*> &,bool &)
FogRegionEntity::RampDownRoadWetness(float)
FogRegionEntity::RampDownGlobalTime(float)
FogRegionEntity::RampDownFogOnlyTime(float)
FogRegionEntity::RampDownEffect(float &,std::list<FogRegionEntity*> &,bool &,std::function<bool ()(void)>)
FogRegionEntity::InitializeEnd(void)
FogRegionEntity::InitReflection(void)
FogRegionEntity::GetSkyFogOpacity(void)
FogRegionEntity::GetEffectBlend(void)
FogRegionEntity::GetDensityMax(void)
FogRegionEntity::GetDensity(void)
FogRegionEntity::FogRegionEntity(void)
FogRegionEntity::FogColorSaturation(double,VEC4)
FogRegionEntity::ApplyTime(float)
FogRegionEntity::ApplyRoadWetness(float)*/
};
std::list<FogRegionEntity *> g_FogRegionsEffectingCamera;
float g_camUpdX;
int g_seed = 0x7b;
float SimplexNoise::Generate(float, float, float) {
    //TODO
    return 0.0f;
}
void Camera::Update(float dt, float a3) {
    BikeEntity *be;
    m_field_B8 += dt;
    if (m_field_18) {
        if (m_field_C0 && g_DesiredCam == this) {
            be = (BikeEntity *)GetLookAtEntity();
            if (be)
                for (auto r : g_FogRegionsEffectingCamera)
                    if (r->m_field_2B4 == 3)
                        if (r->IsInRegion(be->GetPosition()))
                            g_DesiredCam = g_FollowCam;
        }
        float /*v33[2],*/ v51 = 0.0f, v52 = 0.0f, v55 = 0.0f, v56 = 0.0f, v179, v261 = 0.0f, v262 = 0.0f, v264 = 0.0f, v80;
        m_bikeWorldPos = m_pos;
        m_field_40 = m_field_34;
        auto io = m_pos;
        auto v10 = m_field_34;
        VEC3 wpos = m_field_34;
        switch (this->m_type) {
        case CT_FREE:
#if 0 //TODO
            if ((getJoystickButtons() & 0x8000) != 0) {
                v261 = 84000.0;
            } else if ((getJoystickButtons() & 0x100) != 0) {
                v261 = 30.0;
            } else {
                v261 = 3000.0;
            }
            v263 = 3.0;
            if ((getJoystickButtons() & 0x1000) != 0) {
                v263 = 0.0;
                v261 = 0.0;
            }
            v20 = wpos.m_data[0] - io.m_data[0];
            v21 = wpos.m_data[1] - io.m_data[1];
            v22 = wpos.m_data[2] - io.m_data[2];
            v23 = wpos.m_data[0] - io.m_data[0];
            if ((float)(wpos.m_data[0] - io.m_data[0]) == 0.0 && v21 == 0.0 && v22 == 0.0)
                v23 = 1.0;
            wpos.m_data[0] = v23;
            wpos.m_data[1] = wpos.m_data[1] - io.m_data[1];
            wpos.m_data[2] = wpos.m_data[2] - io.m_data[2];
            v24 = sqrtf((float)((float)(v21 * v21) + (float)(v23 * v23)) + (float)(v22 * v22));
            if (v24 == 0.0) {
                v25 = wpos.m_data[2];
                v26 = wpos.m_data[0];
                LODWORD(v27) = _mm_shuffle_ps(
                    (__m128) * (unsigned __int64 *)wpos.m_data,
                    (__m128) * (unsigned __int64 *)wpos.m_data,
                    85).m128_u32[0];
            } else {
                v26 = v23 * (float)(1.0 / v24);
                v25 = (float)(1.0 / v24) * v22;
                v27 = (float)(1.0 / v24) * v21;
            }
            v266 = v26;
            v265 = v26 * v27;
            v262 = -v25;
            vec2.m_data[0] = -(float)((float)-v25 * v27);
            v272 = (float)((float)-v25 * v25) - (float)(v26 * v26);
            JoyTrg1();
            v28 = -JoyY2();
            v274 = 1065353216i64;
            v275 = 0.0;
            v276 = 0.0;
            v277 = 1065353216i64;
            v278 = 0i64;
            v279 = 1.0;
            v280 = 0i64;
            v281 = 0.0;
            v282 = 1065353216i64;
            v283 = 0.0;
            v284 = 0.0;
            v285 = 1065353216i64;
            v286 = 0i64;
            v287 = 1.0;
            v288 = 0i64;
            v289 = 0.0;
            sub_7FF66CEA3A00(
                (__int64)&v274,
                v26 * v27,
                COERCE_DOUBLE((unsigned __int64)LODWORD(v272)),
                vec2.m_data[0],
                (float)((float)-v26 * dt) * v263);
            sub_7FF66CEA3A00((__int64)&v282, -v25, 0.0, v26, (float)(v28 * dt) * v263);
            if (v20 == 0.0 && v21 == 0.0 && v22 == 0.0)
                v20 = 1.0;
            v29 = (float)((float)((float)(v20 * *(float *)&v274) + (float)(v276 * v21)) + (float)(*(float *)&v278 * v22))
                + *(float *)&v280;
            v30 = (float)((float)((float)(v20 * *((float *)&v274 + 1)) + (float)(*(float *)&v277 * v21))
                + (float)(*((float *)&v278 + 1) * v22))
                + *((float *)&v280 + 1);
            v31 = (float)((float)((float)(v20 * v275) + (float)(*((float *)&v277 + 1) * v21)) + (float)(v279 * v22)) + v281;
            v32 = (float)((float)((float)((float)(v30 * v284) + (float)(v29 * *(float *)&v282))
                + (float)(v31 * *(float *)&v286))
                + *(float *)&v288)
                + io.m_data[0];
            v264 = v32;
            v33[0] = (float)((float)((float)((float)(v30 * *((float *)&v285 + 1)) + (float)(v29 * v283))
                + (float)(v31 * v287))
                + v289)
                + io.m_data[2];
            v34 = (float)((float)((float)((float)(v30 * *(float *)&v285) + (float)(v29 * *((float *)&v282 + 1)))
                + (float)(v31 * *((float *)&v286 + 1)))
                + *((float *)&v288 + 1))
                + io.m_data[1];
            v35 = v32 - io.m_data[0];
            v36 = v33[0] - io.m_data[2];
            v263 = v33[0] - io.m_data[2];
            v37 = v34 - io.m_data[1];
            if (v35 == 0.0 && v37 == 0.0 && v36 == 0.0)
                v35 = 1.0;
            v38 = sqrtf((float)((float)(v35 * v35) + (float)(v37 * v37)) + (float)(v36 * v36));
            if (v38 != 0.0) {
                v35 = v35 * (float)(1.0 / v38);
                v37 = (float)(1.0 / v38) * v37;
                v263 = (float)(1.0 / v38) * v36;
            }
            if (g_enableJoystick
                && (JoystickAxes = glfwGetJoystickAxes(0xFFFFFFFF, (int *)&v271), SLODWORD(v271) > 4)
                && (v40 = *((float *)JoystickAxes + 2), v40 > 0.176)) {
                v41 = (float)(v40 - 0.176) * 1.2135923;
            } else {
                v41 = 0.0;
            }
            v42 = v41 - JoyX2();
            if (g_enableJoystick) {
                v45 = glfwGetJoystickAxes(0xFFFFFFFF, (int *)&v270);
                v43 = -0.0;
                if (SLODWORD(v270) <= 0) {
                    v44 = -0.0;
                } else {
                    v46 = *(float *)v45;
                    if (*(float *)v45 >= -0.176) {
                        if (v46 <= 0.176)
                            v44 = 0.0;
                        else
                            v44 = (float)(v46 - 0.176) * 1.2135923;
                    } else {
                        v44 = (float)(v46 * 1.2135923) + 0.21359225;
                    }
                }
                if (g_enableJoystick) {
                    v47 = glfwGetJoystickAxes(0xFFFFFFFF, (int *)&v267);
                    if ((int)v267 > 1) {
                        v48 = *((float *)v47 + 1);
                        if (v48 >= -0.176) {
                            if (v48 > 0.176)
                                v43 = (float)(v48 - 0.176) * -1.2135923;
                        } else {
                            v43 = -0.21359225 - (float)(v48 * 1.2135923);
                        }
                    }
                }
            } else {
                v43 = -0.0;
                v44 = -0.0;
            }
            v49 = (__m128)LODWORD(v265);
            v50 = (__m128)LODWORD(v272);
            v50 = (float)(v272 * v42) + (float)(v43 * v37);
            v49 = (float)((float)((float)((float)(v265 * v42) + (float)(v262 * v44)) + (float)(v43 * v35)) * v261)
                * dt;
            v51 = v49;
            v51 = v49 + io.m_data[0];
            v50 = (float)(v50 * v261) * dt;
            v264 = v264 + v49;
            v52 = v50;
            v52 = v50 + io.m_data[1];
            wpos.m_data[0] = v264 - this->m_pos.m_data[0];
            v262 = v34 + v50;
            v53 = (float)(v34 + v50) - this->m_pos.m_data[1];
            v54 = (float)((float)((float)((float)(vec2.m_data[0] * v42) + (float)(v266 * v44)) + (float)(v43 * v263))
                * v261)
                * dt;
            v55 = v33[0] + v54;
            wpos.m_data[1] = v53;
            v56 = v54 + io.m_data[2];
            v261 = v55;
            wpos.m_data[2] = v55 - this->m_pos.m_data[2];
            v57 = sqrtf(
                (float)((float)(v53 * v53) + (float)(wpos.m_data[0] * wpos.m_data[0]))
                + (float)(wpos.m_data[2] * wpos.m_data[2]));
            if (v57 == 0.0)
            {
                v58 = *(_QWORD *)wpos.m_data;
                v64 = wpos.m_data[2];
                *(_QWORD *)this->m_field_8.m_data = v58;
                this->m_field_8.m_data[2] = v64;
            } else {
                v59 = 1.0 / v57;
                v60 = (float)(1.0 / v57) * wpos.m_data[0];
                v61 = (float)(1.0 / v57) * wpos.m_data[2];
                wpos.m_data[2] = v61;
                v62 = (__m128) * (unsigned __int64 *)wpos.m_data;
                v62 = v60;
                v63 = _mm_shuffle_ps(v62, v62, 225);
                v63 = v59 * v53;
                v58 = _mm_shuffle_ps(v63, v63, 225).m128_u64[0];
                v64 = wpos.m_data[2];
                *(_QWORD *)this->m_field_8.m_data = v58;
                this->m_field_8.m_data[2] = v64;
            }
#endif
            break;
        case CT_BIKE:
            UpdateFreeRideCamera(dt);
            io = m_bikeLocalPos;
            wpos = m_field_74;
            if (m_field_C8 && SteeringModule::Self()->ShouldUseDroneCam()) {
                RoadLocalToWorldPos(&io);
                RoadLocalToWorldPos(&wpos);
            } else {
                BikeLocalToWorldPos(&io);
                BikeLocalToWorldPos(&wpos);
            }
            if (m_isElastic) {
                auto v71 = 1.0f - a3;
                v51 = v71 * m_field_EC.m_data[0] + a3 * io.m_data[0];
                v52 = v71 * m_field_EC.m_data[1] + a3 * io.m_data[1];
                v56 = v71 * m_field_EC.m_data[2] + a3 * io.m_data[2];
            } else {
                v51 = io.m_data[0];
                v52 = io.m_data[1];
                v56 = io.m_data[2];
            }
            zassert(g_pRoadManager && HeightMapManager::GetInst());
            be = (BikeEntity *)g_FollowCam->GetLookAtEntity();
            if (be && g_pRoadManager && HeightMapManager::GetInst()) {
                auto v73 = be->m_road;
                int segment = v73 ? v73->m_segmentId : 0;
                auto v76 = g_pRoadManager->GetRoadSegment(segment);
                if (g_SideCam == this) {
                    if (!v76 || v76->IsPlaceholder())
                        v52 = HeightMapManager::GetInst()->GetHeightAtLocation(VEC2{ be->m_pos.m_data[0], be->m_pos.m_data[2] }) + 90.0f;
                    else
                        v52 = v76->GetCyclistAltitudeAtRoadTime(be->m_pos, be->m_field_888, be->m_field_8B8) + 90.0f;
                }
                auto v79 = -be->m_field_500 * 200.0f;
                v80 = wpos.m_data[1] + v79;
                v52 += be->m_field_500 * 100.0f;
                v262 = v80;
                if (v76 && !v76->IsPlaceholder() && be->UpdateAnimation(v79))
                    m_bikeLocalPos.m_data[2] = -fabs(m_bikeLocalPos.m_data[2]);
                else
                    m_bikeLocalPos.m_data[2] = fabs(m_bikeLocalPos.m_data[2]);
            } else {
                v80 = wpos.m_data[1];
                v262 = wpos.m_data[1];
            }
            wpos.m_data[1] = v80 - v52;
            v261 = v55 = wpos.m_data[2];
            v264 = wpos.m_data[0];
            wpos.m_data[0] -= v51;
            wpos.m_data[2] -= v56;
            if (wpos.Empty())
                wpos.m_data[0] = 1.0f;
            m_field_8 = wpos.Normalized();
            break;
        case CT_BIGBRO:
            v262 = wpos.m_data[1];
            v51 = io.m_data[0] + dt * m_worldVec.m_data[0];
            v52 = io.m_data[1] + dt * m_worldVec.m_data[1];
            v56 = io.m_data[2] + dt * m_worldVec.m_data[2];
            wpos = m_field_74;
            BikeLocalToWorldPos(&wpos);
            v55 = wpos.m_data[2];
            v261 = wpos.m_data[2];
            v264 = wpos.m_data[0];
            wpos.m_data[0] -= v51;
            wpos.m_data[1] -= v52;
            wpos.m_data[2] -= v56;
            if (wpos.Empty())
                wpos.m_data[0] = 1.0f;
            m_field_8 = wpos.Normalized();
            break;
        case CT_TITLE:
            v55 = m_field_4C.m_data[2];
            wpos = m_field_4C;
            v264 = wpos.m_data[0];
            wpos.m_data[2] = v55 - m_pos.m_data[2];
            v262 = wpos.m_data[1];
            v261 = v55;
            wpos.m_data[1] = wpos.m_data[1] - m_pos.m_data[1];
            m_field_8 = wpos.Normalized();
            v51 = io.m_data[0];
            v52 = io.m_data[1];
            v56 = io.m_data[2];
            break;
        case CT_ORBIT:
#if 0 //TODO
            if (this->m_bCut)
            {
                this->m_bCut = 0;
                vec2 = (VEC2)&g_CameraManager.m_cameras;
                Myoff = g_CameraManager.m_cameras.Myoff;
                v103 = *(_QWORD *)&g_CameraManager.m_cameras.Map[((g_CameraManager.m_cameras.Myoff
                    + (unsigned __int64)(unsigned int)g_CameraManager.m_prevSelIdx) >> 1) & (g_CameraManager.m_cameras.Mapsize - 1)]->field_0[8 * ((LODWORD(g_CameraManager.m_cameras.Myoff) + g_CameraManager.m_prevSelIdx) & 1)];
                *(_QWORD *)this->m_pos.m_data = *(_QWORD *)(v103 + 28);
                v104 = *(float *)(v103 + 36);
                this->m_pos.m_data[2] = v104;
                v105 = *(_QWORD *)this->m_pos.m_data;
                *(_QWORD *)this->m_bikeWorldPos.m_data = v105;
                *(_QWORD *)io.m_data = v105;
                v106 = *(_QWORD *)this->m_field_74.m_data;
                io.m_data[2] = v104;
                this->m_bikeWorldPos.m_data[2] = v104;
                v107 = this->m_field_74.m_data[2];
                *(_QWORD *)this->m_field_34.m_data = v106;
                this->m_field_34.m_data[2] = v107;
                Camera::BikeLocalToWorldPos(this, &this->m_field_34);
                *(float *)&v106 = this->m_field_34.m_data[2] - this->m_pos.m_data[2];
                *(float *)&v106 = sqrtf(
                    (float)((float)((float)(this->m_field_34.m_data[1] - this->m_pos.m_data[1])
                        * (float)(this->m_field_34.m_data[1] - this->m_pos.m_data[1]))
                        + (float)((float)(this->m_field_34.m_data[0] - this->m_pos.m_data[0])
                            * (float)(this->m_field_34.m_data[0] - this->m_pos.m_data[0])))
                    + (float)(*(float *)&v106 * *(float *)&v106));
                LODWORD(this->m_bikeLocalPos.m_data[0]) = v106;
                if (*(float *)&v106 > 2000.0
                    && (WorldID(g_pGameWorld) == WID_RICHMOND || WorldID(g_pGameWorld) == WID_LONDON))
                {
                    this->m_bikeLocalPos.m_data[0] = 2000.0;
                }
            }
            v108 = this->m_field_74.m_data[2];
            *(_QWORD *)wpos.m_data = *(_QWORD *)this->m_field_74.m_data;
            wpos.m_data[2] = v108;
            Camera::BikeLocalToWorldPos(this, &wpos);
            v109 = this->m_field_34.m_data[1] - this->m_pos.m_data[1];
            v110 = this->m_field_34.m_data[2] - this->m_pos.m_data[2];
            v111 = this->m_field_34.m_data[0];
            v112 = v111 - this->m_pos.m_data[0];
            vec2.m_data[1] = v109;
            vec2.m_data[0] = v112;
            *(float *)&Myoff = v110;
            v113 = sqrtf((float)((float)(v109 * v109) + (float)(v112 * v112)) + (float)(v110 * v110));
            if (v113 == 0.0) {
                v114 = (__m128)(unsigned int)Myoff;
                v115 = (__m128)LODWORD(vec2.m_data[0]);
                LODWORD(v116) = _mm_shuffle_ps(
                    (__m128) * (unsigned __int64 *)&vec2,
                    (__m128) * (unsigned __int64 *)&vec2,
                    85).m128_u32[0];
            } else {
                v114 = 1.0f / v113;
                v115 = v114;
                v115 = (float)(1.0 / v113) * v112;
                v116 = (float)(1.0 / v113) * v109;
                v114 = (float)(1.0 / v113) * v110;
            }
            v117 = wpos.m_data[0];
            v118 = -v114;
            v264 = wpos.m_data[0];
            v265 = v118;
            v119 = v118;
            v119 = v118 * v116;
            v120 = v115;
            v120 = v115 * v116;
            v121 = -v119;
            v122 = wpos.m_data[1];
            v123 = wpos.m_data[0] - v111;
            v55 = wpos.m_data[2];
            v263 = (float)(v118 * v114) - (float)(v115 * v115);
            v262 = wpos.m_data[1];
            v261 = wpos.m_data[2];
            *(float *)&v267 = v123;
            v271 = wpos.m_data[1] - this->m_field_34.m_data[1];
            v266 = wpos.m_data[2] - this->m_field_34.m_data[2];
            vec2.m_data[0] = JoyY2();
            v270 = (float)(vec2.m_data[0]
                * (float)((float)(v118 * v114)
                    - (float)(v115 * v115)))
                * 2500.0;
            v124 = JoyTrg1();
            v125 = dword_7FF66E6E7080;
            v126 = 0;
            v127 = dword_7FF66E6E7080;
            v128 = 0;
            v129 = (float)((float)(vec2.m_data[0] * v121) + (float)((float)-v124 * v115)) * 2500.0f;
            vec2.m_data[0] = (float)((float)(vec2.m_data[0] * (float)(v115 * v116)) + (float)((float)-v124 * v265)) * 2500.0f;
        while (1) {
            v130 = dword_7FF66E6E984C;
            if (*v127 == 263)
                break;
            ++v128;
            ++v127;
            if (v128 >= 3)
                goto LABEL_129;
        }
        if (dword_7FF66E6E984C > -1.0f) {
            v134 = dt + dt;
            v130 = fmaxf(dword_7FF66E6E984C - (float)(dt + dt), -1.0);
        LABEL_136:
            dword_7FF66E6E984C = v130;
            goto LABEL_137;
        }
    LABEL_129:
        v131 = 0;
        v132 = dword_7FF66E6E7080;
        while (*v132 != 262) {
            ++v131;
            ++v132;
            if (v131 >= 3)
                goto LABEL_132;
        }
        if (dword_7FF66E6E984C < 1.0f) {
            v134 = dt + dt;
            v130 = fminf((float)(dt + dt) + dword_7FF66E6E984C, 1.0);
            goto LABEL_136;
        }
    LABEL_132:
        v133 = dword_7FF66E6E984C <= 0.0;
        v134 = dt + dt;
        if (dword_7FF66E6E984C < 0.0) {
            v130 = dword_7FF66E6E984C + fminf(-dword_7FF66E6E984C, v134);
            dword_7FF66E6E984C = v130;
            v133 = v130 <= 0.0;
        }
        if (!v133) {
            v130 = v130 - fminf(v134, v130);
            goto LABEL_136;
        }
    LABEL_137:
        v135 = io.m_data[1];
        v136 = io.m_data[1] - v122;
        v137 = v115 * v130;
        v265 = v265 * v130;
        wpos.m_data[1] = io.m_data[1] - v122;
        wpos.m_data[2] = io.m_data[2] - v55;
        wpos.m_data[0] = io.m_data[0] - v117;
        v138 = sqrtf(
            (float)((float)(v136 * v136) + (float)(wpos.m_data[0] * wpos.m_data[0]))
            + (float)(wpos.m_data[2] * wpos.m_data[2]));
        if (v138 == 0.0)
            LODWORD(v139) = _mm_shuffle_ps(
                (__m128) * (unsigned __int64 *)wpos.m_data,
                (__m128) * (unsigned __int64 *)wpos.m_data,
                85).m128_u32[0];
        else
            v139 = v136 / v138;
        v140 = 0;
        v141 = dword_7FF66E6E7080;
        while (*v141 != 265) {
            ++v140;
            ++v141;
            if (v140 >= 3)
                goto LABEL_147;
        }
        if (v139 < 0.80000001) {
            v145 = fmaxf(dword_7FF66E6E9848 - v134, -1.0);
        LABEL_156:
            dword_7FF66E6E9848 = v145;
            goto LABEL_157;
        }
    LABEL_147:
        v142 = 0;
        v143 = dword_7FF66E6E7080;
        while (*v143 != 264) {
            ++v142;
            ++v143;
            if (v142 >= 3)
                goto LABEL_150;
        }
        if (v139 > 0.1f) {
            v145 = fminf(v134 + dword_7FF66E6E9848, 1.0);
            goto LABEL_156;
        }
    LABEL_150:
        v144 = 2.0f;
        if (v139 > 0.80000001)
            v144 = 5.0;
        v145 = dword_7FF66E6E9848;
        v146 = dword_7FF66E6E9848 <= 0.0;
        v147 = v144 * dt;
        if (dword_7FF66E6E9848 < 0.0f) {
            v145 = dword_7FF66E6E9848 + fminf(-dword_7FF66E6E9848, v147);
            dword_7FF66E6E9848 = v145;
            v146 = v145 <= 0.0;
        }
        if (!v146) {
            v145 = v145 - fminf(v147, v145);
            goto LABEL_156;
        }
    LABEL_157:
        v148 = 0;
        v149 = dword_7FF66E6E7080;
        v150 = (float)((float)((float)((float)(v263 * v145) * 1250.0) + v270) * dt) + v271;
        v120 = (float)((float)((float)((float)((float)(v120 * v145) + v265) * 1250.0)
            + vec2.m_data[0])
            * dt)
            + *(float *)&v267;
        v121 = (float)((float)((float)((float)((float)(v121 * v145) + v137) * 1250.0) + v129) * dt)
            + v266;
        v263 = v150;
        while (*v149 != 61) {
            ++v148;
            ++v149;
            if (v148 >= 3) {
                v151 = 0;
                v152 = dword_7FF66E6E7080;
                while (*v152 != 334) {
                    ++v151;
                    ++v152;
                    if (v151 >= 3)
                        goto LABEL_163;
                }
                break;
            }
        }
        if (this->m_bikeLocalPos.m_data[0] <= 150.0f) {
        LABEL_163:
            v153 = 0;
            v154 = dword_7FF66E6E7080;
            while (*v154 != 45) {
                ++v153;
                ++v154;
                if (v153 >= 3) {
                    while (*v125 != 333) {
                        ++v126;
                        ++v125;
                        if (v126 >= 3)
                            goto LABEL_168;
                    }
                    break;
                }
            }
            if (this->m_bikeLocalPos.m_data[0] >= 1300.0f) {
            LABEL_168:
                v155 = g_camUpdX;
                v156 = g_camUpdX <= 0.0;
                v157 = dt * 2.5;
                if (g_camUpdX < 0.0f) {
                    v155 = g_camUpdX + fminf(-g_camUpdX, v157);
                    g_camUpdX = v155;
                    v156 = v155 <= 0.0;
                }
                if (!v156) {
                    v155 = v155 - fminf(v157, v155);
                    g_camUpdX = v155;
                }
                v150 = v263;
            } else {
                v155 = fminf((float)(dt * 4.0) + g_camUpdX, 1.0);
                g_camUpdX = v155;
            }
        } else {
            v155 = fmaxf(g_camUpdX - (float)(dt * 4.0), -1.0);
            g_camUpdX = v155;
        }
        v158 = (float)((float)(v155 * 500.0) * dt) + this->m_bikeLocalPos.m_data[0];
        this->m_bikeLocalPos.m_data[0] = v158;
        if (v158 <= 2500.0) {
            if (v158 < 150.0)
                this->m_bikeLocalPos.m_data[0] = 150.0;
        } else {
            this->m_bikeLocalPos.m_data[0] = 2500.0;
        }
        v120 = v120 + io.m_data[0];
        v121 = v121 + io.m_data[2];
        v159 = v135 + v150;
        Inst = HeightMapManager::GetInst();
        v161 = HeightMapManager::GetHeightAtLocation(Inst, (VEC2) * (_OWORD *)&_mm_unpacklo_ps(v120, v121)) + 50.0;
        if (v159 < v161)
            v159 = v161;
        v162 = v120 - v117;
        v163 = v159 - v122;
        v164 = v121 - v55;
        wpos.m_data[0] = v162;
        wpos.m_data[1] = v163;
        wpos.m_data[2] = v164;
        v165 = sqrtf((float)((float)(v162 * v162) + (float)(v163 * v163)) + (float)(v164 * v164));
        if (v165 == 0.0f) {
            v166 = wpos.m_data[2];
            v167 = wpos.m_data[0];
            vec2 = (VEC2)_mm_shuffle_ps(
                (__m128) * (unsigned __int64 *)wpos.m_data,
                (__m128) * (unsigned __int64 *)wpos.m_data,
                85).m128_u64[0];
        } else {
            v167 = v162 * (float)(1.0 / v165);
            vec2.m_data[0] = v163 * (float)(1.0 / v165);
            v166 = v164 * (float)(1.0 / v165);
        }
        v168 = (float)((getJoystickButtons() & 0x200) != 0 ? 0x12C : 0);
        JoystickButtons = getJoystickButtons();
        v170 = v264;
        v171 = (__m128)COERCE_UNSIGNED_INT((float)(int)((JoystickButtons & 0x100) != 0 ? 0xFFFFFED4 : 0));
        v171 = (float)((float)(v171 + v168) * dt) + this->m_bikeLocalPos.m_data[0];
        v51 = v171;
        LODWORD(this->m_bikeLocalPos.m_data[0]) = v171.m128_i32[0];
        v51 = v171 * v167;
        v52 = v171;
        v51 = v51 + v170;
        v52 = (float)(v171 * vec2.m_data[0]) + v122;
        v56 = (float)(v171 * v166) + v55;
        v172 = v170 - v51;
        v173 = v122 - v52;
        v174 = v55 - v56;
        if ((float)(v170 - v51) == 0.0 && v173 == 0.0 && v174 == 0.0)
            v172 = 1.0;
        wpos.m_data[0] = v172;
        wpos.m_data[1] = v173;
        wpos.m_data[2] = v55 - v56;
        v175 = sqrtf((float)((float)(v172 * v172) + (float)(v173 * v173)) + (float)(v174 * v174));
        if (v175 == 0.0f) {
            v58 = *(_QWORD *)wpos.m_data;
        } else {
            v176 = (__m128) * (unsigned __int64 *)wpos.m_data;
            v176 = v172 * (float)(1.0 / v175);
            v63 = _mm_shuffle_ps(v176, v176, 225);
            v63 = v173 * (float)(1.0 / v175);
            wpos.m_data[2] = v174 * (float)(1.0 / v175);
            v58 = _mm_shuffle_ps(v63, v63, 225).m128_u64[0];
        }
        v64 = wpos.m_data[2];
        *(_QWORD *)this->m_field_8.m_data = v58;
        this->m_field_8.m_data[2] = v64;
#endif
            break;
        case CT_8:
            if (!BikeManager::Instance()->FindBikeWithNetworkID(m_playerId, true) || !m_field_5C)
                return;
            m_field_34 = m_field_74;
            BikeLocalToWorldPos(&m_field_34);
            v262 = wpos.m_data[1];
            v264 = wpos.m_data[0];
            v55 = wpos.m_data[2];
            v261 = wpos.m_data[2];
            v51 = io.m_data[0];
            v52 = io.m_data[1];
            v56 = io.m_data[2];
            break;
        default:
            v264 = wpos.m_data[0];
            v262 = wpos.m_data[1];
            v55 = wpos.m_data[2];
            v261 = wpos.m_data[2];
            v51 = io.m_data[0];
            v52 = io.m_data[1];
            v56 = io.m_data[2];
            break;
        }
        if (m_field_BC != 0.0f)
            m_field_80 = (m_field_BC * dt) + m_field_80;
        v51 -= m_bikeWorldPos.m_data[0];
        v52 -= m_bikeWorldPos.m_data[1];
        v179 = v56 - m_bikeWorldPos.m_data[2];
        be = BikeManager::Instance()->FindBikeWithNetworkID(m_playerId, true);
        float speed = 0.0f;
        bool nv187 = false;
        if (be) {
            speed = be->m_bc->GetSpeed();
            if (speed >= 0.1f)
                nv187 = true;
        }
        if (m_field_C4 < 0.1f || nv187) {
            if (m_isElastic && v52 * v52 + v51 * v51 + v179 * v179 <= 4'000'000.0f && be && be->m_entityType == 1 && nv187) {
                auto v189 = -be->m_heading.m_cos, v190 = -be->m_heading.m_heading2, v191 = -be->m_heading.m_sin;
                auto v193 = speed - m_field_C4;
                auto v194 = v52 * v190 + v51 * v189 + v179 * v191;
                auto v195 = v194 * v190;
                auto v197 = v194 * v191;
                if (fabs(v193) <= fminf(m_field_C4 * 0.04f, 1.0f))
                    v193 = -0.5f;
                float vec2 = m_field_D0 - (m_field_104 - m_field_100) * v193 * dt * 0.1f, v213, v215, v216, v217, v219, v220, v221;
                m_field_C4 = (1.0f - dt) * m_field_C4 + speed * dt;
                auto p_vec2 = &vec2;
                if (vec2 <= m_field_100)
                    p_vec2 = &m_field_100;
                if (m_field_104 > *p_vec2)
                    m_field_D0 = *p_vec2;
                else
                    m_field_D0 = m_field_104;
                if (fabs(m_field_C4 - speed) < 0.1f)
                    m_field_C4 = speed;
                auto v206 = m_field_D0 * v195;
                auto v209 = (v51 - v194 * v189) * 20.0f;
                auto v210 = (v52 - v195) * 20.0f;
                auto v211 = (v179 - v197) * 20.0f;
                auto v212 = m_field_D0 * v197;
                if (m_field_CC < 0.0f) {
                    v51 = v209 - m_field_F8 * m_field_D4.m_data[0];
                    v52 = v210 - (m_field_F8 * m_field_D4.m_data[1]);
                    v219 = m_field_FC * m_field_E0.m_data[0];
                    v221 = v211 - (m_field_F8 * m_field_D4.m_data[2]);
                    v206 -= m_field_FC * m_field_E0.m_data[1];
                    v220 = v212 - (m_field_FC * m_field_E0.m_data[2]);
                } else {
                    v206 = m_field_D0 * v195;
                    v213 = 1.0f - m_field_CC * 0.1f;
                    v215 = m_field_F8 + m_field_F8 - (m_field_F8 * v213);
                    v216 = m_field_E0.m_data[1];
                    v217 = 2.0f * m_field_FC - v213 * m_field_FC;
                    m_field_CC -= dt;
                    v206 -= v216 * v217;
                    v51 = v209 - (v215 * m_field_D4.m_data[0]);
                    v219 = v217 * m_field_E0.m_data[0];
                    v52 = v210 - (v215 * m_field_D4.m_data[1]);
                    v220 = v212 - (v217 * m_field_E0.m_data[2]);
                    v221 = v211 - (v215 * m_field_D4.m_data[2]);
                }
                m_field_D4.m_data[0] += v51 * dt;
                m_field_D4.m_data[1] += v52 * dt;
                m_field_D4.m_data[2] += v221 * dt;
                m_field_E0.m_data[0] += (m_field_D0 * v194 * v189 - v219) * dt;
                m_field_E0.m_data[1] += v206 * dt;
                m_field_E0.m_data[2] += v220 * dt;
                m_pos.m_data[0] = (m_field_E0.m_data[0] + m_field_D4.m_data[0]) * 0.5f * dt + m_bikeWorldPos.m_data[0];
                m_pos.m_data[1] = (m_field_E0.m_data[1] + m_field_D4.m_data[1]) * 0.5f * dt + m_bikeWorldPos.m_data[1];
                m_pos.m_data[2] = (m_field_E0.m_data[2] + m_field_D4.m_data[2]) * 0.5f * dt + m_bikeWorldPos.m_data[2];
            } else {
                m_pos.m_data[0] = v51 + m_bikeWorldPos.m_data[0];
                m_pos.m_data[1] = v52 + m_bikeWorldPos.m_data[1];
                m_pos.m_data[2] = v179 + m_bikeWorldPos.m_data[2];
            }
            auto v233 = m_field_B0 * 25.0f; // Camera::ApplyShake inlined
            auto v235 = v264 * m_field_B4 * 0.002f;
            auto v236 = v56 * m_field_B4 * 0.002f;
            auto v238 = SimplexNoise::Generate(v235, v236, m_field_B4 * v262 * 0.002f) * v233 + v262;
            auto v240 = SimplexNoise::Generate(m_field_B4 * v238 * 0.002f, v235, v236) * v233 + v264;
            auto v243 = v236 * v233 + v261;
            auto c2 = cosf(m_field_B8 * 0.023f + 1.0f) * 10.0f * m_field_B4;
            auto sn = sinf(m_field_B8 * 0.024f) * 10.0f * m_field_B4;
            auto cs = cosf(m_field_B8 * 0.02f) * 10.0f * m_field_B4;
            auto v246 = SimplexNoise::Generate(cs, sn, c2);
            auto v247 = SimplexNoise::Generate(c2, cs, sn);
            auto v242 = SimplexNoise::Generate(sn, c2, cs);
            m_field_34.m_data[0] = (v247 * v233 + v240 - m_field_40.m_data[0]) * m_field_AC + m_field_34.m_data[0];
            m_field_34.m_data[1] = (v246 * v233 + v238 - m_field_40.m_data[1]) * m_field_AC + m_field_34.m_data[1];
            m_field_34.m_data[2] = (v242 * v233 + v243 - m_field_40.m_data[2]) * m_field_AC + m_field_34.m_data[2];
            if (m_field_10C > 0.0f) { // Camera::ApplyCameraShake inlined
                m_field_10C -= dt;
                int v251 = 214013 * g_seed + 2531011;
                g_seed = 214013 * v251 + 2531011;
                int v253 = HIWORD(g_seed);
                int v254 = 214013 * g_seed + 2531011;
                int v255 = 214013 * v254 + 2531011;
                int v257 = 214013 * v255 + 2531011;
                m_pos.m_data[0] += ((HIWORD(v251) & 0x7FFF) * m_field_108 * 0.000061037019f - m_field_108) * (int(v253 & 0x80000001) * 2 - 1.0f);
                m_pos.m_data[1] += ((HIWORD(v254) & 0x7FFF) * m_field_108 * 0.000061037019f - m_field_108) * (((HIWORD(v255) & 0x80000001) * 2) - 1.0f);
                m_pos.m_data[2] += ((HIWORD(v257) & 0x7FFF) * m_field_108 * 0.000061037019f - m_field_108) * ((((214013 * v257 + 2531011) >> 16) & 0x80000001) * 2 - 1.0f);
                g_seed = 214013 * v257 + 2531011;
            }
        }
    }
}
void Camera::UpdateFreeRideCamera(float dt) {
    if (this != g_FollowCam)
        return;
    if (m_field_110 > 0.0f) {
        m_accTime += dt;
        auto v5 = m_accTime / m_field_110;
        if (v5 < 1.0f) {
            m_bikeLocalPos.m_data[0] = (1.0f - v5) * m_field_118.m_data[0] + v5 * m_field_130.m_data[0];
            m_bikeLocalPos.m_data[1] = (1.0f - v5) * m_field_118.m_data[1] + v5 * m_field_130.m_data[1];
            m_bikeLocalPos.m_data[2] = (1.0f - v5) * m_field_118.m_data[2] + v5 * m_field_130.m_data[2];
            m_field_74.m_data[0] = (1.0f - v5) * m_field_124.m_data[0] + v5 * m_field_13C.m_data[0];
            m_field_74.m_data[1] = (1.0f - v5) * m_field_124.m_data[1] + v5 * m_field_13C.m_data[1];
            m_field_74.m_data[2] = (1.0f - v5) * m_field_124.m_data[2] + v5 * m_field_13C.m_data[2];
        } else {
            m_field_74 = m_field_13C;
            m_bikeLocalPos = m_field_130;
            m_field_110 = 0.0f;
            m_accTime = 0.0f;
            m_field_130 = m_field_13C = VEC3{};
        }
    }
    auto mainBike = BikeManager::g_pBikeManager->m_mainBike;
    RoadSegment *road = nullptr;
    int v16 = 0;
    if (mainBike) {
        if (g_pRoadManager) {
            road = mainBike->m_road;
            if (!road)
                road = g_pRoadManager->GetRoadSegment(0);
        }
        if (g_pGameWorld && road && road->m_segmentId == 96 && g_pGameWorld->WorldID() == WID_WATOPIA) {
            auto v15 = mainBike->m_field_888;
            if (v15 < 0.94f && v15 > 0.85f)
                v16 = 1;
            if ((v15 < 0.83f && v15 > 0.79f) || (v15 < 0.6f && v15 > 0.48f))
                v16 = 1;
            if ((v15 < 0.7f && v15 > 0.61f) || (v15 < 0.37f && v15 > 0.25f) || (v15 < 0.235f && v15 > 0.07f))
                v16 = 2;
        }
    }
    if (v16 == m_field_148 || m_field_110 > 0.0f)
        return;
    m_field_148 = v16;
    switch (v16) {
    case 0:
        m_field_130.m_data[0] = -150.0f;
        m_field_130.m_data[1] = 200.0f;
        m_field_13C.m_data[0] = 200.0f;
        m_field_13C.m_data[1] = 150.0f;
        break;
    case 1:
        m_field_130.m_data[0] = -100.0f;
        m_field_130.m_data[1] = 300.0f;
        m_field_13C.m_data[0] = 300.0f;
        m_field_13C.m_data[1] = 75.0f;
        break;
    case 2:
        m_field_130.m_data[0] = -120.0f;
        m_field_130.m_data[1] = 170.0f;
        m_field_13C.m_data[0] = 200.0f;
        m_field_13C.m_data[1] = 190.0f;
        break;
    }
    m_field_130.m_data[2] = 0.0f;
    m_field_13C.m_data[2] = 0.0f;
    m_field_118 = m_bikeLocalPos;
    m_field_124 = m_field_74;
    m_field_110 = 3.0f;
}
void Camera::ResetMovement() {
    m_worldVec = m_localVec;
    auto be = BikeManager::Instance()->FindBikeWithNetworkID(m_playerId, true);// Camera::LocalToWorldVec inlined
    if (be) {
        VEC4 dest{ m_worldVec.m_data[0], m_worldVec.m_data[1], m_worldVec.m_data[2], 1.0f };
        MAT_MulVecXYZW(&dest, dest, be->m_matrix);
        VEC4 v11{ g_cident3 };
        MAT_MulVecXYZW(&v11, v11, be->m_matrix);
        m_worldVec.m_data[0] = dest.m_data[0] - v11.m_data[0];
        m_worldVec.m_data[1] = dest.m_data[1] - v11.m_data[1];
        m_worldVec.m_data[2] = dest.m_data[2] - v11.m_data[2];
    }
}
void Camera::RoadLocalToWorldPos(VEC3 *io) {
    auto be = BikeManager::Instance()->FindBikeWithNetworkID(m_playerId, true);
    if (be && g_pRoadManager) {
        auto road = be->m_road;
        if (!road)
            road = g_pRoadManager->GetRoadSegment(0);
        if (road) {
            auto v16 = road->CalculateRoadPositionAtTimeF(be->m_field_888, false);
            auto v7 = road->GetRoadWidth(be->m_field_888, false, nullptr, nullptr, nullptr) * 0.5f * (1.0f - road->GetRiderBoundsRatio());
            VEC3 v15 = be->m_field_8B8 ? be->m_field_528 : -be->m_field_528;
            auto v8 = -v15.m_data[2], v9 = io->m_data[2];
            auto v10 = v7 * v8 * 0.5f + v16.m_data[0] + v15.m_data[0] * io->m_data[0] - v15.m_data[0] * v15.m_data[1] * io->m_data[1] + v9 * v8;
            auto v12 = v7 * v15.m_data[0] * 0.5f + v16.m_data[2] + v15.m_data[2] * io->m_data[0] + v8 * v15.m_data[1] * io->m_data[1] + v9 * v15.m_data[0];
            auto v13 = (be->m_pos.m_data[2] - v12) * v15.m_data[0] + (be->m_pos.m_data[0] - v10) * v8;
            io->m_data[0] = (v8 * v13) + v10;
            io->m_data[1] = (v15.m_data[0] * v15.m_data[0] - v8 * v15.m_data[2]) * io->m_data[1] + v15.m_data[1] * io->m_data[0] + v16.m_data[1];
            io->m_data[2] = (v15.m_data[0] * v13) + v12;
        }
    }
}
