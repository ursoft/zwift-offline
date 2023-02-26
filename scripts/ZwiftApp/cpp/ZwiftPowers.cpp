#include "ZwiftApp.h"
void ZwiftPowers::AddOptionalTrainerProfile() {
    //TODO
}
void ZwiftPowers::Destroy() {
    //TODO
}
void ZwiftPowers::GetCappedPower(const char *) {
    //TODO
}
void ZwiftPowers::GetCappedPower(int) {
    //TODO
}
ZwiftPowers *ZwiftPowers::GetInst() {
    if (!g_pZwiftPowers) {
        g_pZwiftPowers = new ZwiftPowers();
    }
    return g_pZwiftPowers;
}
void ZwiftPowers::GetInterval(const char *) {
    //TODO
}
void ZwiftPowers::GetInterval(int) {
    //TODO
}
void ZwiftPowers::GetMaxAccel(const char *) {
    //TODO
}
void ZwiftPowers::GetMaxAccel(int) {
    //TODO
}
uint64_t ZwiftPowers::GetPower(const char *) {
    //TODO
    return 0;
}
uint64_t ZwiftPowers::GetPower(uint32_t) {
    //TODO
    return 0;
}
void ZwiftPowers::GetRegularCoefficient(const char *, float **) {
    //TODO
}
void ZwiftPowers::GetRegularCoefficient(int, float **) {
    //TODO
}
void ZwiftPowers::Init() {
    //TODO
}
void ZwiftPowers::IsSimplePower(const char *) {
    //TODO
}
void ZwiftPowers::IsSimplePower(int) {
    //TODO
}
void ZwiftPowers::ReadPowers(XMLDoc *, bool) {
    //TODO
}
ZwiftPowers::ZwiftPowers() { Init(); }
ZwiftPowers::~ZwiftPowers() {
    //TODO
}