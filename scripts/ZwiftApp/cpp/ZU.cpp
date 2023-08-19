#include "ZU.h" //READY for testing
#include "GameCritical.h"
bool ZU_IsInPreReleaseRestrictedMode(int courseId) { return false; }
bool ZU_IsInPreReleaseRestrictedMode() { return false; }
void AUDIO_Shutdown();
void ZwiftExit(int code) {
    GameCritical::AbortJobs();
    //using namespace std::chrono_literals;
    //std::this_thread::sleep_for(5000ms); //бред!
    AUDIO_Shutdown();
    ShutdownSingletons();
    exit(code);
}
