#pragma once
namespace zwift_network {
    enum NetworkRequestOutcome { NRO_NULL, NRO_CNT };
    void shutdown_zwift_network();
}
namespace ZNet {
    struct Error {
        Error(std::string_view msg, zwift_network::NetworkRequestOutcome netReqOutcome) : m_msg(msg), m_netReqOutcome(netReqOutcome), m_hasNetReqOutcome(true) {}
        Error(std::string_view msg) : m_msg(msg) {}

        std::string_view m_msg;
        zwift_network::NetworkRequestOutcome m_netReqOutcome = zwift_network::NRO_NULL;
        bool m_hasNetReqOutcome = false;
    };
    class NetworkService {
        inline static std::unique_ptr<NetworkService> g_NetworkService;
    public:
        NetworkService() {}
        //static void Initialize(Experimentation *exp);
        static bool IsInitialized() { return g_NetworkService.get() != nullptr; }
        static NetworkService *Instance() { zassert(g_NetworkService.get() != nullptr); return g_NetworkService.get(); }
        static void Shutdown() { g_NetworkService.reset(); }
    };
}
void ZNETWORK_Shutdown();
