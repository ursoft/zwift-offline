#pragma once
class ParticulateManager {
public:
    static void Create();
    static void Init();
};
class ParticleSystem { //0xD0 bytes
public:
    int m_texture, m_shader;
    ParticleSystem(int dur, void *func, void *n1, void *n2, void *n3, void *n4) { /*TODO*/ }
    void UpdateAndRender(float, VEC3, VEC3, float) { /*TODO*/ }
};
inline void PARTICLESYS_Register(ParticleSystem *) { /*TODO*/ }
inline void PARTICLESYS_Unregister(ParticleSystem *) { /*TODO*/ }
inline void PARTICLESYS_UpdateAndRender(float, VEC3, VEC3) { /*TODO*/ }
inline void DustParticleUpdate(void *a1, void *a2, float a3) { /*TODO*/ }
inline void ConfettiParticleUpdate(void *a1, void *a2, float a3) { /*TODO*/ }

inline ParticleSystem *g_pDustSystem, *g_pConfettiSystem;