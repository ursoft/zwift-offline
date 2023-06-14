#pragma once
void ZwiftLeaveCriticalSection(int idx);
bool ZwiftEnterCriticalSection(int idx);
void ZMUTEX_SystemInitialize();
void ZMUTEX_SystemShutdown();
int ZMUTEX_Create(const char *name);
void ZMUTEX_Delete(int handle);