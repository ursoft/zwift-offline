#pragma once
class ConnectionManager { // 0x138 bytes
public:
    ConnectionManager();
    ~ConnectionManager();
    static void DestroyInstance();
    void Subscribe(SaveActivityService *serv);
    void Unsubscribe(SaveActivityService *serv);
    static ConnectionManager *Instance();
    static bool IsInitialized();
    static void Initialize();
};
inline std::unique_ptr<ConnectionManager> g_ConnectionManagerUPtr;
