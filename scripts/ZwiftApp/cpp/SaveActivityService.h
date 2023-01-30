#pragma once
class SaveActivityService {
public:
    SaveActivityService();
    static void Shutdown();
    static bool IsInitialized() { return g_SaveActivityServicePtr.get() != nullptr; }
    inline static std::unique_ptr<SaveActivityService> g_SaveActivityServicePtr;
};
