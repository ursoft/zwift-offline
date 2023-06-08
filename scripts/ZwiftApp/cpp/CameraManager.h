#pragma once
void SetupGameCameras();
enum CAMTYPE { CT_1 = 1, CT_2 = 2, CT_3 = 3, CT_6 = 6, CT_7 = 7 };
struct Camera { //0x150 bytes
    /*Camera::ApplyCameraShake(float)
Camera::ApplyShake(VEC3 &,VEC3 &,float,float)
Camera::BikeLocalToWorldPos(VEC3 &)
Camera::DebugCameraElasticity(void)
Camera::GetLookAtEntity(void)
Camera::LocalToWorldVec(VEC3 &)
Camera::MoveLocalCamera(VEC3,VEC3,float)
Camera::ResetMovement(void)
Camera::RoadLocalToWorldPos(VEC3 &)
Camera::SetElasticity(bool)
Camera::SetLookAtEntity(Entity *)
Camera::SnapToPosition(void)
Camera::StartCameraShake(float,float)
Camera::Update(float,float)
Camera::UpdateFreeRideCamera(float)
Camera::UpdateLocalCameraOffsets(float)*/
    virtual ~Camera() {}
    Camera(BikeEntity *bike, CAMTYPE ty) : m_type(ty) {
        if (bike) {
            if (bike->m_field_B8 == 1) {
                m_playerId = 0;
                if (!bike->m_field_C98)
                    m_playerId = bike->m_playerIdTx;
            } else {
                zassert(0);
            }
        }
    }
    int64_t m_playerId = -1;
    float field_1C[16]{};
    VEC3 field_8{};
    CAMTYPE m_type;
    bool field_18 = true, field_5C = true, field_A4 = false, field_A5 = false, field_A6 = false, field_A7 = false, field_C0 = false, m_isElastic = false, field_C8 = false;
    //15 floats:
    float field_68 = 0.0f, field_6C = 0.0f, field_70 = 0.0f, field_74 = 0.0f, field_78 = 0.0f, field_7C = 0.0f; //2x VEC3?
    float field_80 = 65.0f, field_84 = 65.0f, field_88 = 10.0f, field_8C = 6600000.0f; //1x VEC4?
    float field_90 = 0.0f, field_94 = 0.0f, field_98 = 0.0f, field_9C = 0.0f, field_A0 = 0.0f; //5x
    //gap here (bools)
    float field_A8 = 1.0f, field_AC = 1.0f, field_B0 = 0.0f, field_B4 = 0.0f, field_B8 = 0.0f, field_BC = 0.0f; //2x VEC3?
    //gap here (bools)
    float field_C4 = 0.0f;
    //gap here (bool)
    float field_CC = 0.0f, field_D0 = 38.0f, field_D4[9]{}, field_F8 = 1.5f, field_FC = 2.0f, field_100 = 19.0f, field_104 = 38.0f;
    float field_108[18]{};
};
inline Camera *g_DesiredCam, *g_HeadCam, *g_FollowCam, *g_WheelCam, *g_SideCam, *g_LeadCam, *g_DollyCam, *g_pTitleCamera, *g_HeliCam, *g_OrbitCam, *g_CloseCam, *g_ClassicCam, *g_FreeCam;
struct CameraManager {
    Camera *GetSelectedCamera();
    Camera *CreateCamera(Entity *, CAMTYPE);
    bool m_hasElasticCams = false;
    /*CameraManager::CameraManager(void)
CameraManager::CopyCameraPositionToClipboard(void)
CameraManager::CutToCamera(Camera *,float)
CameraManager::DebugApplyCameraPosition(VEC3 const&,VEC3 const&)
CameraManager::DeleteCamera(Camera *)
CameraManager::DisableElasticity(uint)
CameraManager::FindCamera(Camera *)
CameraManager::GetCameraPos(void)
CameraManager::GetCameraRotMatrix(MATRIX33 &)
CameraManager::GetTemporaryCamera(void)
CameraManager::IsElasticityEnabled(void)
CameraManager::PasteCameraPositionFromClipboard(void)
CameraManager::RegisterCommands(void)
CameraManager::Self(void)
CameraManager::SetSteeringMode(SteeringMode,bool)
CameraManager::SetTemporaryCamera(Camera *,float)
CameraManager::Update(float)
CameraManager::UpdateGlobalElasticity(void)
CameraManager::~CameraManager()
CameraManager_Update(float)*/
};
inline CameraManager g_CameraManager;