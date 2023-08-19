#pragma once //READY for testing
void ZMUTEX_Unlock(int idx);
void ZMUTEX_Lock(int idx);
inline bool ZMUTEX_TryLock(int idx) { ZMUTEX_Lock(idx); return true; }
void ZMUTEX_SystemInitialize();
void ZMUTEX_Shutdown();
int ZMUTEX_Create(const char *name);
void ZMUTEX_Delete(int handle);