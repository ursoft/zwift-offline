#include "ZwiftApp.h" //READY for testing
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
float g_camUpdX;
const uint8_t SimplexNoise::perm[512] = { 151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30,
 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177,
 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122,
 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126,
 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153,
 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193,
 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121,
 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180, 151, 160, 137, 91,
 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120,
 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175,
 74, 165, 71, 134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244, 102,
 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198,
 173, 186, 3, 64, 52, 217, 226, 250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212,207, 206, 59, 227, 47, 16, 58, 17, 182, 189,
 28, 42, 223, 183, 170, 213, 119, 248, 152, 2,44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 
 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228, 251, 34, 242, 193, 238, 210, 144, 12,191, 179, 162, 241, 81, 51, 145, 235, 249, 14,
 239, 107, 49, 192, 214, 31, 181, 199, 106, 157,184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254, 138, 236, 205, 93, 222, 114, 67, 29,
 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };
float SimplexNoise::Generate(float a2, float a3, float a4) {
    float v4 = (a2 + a3 + a4) * 0.33333f, v5 = v4 + a2, v6 = v4 + a3, v8 = v4 + a4;
    int v11 = int(v5) - (v5 <= 0.0), v12 = int(v6) - (v6 <= 0.0f), v13 = int(v8) - (v8 <= 0.0f);
    float v14 = (v11 + v12 + v13) * 0.16667f, v38 = 0.0f, v40 = 0.0f, v44, v54, v55;
    float v15 = a2 - float(v11) + v14, v16 = a3 - float(v12) + v14, v17 = a4 - float(v13) + v14;
    bool v18, v20, v21, v23, v22, v19;
    if (v15 >= v16) {
        if (v16 >= v17) {
            v19 = v21 = v23 = false;
            v22 = v18 = v20 = true;
        } else {
            v19 = v22 = false;
            v23 = v15 < v17;
            v20 = v15 >= v17;
            v18 = v21 = true;
        }
    } else if (v16 >= v17) {
        v23 = v20 = false;
        v22 = v19 = true;
        v21 = v15 < v17;
        v18 = v15 >= v17;
    } else {
        v18 = v20 = v19 = false;
        v21 = v23 = v22 = true;
    }
    uint32_t v25 = v11 - ((v11 >= 0 ? v11 : v11 + 255) & 0xFFFFFF00);
    uint32_t v27 = v12 - ((v12 >= 0 ? v12 : v12 + 255) & 0xFFFFFF00);
    uint32_t v29 = v13 - ((v13 >= 0 ? v13 : v13 + 255) & 0xFFFFFF00);
    int v30 = ((v25 >> 23) & 0x100) + v25;
    int v31 = ((v27 >> 23) & 0x100) + v27;
    int v32 = ((v29 >> 23) & 0x100) + v29;
    float v33 = (v15 - v20) + 0.16667f;
    float v34 = (v16 - v19) + 0.16667f;
    float v35 = (v17 - v23) + 0.16667f;
    float v37 = 0.6f - v15 * v15 - v16 * v16 - v17 * v17;
    if (v37 >= 0.0f) {
        float v42 = v16;
        uint8_t v43 = perm[perm[perm[v32] + v31] + v30];
        if ((v43 & 0xFu) >= 8)
            v44 = v16;
        else
            v44 = v15;
        if ((v43 & 0xFu) >= 4) {
            if ((v43 & 0xD | 2) == 14)
                v42 = v15;
            else
                v42 = v17;
        }
        if (perm[perm[perm[v32] + v31] + v30] & 1)
            v44 = -v44;
        if (perm[perm[perm[v32] + v31] + v30] & 2)
            v42 = -v42;
        v40 = v37 * v37 * v37 * v37 * (v44 + v42);
    }
    float v47 = 0.6f - v33 * v33 - v34 * v34 - v35 * v35;
    if (v47 >= 0.0f) {
        uint8_t v53 = perm[perm[perm[v23 + v32] + (v19 + v31)] + (v20 + v30)];
        if ((v53 & 0xFu) >= 8)
            v54 = v34;
        else
            v54 = v33;
        if ((v53 & 0xFu) >= 4) {
            if ((v53 & 0xD | 2) == 14)
                v34 = v33;
            else
                v34 = v35;
        }
        if (v53 & 1)
            v54 = -v54;
        if (v53 & 2)
            v55 = -v34;
        else
            v55 = v34;
        v38 = v47 * v47 * v47 * v47 * (v54 + v55);
    }
    float v49 = v15 - v18 + 0.33333f, v51 = v17 - v21 + 0.33333f, v50 = v16 - v22 + 0.33333f;
    v15 -= 0.5f;
    v17 -= 0.5f;
    float v56 = 0.6f - v49 * v49 - v50 * v50 - v51 * v51, v57 = v16 - 0.5f, v58 = 0.0f, v59 = 0.0f, v62, v63;
    if (v56 >= 0.0f) {
        uint8_t v61 = perm[perm[perm[v21 + v32] + (v22 + v31)] + (v18 + v30)];
        if ((v61 & 0xFu) >= 8)
            v62 = v50;
        else
            v62 = v49;
        if ((v61 & 0xFu) >= 4) {
            if ((v61 & 0xD | 2) == 14)
                v50 = v49;
            else
                v50 = v51;
        }
        v63 = -v62;
        if ((v61 & 1) == 0)
            v63 = v62;
        if (v61 & 2)
            v50 = -v50;
        v59 = v56 * v56 * v56 * v56 * (v63 + v50);
    }
    float v64 = 0.6f - v15 * v15 - v57 * v57 - v17 * v17, v67, v68;
    if (v64 < 0.0f)
        return (v40 + v38 + v59 + v58) * 32.0f;
    uint8_t v66 = perm[perm[perm[v32 + 1] + 1 + v31] + 1 + v30];
    if ((v66 & 0xFu) >= 8)
        v67 = v57;
    else
        v67 = v15;
    if ((v66 & 0xFu) >= 4) {
        if ((v66 & 0xD | 2) == 14)
            v57 = v15;
        else
            v57 = v17;
    }
    v68 = -v67;
    if ((perm[perm[perm[v32 + 1] + 1 + v31] + 1 + v30] & 1) == 0)
        v68 = v67;
    if (perm[perm[perm[v32 + 1] + 1 + v31] + 1 + v30] & 2)
        v57 = -v57;
    v58 = v64 * v64 * v64 * v64 * (v68 + v57);
    return (v40 + v38 + v59 + v58) * 32.0f;
}
float g_timeStuff, g_timeStuff1;
int g_keys4[4];
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
        float v179, v261 = 0.0f, v263 = 0.0f, v80, v265, v265a, a3a, v42, v43, v44, dt2, jt1, jy2, v120, v121, v270, v150, v129, v137, v139;
        m_bikeWorldPos = m_pos;
        m_field_40 = m_field_34;
        auto io = m_pos;
        bool check;
        VEC3 wpos = m_field_34, tmp, tmp34, tmp35, epos{}, ipos{}, tmp49;
        MATRIX44 mx, mx1;
        auto jb = getJoystickButtons();
        switch (this->m_type) {
        case CT_FREE:
            if ((jb & 0x1000) == 0) {
                if (jb & 0x8000)
                    v261 = 84000.0f;
                else if (jb & 0x100)
                    v261 = 30.0f;
                else
                    v261 = 3000.0f;
                v263 = 3.0f * dt;
            }
            wpos = wpos - io;
            tmp = SafeNormalized(&wpos);
            v265 = tmp.m_data[0] * tmp.m_data[1];
            v265a = tmp.m_data[2] * tmp.m_data[1];
            a3a = -tmp.m_data[2] * tmp.m_data[2] - tmp.m_data[0] * tmp.m_data[0];
            MAT_Set44_Identity(&mx);
            MAT_Set44_Identity(&mx1);
            MAT_Set44_AxisAngle(&mx,  v265,           a3a,  v265a,         -JoyTrg1() * v263);
            MAT_Set44_AxisAngle(&mx1, -tmp.m_data[2], 0.0f, tmp.m_data[0], -JoyY2() * v263);
            tmp34 = mx1 * (mx * wpos);
            tmp35 = tmp34 - io;
            tmp35 = SafeNormalized(&tmp35);
            v42 = JoyX2p() - JoyX2m();
            v44 = JoyX1();
            v43 = JoyY1();
            tmp49.m_data[0] = v265   * v42 - tmp.m_data[2]   * v44 + v43 * tmp35.m_data[0]  * v261 * dt;
            tmp49.m_data[1] = (a3a   * v42                         + v43 * tmp35.m_data[1]) * v261 * dt;
            tmp49.m_data[2] = (v265a * v42 + tmp.m_data[0]   * v44 + v43 * tmp35.m_data[2]) * v261 * dt;
            epos = tmp34 + tmp49;
            ipos = tmp49 + io;
            v261 = epos.m_data[2];
            wpos = epos - m_pos;
            m_field_8 = wpos.Normalized();
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
            if (m_isElastic)
                ipos = m_field_EC * (1.0f - a3) + io * a3;
            else
                ipos = io;
            zassert(g_pRoadManager && HeightMapManager::GetInst());
            be = (BikeEntity *)g_FollowCam->GetLookAtEntity();
            if (be && g_pRoadManager && HeightMapManager::GetInst()) {
                auto v73 = be->m_road;
                int segment = v73 ? v73->m_segmentId : 0;
                auto v76 = g_pRoadManager->GetRoadSegment(segment);
                if (g_SideCam == this) {
                    if (!v76 || v76->IsPlaceholder())
                        ipos.m_data[1] = HeightMapManager::GetInst()->GetHeightAtLocation(VEC2{ be->m_pos.m_data[0], be->m_pos.m_data[2] }) + 90.0f;
                    else
                        ipos.m_data[1] = v76->GetCyclistAltitudeAtRoadTime(be->m_pos, be->m_field_888, be->m_field_8B8) + 90.0f;
                }
                auto v79 = -be->m_field_500 * 200.0f;
                v80 = wpos.m_data[1] + v79;
                ipos.m_data[1] += be->m_field_500 * 100.0f;
                epos.m_data[1] = v80;
                if (v76 && !v76->IsPlaceholder() && be->UpdateAnimation(v79))
                    m_bikeLocalPos.m_data[2] = -fabs(m_bikeLocalPos.m_data[2]);
                else
                    m_bikeLocalPos.m_data[2] = fabs(m_bikeLocalPos.m_data[2]);
            } else {
                v80 = wpos.m_data[1];
                epos.m_data[1] = wpos.m_data[1];
            }
            v261 = epos.m_data[2] = wpos.m_data[2];
            epos.m_data[0] = wpos.m_data[0];
            wpos.m_data[0] -= ipos.m_data[0];
            wpos.m_data[1] = v80 - ipos.m_data[1];
            wpos.m_data[2] -= ipos.m_data[2];
            m_field_8 = SafeNormalized(&wpos);
            break;
        case CT_BIGBRO:
            epos.m_data[1] = wpos.m_data[1];
            ipos = io + m_worldVec * dt;
            wpos = m_field_74;
            BikeLocalToWorldPos(&wpos);
            epos.m_data[2] = wpos.m_data[2];
            v261 = wpos.m_data[2];
            epos.m_data[0] = wpos.m_data[0];
            wpos -= ipos;
            m_field_8 = SafeNormalized(&wpos);
            break;
        case CT_TITLE:
            epos.m_data[2] = m_field_4C.m_data[2];
            wpos = m_field_4C;
            epos.m_data[0] = wpos.m_data[0];
            wpos.m_data[2] = epos.m_data[2] - m_pos.m_data[2];
            epos.m_data[1] = wpos.m_data[1];
            v261 = epos.m_data[2];
            wpos.m_data[1] = wpos.m_data[1] - m_pos.m_data[1];
            m_field_8 = wpos.Normalized();
            ipos = io;
            break;
        case CT_ORBIT:
            wpos = m_field_74;
            BikeLocalToWorldPos(&wpos);
            if (m_bCut) {
                m_bCut = false;
                auto prev_cam = g_CameraManager.m_cameras[g_CameraManager.m_prevSelIdx];
                io = m_bikeWorldPos = m_pos = prev_cam->m_pos;
                m_field_34 = wpos;
                m_bikeLocalPos.m_data[0] = (m_field_34 - m_pos).len();
                if (m_bikeLocalPos.m_data[0] > 2000.0f && (g_pGameWorld->WorldID() == WID_RICHMOND || g_pGameWorld->WorldID() == WID_LONDON))
                    m_bikeLocalPos.m_data[0] = 2000.0f;
            }
            v261 = wpos.m_data[2];
            ipos = m_field_34 - m_pos;
            epos = wpos;
            dt2 = dt + dt;
            ipos.Normalize();
            v265 = -ipos.m_data[0];
            v120 = ipos.m_data[1] * ipos.m_data[2];
            v121 = ipos.m_data[0] * ipos.m_data[2];
            tmp = wpos - m_field_34;
            v263 = -ipos.m_data[0] * ipos.m_data[0] - ipos.m_data[1] * ipos.m_data[1];
            jy2 = JoyY2();
            v270 = jy2 * v263 * 2500.0f;
            jt1 = JoyTrg1();
            v129 = (jy2 * v121 - jt1 * ipos.m_data[1]) * 2500.0f;
            jy2 = (jy2 * v120 + jt1 * ipos.m_data[0]) * 2500.0f;
            check = true;
            if (std::ranges::find(g_keys4, 263)) {
                if (g_timeStuff > -1.0f) {
                    g_timeStuff = fmaxf(g_timeStuff - dt2, -1.0f);
                    check = false;
                }
            }
            if (std::ranges::find(g_keys4, 262)) {
                if (g_timeStuff < 1.0f) {
                    g_timeStuff = fminf(dt2 + g_timeStuff, 1.0f);
                    check = false;
                }
            }
            if (check) {
                if (g_timeStuff < 0.0f)
                    g_timeStuff += fminf(-g_timeStuff, dt2);
                if (g_timeStuff > 0.0f)
                    g_timeStuff -= fminf(dt2, g_timeStuff);
            }
            v137 = ipos.m_data[1] * g_timeStuff;
            v265 = v265 * g_timeStuff;
            wpos = io - epos;
            if (wpos.Empty())
                v139 = wpos.m_data[1];
            else
                v139 = wpos.m_data[1] / wpos.len();
            check = true;
            if (std::ranges::find(g_keys4, 265)) {
                if (v139 < -0.8f) {
                    g_timeStuff1 = fmaxf(g_timeStuff1 - dt2, -1.0f);
                    check = false;
                }
            }
            if (std::ranges::find(g_keys4, 265)) {
                if (v139 > 0.1f) {
                    g_timeStuff1 = fminf(dt2 + g_timeStuff1, 1.0f);
                    check = false;
                }
            }
            if (check) {
                auto v144 = 2.0f;
                if (v139 >= 0.8f)
                    v144 = 5.0f;
                auto v147 = v144 * dt;
                if (g_timeStuff1 < 0.0f)
                    g_timeStuff1 += fminf(-g_timeStuff1, v147);
                if (g_timeStuff1 > 0.0f)
                    g_timeStuff1 -= fminf(v147, g_timeStuff1);
            }
            v120 = ((v120 * g_timeStuff1 + v265) * 1250.0 + jy2)  * dt + tmp.m_data[0];
            v150 =          (v263 * g_timeStuff1 * 1250.0 + v270) * dt + tmp.m_data[1];
            v121 = ((v121 * g_timeStuff1 + v137) * 1250.0 + v129) * dt + tmp.m_data[2];
            v263 = v150;
            check = true;
            if (std::ranges::find(g_keys4, 61) || std::ranges::find(g_keys4, 334)) {
                if (m_bikeLocalPos.m_data[0] > 150.0f) {
                    check = false;
                    g_camUpdX = fmaxf(g_camUpdX - dt * 4.0f, -1.0f);
                }
            }
            if (check) {
                if (std::ranges::find(g_keys4, 45) || std::ranges::find(g_keys4, 333)) {
                    if (m_bikeLocalPos.m_data[0] < 1300.0f) {
                        check = false;
                        g_camUpdX = fminf(dt * 4.0f + g_camUpdX, 1.0f);
                    }
                }
                if (check) {
                    auto dcam = dt * 2.5f;
                    if (g_camUpdX < 0.0f)
                        g_camUpdX += fminf(-g_camUpdX, dcam);
                    if (g_camUpdX > 0.0f)
                        g_camUpdX -= fminf(dcam, g_camUpdX);
                    v150 = v263;
                }
            }
            m_bikeLocalPos.m_data[0] = std::clamp(m_bikeLocalPos.m_data[0] + g_camUpdX * 500.0f * dt, 150.0f, 2500.0f);
            ipos.m_data[0] = v120 + io.m_data[0] - epos.m_data[0];
            ipos.m_data[1] = std::min(io.m_data[1] + v150, HeightMapManager::GetInst()->GetHeightAtLocation(VEC2{ v120, v121 }) + 50.0f) - epos.m_data[1];
            ipos.m_data[2] = v121 + io.m_data[2] - epos.m_data[2];
            ipos.Normalize();
            //QUEST v171 = ((COERCE_UNSIGNED_INT((jb & 0x100) ? 0xFFFFFED4 : 0) + (jb & 0x200) ? 0x12C : 0) * dt) + m_bikeLocalPos.m_data[0];
            //m_bikeLocalPos.m_data[0] = v171.m128_i32[0];
            wpos = epos - ipos * m_bikeLocalPos.m_data[0];
            m_field_8 = SafeNormalized(&wpos);
            break;
        case CT_8:
            if (!BikeManager::Instance()->FindBikeWithNetworkID(m_playerId, true) || !m_field_5C)
                return;
            BikeLocalToWorldPos(&m_field_34);
        //no break here
        default:
            epos.m_data[0] = wpos.m_data[0];
            epos.m_data[1] = wpos.m_data[1];
            epos.m_data[2] = wpos.m_data[2];
            v261 = wpos.m_data[2];
            ipos.m_data[0] = io.m_data[0];
            ipos.m_data[1] = io.m_data[1];
            ipos.m_data[2] = io.m_data[2];
            break;
        }
        if (m_field_BC != 0.0f)
            m_field_80 = (m_field_BC * dt) + m_field_80;
        ipos.m_data[0] -= m_bikeWorldPos.m_data[0];
        ipos.m_data[1] -= m_bikeWorldPos.m_data[1];
        v179 = ipos.m_data[2] - m_bikeWorldPos.m_data[2];
        be = BikeManager::Instance()->FindBikeWithNetworkID(m_playerId, true);
        float speed = 0.0f;
        bool nv187 = false;
        if (be) {
            speed = be->m_bc->GetSpeed();
            if (speed >= 0.1f)
                nv187 = true;
        }
        if (m_field_C4 < 0.1f || nv187) {
            if (m_isElastic && ipos.m_data[1] * ipos.m_data[1] + ipos.m_data[0] * ipos.m_data[0] + v179 * v179 <= 4'000'000.0f && be && be->m_entityType == 1 && nv187) {
                auto v189 = -be->m_heading.m_cos, v190 = -be->m_heading.m_heading2, v191 = -be->m_heading.m_sin;
                auto v193 = speed - m_field_C4;
                auto v194 = ipos.m_data[1] * v190 + ipos.m_data[0] * v189 + v179 * v191;
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
                auto v209 = (ipos.m_data[0] - v194 * v189) * 20.0f;
                auto v210 = (ipos.m_data[1] - v195) * 20.0f;
                auto v211 = (v179 - v197) * 20.0f;
                auto v212 = m_field_D0 * v197;
                if (m_field_CC < 0.0f) {
                    ipos.m_data[0] = v209 - m_field_F8 * m_field_D4.m_data[0];
                    ipos.m_data[1] = v210 - (m_field_F8 * m_field_D4.m_data[1]);
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
                    ipos.m_data[0] = v209 - (v215 * m_field_D4.m_data[0]);
                    v219 = v217 * m_field_E0.m_data[0];
                    ipos.m_data[1] = v210 - (v215 * m_field_D4.m_data[1]);
                    v220 = v212 - (v217 * m_field_E0.m_data[2]);
                    v221 = v211 - (v215 * m_field_D4.m_data[2]);
                }
                m_field_D4.m_data[0] += ipos.m_data[0] * dt;
                m_field_D4.m_data[1] += ipos.m_data[1] * dt;
                m_field_D4.m_data[2] += v221 * dt;
                m_field_E0.m_data[0] += (m_field_D0 * v194 * v189 - v219) * dt;
                m_field_E0.m_data[1] += v206 * dt;
                m_field_E0.m_data[2] += v220 * dt;
                m_pos.m_data[0] = (m_field_E0.m_data[0] + m_field_D4.m_data[0]) * 0.5f * dt + m_bikeWorldPos.m_data[0];
                m_pos.m_data[1] = (m_field_E0.m_data[1] + m_field_D4.m_data[1]) * 0.5f * dt + m_bikeWorldPos.m_data[1];
                m_pos.m_data[2] = (m_field_E0.m_data[2] + m_field_D4.m_data[2]) * 0.5f * dt + m_bikeWorldPos.m_data[2];
            } else {
                m_pos.m_data[0] = ipos.m_data[0] + m_bikeWorldPos.m_data[0];
                m_pos.m_data[1] = ipos.m_data[1] + m_bikeWorldPos.m_data[1];
                m_pos.m_data[2] = v179 + m_bikeWorldPos.m_data[2];
            }
            auto v233 = m_field_B0 * 25.0f; // Camera::ApplyShake inlined
            auto v235 = epos.m_data[0] * m_field_B4 * 0.002f;
            auto v236 = ipos.m_data[2] * m_field_B4 * 0.002f;
            auto v238 = SimplexNoise::Generate(v235, v236, m_field_B4 * epos.m_data[1] * 0.002f) * v233 + epos.m_data[1];
            auto v240 = SimplexNoise::Generate(m_field_B4 * v238 * 0.002f, v235, v236) * v233 + epos.m_data[0];
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
void SwitchCamera(int to) {
    switch (to) {
    case 0:
        g_DesiredCam = g_OrbitCam;
        break;
    case 1:
        g_DesiredCam = g_FollowCam;
        break;
    case 2:
        g_DesiredCam = g_CloseCam;
        break;
    case 3:
        g_DesiredCam = g_HeadCam;
        break;
    case 4:
        g_DesiredCam = g_SideCam;
        break;
    case 5:
        g_DesiredCam = g_WheelCam;
        break;
    case 6:
        g_DesiredCam = g_LeadCam;
        break;
    case 7:
        g_DesiredCam = g_DollyCam;
        break;
    case 8:
        g_DesiredCam = g_HeliCam;
        break;
    case 9:
        g_DesiredCam = g_ClassicCam;
        break;
    default:
        break;
    }
    if (g_friendsListGUIObj)
        g_friendsListGUIObj->m_changeCamera = true;
}