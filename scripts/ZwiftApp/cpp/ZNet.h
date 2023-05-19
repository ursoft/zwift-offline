#pragma once
inline void str_tolower(std::string &s) { std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); }); }
enum NetworkRequestOutcome {
    NRO_OK = 0, NRO_INITIALIZATION_FAILED, NRO_NOT_INITIALIZED, NRO_NOT_LOGGED_IN, NRO_NO_LOG_IN_ATTEMPTED, NRO_NO_PLAYER_ID_YET,
    NRO_NO_WORLD_SELECTED, NRO_NO_MAP_REVISION_ID_SELECTED, NRO_WORLD_CLOCK_NOT_INITIALIZED_YET, NRO_NO_HASH_CODES_YET, NRO_UDP_CLIENT_STOPPED,
    NRO_INVALID_WORLD_ID, NRO_NOT_PAIRED_TO_PHONE, NRO_TCP_PAIRING_NOT_SUPPORTED, NRO_DISCONNECTED_DUE_TO_SIMULTANEOUS_LOGINS, NRO_WORLD_FULL,
    NRO_ROADS_CLOSED, NRO_MISSING_REQUIRED_PARTNER_SCOPE, NRO_NOT_CONNECTED_TO_PARTNER, NRO_CLIENT_TO_SERVER_IS_TOO_BIG, NRO_REQUEST_ABORTED,
    NRO_INVALID_ARGUMENT, NRO_ERROR_READING_FILE, NRO_JSON_PARSING_ERROR, NRO_JSON_WEB_TOKEN_PARSING_ERROR, NRO_PROTOBUF_FAILURE_TO_ENCODE,
    NRO_PROTOBUF_FAILURE_TO_DECODE, NRO_NO_CREDENTIALS_TO_REQUEST_ACCESS_TOKEN_WITH, NRO_REFRESH_TOKEN_EXPIRED, NRO_TOO_MANY_ACCESS_TOKEN_REQUESTS,
    NRO_CURL_ERROR, NRO_REQUEST_TIMED_OUT, NRO_UNEXPECTED_HTTP_RESPONSE, NRO_LAN_EXERCISE_DEVICE_INVALID_HANDLE,
    NRO_LAN_EXERCISE_DEVICE_INVALID_STATUS, NRO_LAN_EXERCISE_DEVICE_ERROR_CONNECTING, NRO_LAN_EXERCISE_DEVICE_CONNECTION_REFUSED,
    NRO_LAN_EXERCISE_DEVICE_ERROR_SENDING_MESSAGE, NRO_TOO_MANY_SEGMENT_RESULTS_SUBSCRIPTIONS, NRO_LAN_EXERCISE_DEVICE_NETWORK_UNREACHABLE,
    NRO_ENCRYPTION_FAILURE,
    NRO_HTTP_STATUS_BAD_REQUEST = 400, NRO_HTTP_STATUS_UNAUTHORIZED = 401, NRO_HTTP_STATUS_FORBIDDEN = 403, NRO_HTTP_STATUS_NOT_FOUND = 404,
    NRO_HTTP_STATUS_CONFLICT = 409, NRO_HTTP_STATUS_GONE = 410, NRO_HTTP_STATUS_TOO_MANY_REQUESTS = 429, NRO_HTTP_STATUS_SERVICE_UNAVAILABLE = 503,
    NRO_HTTP_STATUS_BANDWIDTH_LIMIT_EXCEEDED = 509 };
