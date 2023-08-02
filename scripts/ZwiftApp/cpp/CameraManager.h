#pragma once
void SetupGameCameras();
enum CAMTYPE { CT_1 = 1, CT_2 = 2, CT_3 = 3, CT_6 = 6, CT_7 = 7 };
struct Camera { //0x150 bytes
    /* inlined: ApplyCameraShake(float), ApplyShake(VEC3 &,VEC3 &,float,float), DebugCameraElasticity(void), MoveLocalCamera(VEC3,VEC3,float), SetElasticity(bool), SnapToPosition,
        StartCameraShake(float,float), UpdateLocalCameraOffsets(float), LocalToWorldVec*/
    virtual ~Camera() { /*nope*/ } //vptr[0]
    Camera(Entity *e, CAMTYPE ty);
    void ResetMovement();
    void RoadLocalToWorldPos(VEC3 *io);
    void UpdateFreeRideCamera(float);
    void Update(float, float); //vptr[1]
    Entity *GetLookAtEntity(); //vptr[2]
    void SetLookAtEntity(Entity *);
    void BikeLocalToWorldPos(VEC3 *);
    int64_t m_playerId = -1;
    VEC3 m_field_8{}, m_pos{}, m_bikeLocalPos{}, m_bikeWorldPos{}, m_field_D4{}, m_field_E0{};
    CAMTYPE m_type;
    bool m_field_18 = true, m_bCut = false, m_field_5C = true, m_field_C0 = false, m_isElastic = false, m_field_C8 = false;
    VEC3 m_field_74{}, m_worldVec{}, m_localVec{}, m_field_EC{}, m_field_34{};
    float m_field_80 = 65.0f, m_field_84 = 65.0f, m_field_88 = 10.0f, m_field_8C = 6'600'000.0f; //1x VEC4?
    float m_field_A8 = 1.0f, m_field_AC = 1.0f, m_field_B0 = 0.0f, m_field_B4 = 0.0f, m_field_B8 = 0.0f, m_field_BC = 0.0f;
    float m_field_C4 = 0.0f;
    float m_field_CC = 0.0f, m_field_D0 = 38.0f, m_field_F8 = 1.5f, m_field_FC = 2.0f, m_field_100 = 19.0f, m_field_104 = 38.0f;
    float field_108[2]{}, m_field_110 = 0.0f, m_accTime = 0.0f;
    VEC3 m_field_118{}, m_field_124{}, m_field_130{}, m_field_13C{};
    int m_field_148 = 0, gap = 0;
};
inline Camera *g_DesiredCam, *g_HeadCam, *g_FollowCam, *g_WheelCam, *g_SideCam, *g_LeadCam, *g_DollyCam, *g_pTitleCamera, *g_HeliCam, *g_OrbitCam, *g_CloseCam, *g_ClassicCam, *g_FreeCam;
inline VEC4 g_cident3{0.0f, 0.0f, 0.0f, 1.0f};
struct CameraManager { //0x98 bytes
    std::string m_reg = "{\"registered\":false,\"registrationStatus\":2}"s;
    VEC3 m_pos{}, m_field_50{};
    std::deque<Camera *> m_cameras;
    Camera *m_tmpCam = nullptr;
    int m_selIdx = 0, m_prevSelIdx = 0, m_disabledElastVal = 0;
    float m_cut = 0.0f, m_cutInv = 0.0f, m_tmpCamVal = 0.0f;
    bool m_needSort = false, m_disabledElast = false;
    ~CameraManager();
    VEC3 GetCameraPos();
    Camera *GetSelectedCamera();
    Camera *CreateCamera(Entity *, CAMTYPE);
    void CutToCamera(Camera *, float cut);
    int FindCamera(Camera *);
    void Update(float dt);
    CameraManager() {}
        /*inlined/not used DebugApplyCameraPosition(VEC3 const&,VEC3 const&), CameraManager::DeleteCamera(Camera *), DisableElasticity(uint), GetCameraRotMatrix(MATRIX33 &)
            GetTemporaryCamera(), IsElasticityEnabled(), PasteCameraPositionFromClipboard(), RegisterCommands(), Self(), SetSteeringMode(SteeringMode,bool), SetTemporaryCamera(Camera *,float), UpdateGlobalElasticity() */
};
inline CameraManager g_CameraManager;
float JoyY2();