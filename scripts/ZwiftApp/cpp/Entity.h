#pragma once
struct Entity {
    enum EType { ET_BLIMP = 4, ET_CNT = 0x33 };
    VEC3 m_pos{};
    virtual const VEC3 &GetPosition() { return m_pos; }
    void SetPos(const VEC3 &pos) { m_pos = pos; }
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
    std::vector<std::vector<Entity>> m_ents;
    EntityManager() {
        //TODO
    }
    static EntityManager g_entityMgr;
    static EntityManager *GetInst() {
        return &g_entityMgr;
    }
    Entity *FindFirstEntityOfType(Entity::EType e) {
        zassert(e >= 0 && e < Entity::ET_CNT && "EntityManager::FindFirstEntityOfType Asserted; invalid type");
        auto &vec = m_ents[e];
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