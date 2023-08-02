#pragma once
struct Entity {
    enum EType { ET_UNK = 0, ET_BIKE = 1, ET_BLIMP = 4, ET_CNT = 0x33 };
    VEC3 m_pos{};
    MATRIX44 m_matrix{};
    int64_t m_playerIdTx = 0;
    EType m_entityType = ET_UNK;
    bool m_field_C98 = false;
    virtual const VEC3 &GetPosition() { return m_pos; }
    void SetPos(const VEC3 &pos) { m_pos = pos; }
    virtual ~Entity() {}
    virtual void Update(float dt) { /*TODO*/ }
};
struct RoadMarkerEntity : public Entity {};
struct Camera;
struct PhotoCamEntity : public Entity { //0x320 bytes
    Camera *m_camera1 = nullptr, *m_camera2 = nullptr;
    VEC3 m_field_314{}, m_field_2B8{ 10.0f, 10'000.0f, 65.0f }, m_field_2D8{ 0.0f, 0.0f, 1.0f }, m_field_2E4{}, m_field_2F0{};
    VEC2 m_field_2C4{ 35.0f, 0.0f };
    void *m_field_2B0 = nullptr;
    int m_field_30C = -1;
    float m_field_2CC = 20'000.0f;
    bool m_field_310 = false, m_field_308 = false, m_field_309 = true, m_field_30A = true, m_field_30B = false, m_field_2D4 = false, m_field_2D5 = false,
        m_field_2D0 = false, m_field_2D1 = false, m_field_2D2 = false, m_field_2D3 = false;
};
struct BlimpEntity : public Entity {
    double m_roadTime = 0.0;
    BlimpEntity() {
        //TODO
    }
    /*void BlimpNavLightsDeath(particle *);
    void BlimpNavLightsRespawn(particle *, ParticleSystem const *);
    void BlimpNavLightsUpdate(particle *, uint, float, ParticleSystem const *);
    void HasNonEventMesh(void);
    void Initialize(int, double, float, VEC4, char const *, char const *);
    void InitializeEnd(void);
    void NetworkUpdate(void *, uint);
    void Render(void);
    void SetupPosition(void);
    void SwapMesh(std::string);
    void SwapTexture(char const *, int, int);
    void Update(float);
    */
    void SetRoadTime(double rt) {
        if (fabs(rt - m_roadTime) > 0.01) //QUEST: why wasting time
            m_roadTime = rt;
    }
    ~BlimpEntity() {
        //TODO
    }
};
struct EntityManager { //0x5F8 bytes
    std::vector<std::vector<Entity>> m_ents_bt;
    std::vector<PhotoCamEntity *> m_ents_photocam;
    EntityManager() {
        //TODO
    }
    static EntityManager g_entityMgr;
    static EntityManager *GetInst() {
        return &g_entityMgr;
    }
    Entity *FindFirstEntityOfType(Entity::EType e) {
        zassert(e >= 0 && e < Entity::ET_CNT && "EntityManager::FindFirstEntityOfType Asserted; invalid type");
        auto &vec = m_ents_bt[e];
        if (vec.empty())
            return nullptr;
        return &vec[0];
    }
    /*
    void AddEntity(Entity *);
    void AddToRenderList(Entity *,GFX_RenderPass);
    void ApplyHolidayMesh(std::string *);
    void AttachMeToPath(int,Entity *);
    void ClearAllRenderLists(void);
    void Destroy(void);
    void DestroyEntity(Entity *);
    void DestroyEntity(std::__wrap_iter<Entity **>,bool);
    void Exists(void);
    void FindFirstEntityOfType(Entity::EType);
    void FindFirstEntityOfTypeAndHashedEntityName(Entity::EType,uint);
    void FindFirstEntityOfTypeAndName(Entity::EType,char const*);
    void FindPathById(int);
    void FindRegionOfTypeAtLocation(uint,VEC2 const&);
    void GetAnimationHandle(GDE_Header_360 *,char const*);
    void GetEntityDisplayName(Entity::EType);
    void GetEntityName(Entity::EType);
    void GetEntityNameShort(Entity::EType);
    void GetMesh(char const*,bool);
    void GetSkinMesh(char const*);
    void HandleEvent(EVENT_ID,std::__va_list);
    void LoadXML(XMLDoc *,bool);
    void MapEntityType(std::string const&);
    void NukeEmAll(void);
    void PostInitialize(void);
    void PostLoadThreadedInitialize(void);
    void RemoveFromRenderList(Entity *);
    void RemoveFromRenderList(Entity *,GFX_RenderPass);
    void Render(void);
    void Render2D(void);
    void SessionReset(void);
    void SortRender(Camera const*);
    void SpawnEntityFromData(EntityNode &,std::vector<Entity *> &);
    void Update(float);
*/
    ~EntityManager() {
        //TODO
    }
};