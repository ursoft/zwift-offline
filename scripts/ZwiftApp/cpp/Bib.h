#pragma once
struct BibInfo {

};
class Bib {
public:
    void CalcScaleAndBias(VEC4 *, const VEC2 &, const VEC2 &);
    uint32_t GetBibShader();
    void GetDefaultInfo();
    static void InitOnce(/*int, int, const CFont2D **/);
    void Set(uint32_t, const BibInfo *, ZSPORT, uint32_t);
};
