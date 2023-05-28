#include "ZwiftApp.h"
void GroupEvents::Initialize(Experimentation *exp) {
    exp->IsEnabled(FID_EVENTCO, [](ExpVariant val) {
        //TODO doClientTelemetryFID_EVENTCO
        });
    exp->IsEnabled(FID_EVENTQU, [](ExpVariant val) {
        //TODO doClientTelemetryEVENTQU
        });
}
void GroupEvents::Shutdown() {
    /* TODO v22 = *(__int64 **)&g_sExperimentationUPtr;
    sub_7FF658BAE530((__int128 ***)&qword_7FF65A1E6DD8);
    Mylast = g_GroupEvents._Mylast;
    for (i = (__int128 *)g_GroupEvents._Myfirst; i != (__int128 *)Mylast; ++i) {
        v25 = *v22;
        v46 = *i;
        (*(void(__fastcall **)(__int64 *, __int128 *))(v25 + 40))(v22, &v46);
    }*/
}
void *GroupEvents::FindSubgroupEvent(int64_t id) {
    //TODO
    return nullptr;
}
