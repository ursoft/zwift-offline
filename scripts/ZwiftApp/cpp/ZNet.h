#pragma once
inline void str_tolower(std::string &s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); }); }
enum NetworkRequestOutcome { NRO_NULL, NRO_CNT };
void shutdown_zwift_network();
struct NetworkClientImpl;
struct NetworkResponseBase {
    std::string m_msg;
    int m_errCode = 0;
    void storeError(int code, const char *errMsg) { 
        if (errMsg)
            m_msg = errMsg;
        else
            m_msg.clear();
        m_errCode = code; 
    }
    const NetworkResponseBase &storeError(const NetworkResponseBase &src) { m_msg = src.m_msg; m_errCode = src.m_errCode; return *this; }
    const NetworkResponseBase &storeError(int code, std::string &&errMsg) { m_msg = std::move(errMsg); m_errCode = code; return *this; }
    const NetworkResponseBase &storeError(int code, const std::string &errMsg) { m_msg = errMsg; m_errCode = code; return *this; }
    bool ok(NetworkResponseBase *errDest = nullptr) const {
        if (errDest)
            errDest->storeError(*this);
        return m_errCode == 0;
    }
};
template<class T> struct NetworkResponse : public NetworkResponseBase { 
    T m_T; 
    operator T &() { return m_T; }
    operator const T &() const { return m_T; }
};
namespace zwift_network {
    void get_goals(int64_t playerId);
    void save_goal(const protobuf::Goal &);
    NetworkResponseBase log_out();
}
struct NetworkClient {
    NetworkClientImpl *m_pImpl;
    NetworkClient();
    ~NetworkClient();
    static void globalInitialize();
    static void globalCleanup();
    void initialize(const std::string &server, const std::string &certs, const std::string &version);
    NetworkResponseBase logInWithOauth2Credentials(const std::string &sOauth, const std::vector<std::string> &a4, const std::string &oauthClient);
    NetworkResponseBase logInWithEmailAndPassword(const std::string &email, const std::string &pwd, const std::vector<std::string> &anEventProps, bool reserved, const std::string &oauthClient);
    NetworkResponseBase logOut();
};
namespace ZNet {
    struct Error {
        Error(std::string_view msg, NetworkRequestOutcome netReqOutcome) : m_msg(msg), m_netReqOutcome(netReqOutcome), m_hasNetReqOutcome(true) {}
        Error(std::string_view msg) : m_msg(msg) {}

        std::string_view m_msg;
        NetworkRequestOutcome m_netReqOutcome = NRO_NULL;
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
namespace uuid {
    static std::random_device              rd;
    static std::mt19937                    gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);
    static std::uniform_int_distribution<> dis2(8, 11);

    inline std::string generate_uuid_v4() {
        std::stringstream ss;
        int i;
        ss << std::hex;
        for (i = 0; i < 8; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 4; i++) {
            ss << dis(gen);
        }
        ss << "-4";
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        ss << dis2(gen);
        for (i = 0; i < 3; i++) {
            ss << dis(gen);
        }
        ss << "-";
        for (i = 0; i < 12; i++) {
            ss << dis(gen);
        };
        return ss.str();
    }
}
void ZNETWORK_Shutdown();
uint64_t ZNETWORK_GetNetworkSyncedTimeGMT();
bool ZNETWORK_IsLoggedIn();
void ZNETWORK_Initialize();

inline bool g_IsOnProductionServer = true;
inline NetworkClient *g_networkClient;