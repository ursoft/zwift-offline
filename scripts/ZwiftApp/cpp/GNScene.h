#pragma once
struct GNSceneCreateParams {
    int m_tier, m_field_4, m_field_8, field_C;
    void *m_field_10;
    float m_field_18, m_field_1C, m_field_20, m_field_24, m_field_28, m_field_2C;
};
struct GNView;
struct GNSceneSSR {};
struct GNScene {
    void AddLight(uint32_t);
    void AddModel(uint32_t);
    void BeginFrame();
    void BeginView(GNView *);
    void DestroyAll();
    void EndFrame();
    void EndView(GNView *);
    void GetCurrentEnvironmentReflection();
    void Initialize(const GNSceneCreateParams &);
    void RemoveLight(uint32_t);
    void RemoveModel(uint32_t);
    void SetCustomEnvironmentReflection(int);
    void SetDefaultEnvironmentReflection();
    void SetSSR(const GNSceneSSR &);
    ~GNScene();
};
inline GNScene g_GNSceneSystem;