void shutdown_zwift_network();
template<class T>
struct Optional {
    T m_T = default();
    bool m_hasValue = false;
    void setJson(Json::Value *dest, const std::string &index, const std::function<std::string(const T&)> &f = std::function<std::string(const T &)>()) const {
        if (!m_hasValue) return;
        if (f)
            (*dest)[index] = f(m_T);
        else
            (*dest)[index] = m_T;
    }
    void setOrClear(bool set = false, const T &val = T()) {
        if (set) {
            m_T = val;
        } else {
            if (m_hasValue)
                m_T = T();
        }
        m_hasValue = set;
    }
};
struct NetworkClientImpl;
struct NetworkResponseBase {
    std::string m_msg;
    NetworkRequestOutcome m_errCode = NRO_OK;
    void storeError(NetworkRequestOutcome code, const char *errMsg) {
        if (errMsg)
            m_msg = errMsg;
        else
            m_msg.clear();
        m_errCode = code; 
    }
    const NetworkResponseBase &storeError(const NetworkResponseBase &src) { m_msg = src.m_msg; m_errCode = src.m_errCode; return *this; }
    const NetworkResponseBase &storeError(NetworkRequestOutcome code, std::string &&errMsg) { m_msg = std::move(errMsg); m_errCode = code; return *this; }
    const NetworkResponseBase &storeError(NetworkRequestOutcome code, const std::string &errMsg) { m_msg = errMsg; m_errCode = code; return *this; }
    bool ok(NetworkResponseBase *errDest = nullptr) const {
        if (errDest)
            errDest->storeError(*this);
        return m_errCode == NRO_OK;
    }
};
template<class T> struct NetworkResponse : public NetworkResponseBase {
    T m_T; 
    operator T &() { return m_T; }
    operator const T &() const { return m_T; }
};
template<> struct NetworkResponse<void> : public NetworkResponseBase {};
template<class RET>
std::future<NetworkResponse<RET>> makeNetworkResponseFuture(NetworkRequestOutcome code, std::string &&msg) {
    std::promise<NetworkResponse<RET>> immediate;
    auto ret = immediate.get_future();
    immediate.set_value(NetworkResponse<RET>{msg, code});
    return ret;
}
namespace zwift_network {
    struct Motion { //32 bytes
        float m_ptg_f3 = 0.0f;
        float m_ptg_f4 = 0.0f;
        float m_ptg_f5 = 0.0f;
        float m_ptg_f6 = 0.0f;
        float m_ptg_f7 = 0.0f;
        float m_ptg_f8 = 0.0f;
        double m_ptg_f9 = 0.0;
    };
    void get_goals(int64_t playerId);
    void save_goal(const protobuf::Goal &);
    std::future<NetworkResponse<std::string>> log_out();
    std::future<NetworkResponse<void>> reset_password(const std::string &newPwd);
    std::future<NetworkResponse<protobuf::PlayerState>> latest_player_state(int64_t worldId, int64_t playerId);
}
struct ProfileRequestLazyContext {
    struct PlayerIdProvider {
        virtual std::unordered_set<int64_t> getPlayerIds(uint32_t key) = 0;
    };
    uint32_t m_key = 0;
    PlayerIdProvider *m_prov;
    ProfileRequestLazyContext(uint32_t key, PlayerIdProvider *prov) : m_key(key), m_prov(prov) {}
    std::unordered_set<int64_t> getPlayerIds() const {
        std::unordered_set<int64_t> ret;
        if (m_prov)
            ret = m_prov->getPlayerIds(m_key);
        return ret;
    }
};
namespace model {
    struct EventSignupResponse {
        std::string m_riderStartTime;
        int m_signUpStatus = 0, m_riderSlot = 0;
        bool m_signedUp = false;
    };
    struct EventsSearch : public Json::Value {};
    /*
zwift_network::model::FirmwareRelease::FirmwareRelease(std::string const&,std::string const&,std::string const&,bool,std::string const&)
zwift_network::model::FirmwareRelease::FirmwareRelease(zwift_network::model::FirmwareRelease const&)
zwift_network::model::FirmwareRelease::getDeviceType(void)
zwift_network::model::FirmwareRelease::getMustPassThrough(void)
zwift_network::model::FirmwareRelease::getNotes(void)
zwift_network::model::FirmwareRelease::getUrl(void)
zwift_network::model::FirmwareRelease::getVersion(void)
zwift_network::model::FirmwareReleaseInfo::FirmwareReleaseInfo(long,std::string const&,std::string const&,std::string const&,bool,bool,std::string const&,std::string const&,std::string const&,std::string const&,std::string const&)
zwift_network::model::FirmwareReleaseInfo::FirmwareReleaseInfo(zwift_network::model::FirmwareReleaseInfo const&)
zwift_network::model::FirmwareReleaseInfo::getCountDevices(void)
zwift_network::model::FirmwareReleaseInfo::getCreatedDate(void)
zwift_network::model::FirmwareReleaseInfo::getDisplayName(void)
zwift_network::model::FirmwareReleaseInfo::getId(void)
zwift_network::model::FirmwareReleaseInfo::getIsForced(void)
zwift_network::model::FirmwareReleaseInfo::getMinVersion(void)
zwift_network::model::FirmwareReleaseInfo::getMustPassThrough(void)
zwift_network::model::FirmwareReleaseInfo::getNotes(void)
zwift_network::model::FirmwareReleaseInfo::getUpdatedDate(void)
zwift_network::model::FirmwareReleaseInfo::getVersion(void)
zwift_network::model::FirmwareReleaseInfo::~FirmwareReleaseInfo()
zwift_network::model::FirmwareRequest::FirmwareRequest(std::string const&,std::string const&,std::string const&,std::string const&)
zwift_network::model::FirmwareRequest::getDeviceSerial(void)
zwift_network::model::FirmwareRequest::getFirmwareVersion(void)
zwift_network::model::FirmwareRequest::getHardwareVersion(void)
zwift_network::model::FirmwareRequest::getType(void)
zwift_network::model::PlayerType::PlayerType(long,std::string const&)
zwift_network::model::PlayerType::getId(void)
zwift_network::model::PlayerType::getName(void)
zwift_network::model::Workout::Workout(std::string const&,std::string const&,std::chrono::time_point<std::chrono::system_clock,std::chrono::duration<long long,std::ratio<1l,1000000l>>>,zwift::protobuf::Sport)
zwift_network::model::Workout::Workout(zwift_network::model::Workout const&)
zwift_network::model::Workout::getId(void)
zwift_network::model::Workout::getName(void)
zwift_network::model::Workout::getScheduledDate(void)
zwift_network::model::Workout::getSport(void)
zwift_network::model::Workout::operator<(zwift_network::model::Workout const&)
zwift_network::model::Workout::operator==(zwift_network::model::Workout const&)
zwift_network::model::WorkoutsFromPartner::WorkoutsFromPartner(zwift_network::model::WorkoutPartnerEnum,std::multiset<zwift_network::model::Workout>)
zwift_network::model::WorkoutsFromPartner::WorkoutsFromPartner(zwift_network::model::WorkoutPartnerEnum,zwift_network::NetworkRequestOutcome)
zwift_network::model::WorkoutsFromPartner::getOutcome(void)
zwift_network::model::WorkoutsFromPartner::getPartner(void)
zwift_network::model::WorkoutsFromPartner::getWorkouts(void)*/
}
struct NetworkClient {
    NetworkClientImpl *m_pImpl;
    NetworkClient();
    ~NetworkClient();
    static void globalInitialize();
    static void globalCleanup();
    void initialize(const std::string &server, const std::string &certs, const std::string &version);
    std::future<NetworkResponse<std::string>> logInWithOauth2Credentials(const std::string &sOauth, const std::vector<std::string> &a4, const std::string &oauthClient);
    std::future<NetworkResponse<std::string>> logInWithEmailAndPassword(const std::string &email, const std::string &pwd, const std::vector<std::string> &anEventProps, bool reserved, const std::string &oauthClient);
    std::future<NetworkResponse<std::string>> logOut();
    std::future<NetworkResponse<void>> resetPassword(const std::string &newPwd);
    std::future<NetworkResponse<protobuf::PlayerState>> latestPlayerState(int64_t worldId, int64_t playerId);
    std::future<NetworkResponse<void>> removeFollowee(int64_t playerId, int64_t followeeId);
    std::future<NetworkResponse<protobuf::SocialNetworkStatus>> addFollowee(int64_t playerId, int64_t followeeId, bool a5, protobuf::ProfileFollowStatus pfs);
};
namespace ZNet {
    struct Error {
        Error(std::string_view msg, NetworkRequestOutcome netReqOutcome) : m_msg(msg), m_netReqOutcome(netReqOutcome), m_hasNetReqOutcome(true) {}
        Error(std::string_view msg) : m_msg(msg) {}

        std::string_view m_msg;
        NetworkRequestOutcome m_netReqOutcome = NRO_OK;
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