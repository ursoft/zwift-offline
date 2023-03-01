#pragma once
class Sky {
public:
    static void Initialize();
    void Create(const char *, uint64_t);
    void DefGetName(int);
    //void DefIterate(std::function<void ()(int)> &&)
    void Destroy(int *);
    void GenerateSphere(uint32_t);
    void GetDefName(int);
    //void GetSceneLightingValues(SceneLightingValues *, float);
    void GetSkyRadius();
    //void Lerp(const SceneLightingValues &, const SceneLightingValues &, float);
    //void Lerp(const SkyInstance &);
    void Render(const VEC3 &, float, uint32_t);
    void Render(int, float, float);
    void Render_PostFX(const VEC3 &, float, bool);
    void SetFogColor(const VEC4 &);
    void SetSkyFogPercent(float);
    void Update(float);
};