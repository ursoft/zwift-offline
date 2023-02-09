#pragma once
namespace zwift_network {
    enum NetworkRequestOutcome { NRO_NULL, NRO_CNT };
    void shutdown_zwift_network();
    void get_goals(int64_t playerId);
    void save_goal(const protobuf::Goal &);

    template<class PB>
    class NetworkResponse {
    public:
        PB *m_data;
    };
}
namespace ZNet {
    struct Error {
        Error(std::string_view msg, zwift_network::NetworkRequestOutcome netReqOutcome) : m_msg(msg), m_netReqOutcome(netReqOutcome), m_hasNetReqOutcome(true) {}
        Error(std::string_view msg) : m_msg(msg) {}

        std::string_view m_msg;
        zwift_network::NetworkRequestOutcome m_netReqOutcome = zwift_network::NRO_NULL;
        bool m_hasNetReqOutcome = false;
    };
    struct Params {};
    class NetworkService {
        inline static std::unique_ptr<NetworkService> g_NetworkService;
    public:
        NetworkService() {}
        ~NetworkService() {}
        static void Initialize() { g_NetworkService.reset(new NetworkService()); }
        static bool IsInitialized() { return g_NetworkService.get() != nullptr; }
        static NetworkService *Instance() { zassert(g_NetworkService.get() != nullptr); return g_NetworkService.get(); }
        static void Shutdown() { g_NetworkService.reset(); }
        //void GetAllTimeBestEffortsPowerCurve(std::function<void()(protobuf::PowerCurveAggregationMsg const &)>, ZNet::Params);
        //void GetAssetSummary(std::string const &, std::function<void()(protobuf::AssetSummary const &)>, ZNet::Params);
        //void GetProfile(long long, std::function<void()(protobuf::PlayerProfile const &)>, ZNet::Params);
        //void GetWorkout(std::string const &, std::function<void()(std::string const &)>, ZNet::Params);
        //void GetWorkoutSummaries(std::optional<std::string>, std::optional<std::string>, std::function<void()(protobuf::WorkoutSummaries const &)>, ZNet::Params);
        //void UpdateProfile(bool, const protobuf::PlayerProfile &, bool, std::function<void()(void)> const &, ZNet::Params const &);
    };
}
void ZNETWORK_Shutdown();
uint64_t ZNETWORK_GetNetworkSyncedTimeGMT();
bool ZNETWORK_IsLoggedIn();
void ZNETWORK_Initialize();

