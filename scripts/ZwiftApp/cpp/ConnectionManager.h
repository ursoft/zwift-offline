#pragma once
class ConnectionManager {
    inline static std::unique_ptr<ConnectionManager> g_ConnectionManagerUPtr;
public:
    ConnectionManager();
    ~ConnectionManager();
    static bool IsInitialized() { return g_ConnectionManagerUPtr.get() != nullptr; }
    static void DestroyInstance();
    void Unsubscribe(SaveActivityService *serv);
    static ConnectionManager *Instance() { zassert(IsInitialized()); return g_ConnectionManagerUPtr.get(); }
};
