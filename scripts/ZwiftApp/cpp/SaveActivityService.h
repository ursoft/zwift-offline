#pragma once
class SaveActivityService { //0x128 bytes
public:
    SaveActivityService(XMLDoc *doc, Experimentation *exp, EventSystem *ev);
    static void Shutdown();
    static bool IsInitialized() { return g_SaveActivityServicePtr.get() != nullptr; }
    static void Initialize(XMLDoc *doc, Experimentation *exp, EventSystem *ev);
    inline static std::unique_ptr<SaveActivityService> g_SaveActivityServicePtr;
};
