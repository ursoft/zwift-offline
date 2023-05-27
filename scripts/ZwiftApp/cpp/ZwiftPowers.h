#pragma once
struct ZwiftPowers { //24 bytes
    void AddOptionalTrainerProfile();
    void Destroy();
    void GetCappedPower(const char *);
    void GetCappedPower(int);
    static ZwiftPowers *GetInst();
    void GetInterval(const char *);
    void GetInterval(int);
    void GetMaxAccel(const char *);
    void GetMaxAccel(int);
    uint64_t GetPower(const char *);
    uint64_t GetPower(uint32_t);
    void GetRegularCoefficient(const char *, float **);
    void GetRegularCoefficient(int, float **);
    void Init();
    void IsSimplePower(const char *);
    void IsSimplePower(int);
    void ReadPowers(XMLDoc *, bool);
    ZwiftPowers();
    ~ZwiftPowers();
};
inline ZwiftPowers *g_pZwiftPowers;