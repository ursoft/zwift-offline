#pragma once //UT Coverage: 99%, 200/203, ENOUGH
enum ProfileProperties { PP_EMAIL, PP_PASSWORD, PP_FIRST_NAME, PP_LAST_NAME };
enum ValidateProperty {
    VP_OK = 0, VP_PARSE_ERROR, VP_EMAIL_REQ = 2, VP_EMAIL_WRONG_LEN = 3, VP_EMAIL_SHORT = 4, VP_EMAIL_LONG = 5, VP_EMAIL_FORMAT = 6,
    VP_EMAIL_UNEQ = 7, VP_PROF_REQ = 8, VP_PROF_UN = 9, VP_PROF_NN = 10, VP_PROF_UNEQ = 11, VP_PROF_INV = 12,
    VP_PSW_REQ = 13, VP_PSW_LEN = 14, VP_PSW_SHORT = 15, VP_PSW_LONG = 16, VP_PSW_FORMAT = 17, VP_PSW_UNEQ = 18,
    VP_FN_REQ = 19, VP_FN_LEN = 20, VP_FN_SHORT = 21, VP_FN_LONG = 22,
    VP_LN_REQ = 23, VP_LN_LEN = 24, VP_LN_SHORT = 25, VP_LN_LONG = 26
};
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
    NRO_HTTP_STATUS_BANDWIDTH_LIMIT_EXCEEDED = 509, NRO_CNT };
std::string_view NetworkRequestOutcomeToString(NetworkRequestOutcome code);
void shutdown_zwift_network();
template<class T>
struct Optional {
    Optional() {}
    Optional(T &&src) { m_T = src; m_hasValue = true; }
    Optional(const T &src) { m_T = src; m_hasValue = true; }
    T m_T{};
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
template<typename R> bool is_ready(const std::future<R> &f) { return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready; }
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
zwift_network::model::Workout::Workout(std::string const&,std::string const&,std::chrono::time_point<std::chrono::system_clock,std::chrono::duration<long long,std::ratio<1l,1000000l>>>,protobuf::Sport)
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
namespace zwift_network {
    struct Motion { //32 bytes
        float m_ptg_f3 = 0.0f;
        float m_ptg_f4 = 0.0f;
        float m_ptg_f5 = 0.0f;
        float m_phoneRot = 0.0f;
        float m_ptg_f7 = 0.0f;
        float m_ptg_f8 = 0.0f;
        double m_ptg_f9 = 0.0;
    };
    bool is_paired_to_phone();
    bool pop_phone_to_game_command(protobuf::PhoneToGameCommand *pDest);
    bool pop_player_id_with_updated_profile(int64_t *ret);
    NetworkRequestOutcome send_ble_peripheral_request(const protobuf::BLEPeripheralRequest &rq);
    NetworkRequestOutcome send_default_activity_name_command(const std::string &name);
    NetworkRequestOutcome send_game_packet(const std::string &a2, bool force);
    NetworkRequestOutcome send_image_to_mobile_app(const std::string &pathName, const std::string &imgName);
    NetworkRequestOutcome send_mobile_alert(const protobuf::MobileAlert &ma);
    NetworkRequestOutcome send_mobile_alert_cancel_command(const protobuf::MobileAlert &ma);
    NetworkRequestOutcome send_player_profile(const protobuf::PlayerProfile &pp);
    NetworkRequestOutcome send_set_power_up_command(const std::string &locName, const std::string &color, const std::string &mask, int puId);
    NetworkRequestOutcome send_social_player_action(const protobuf::SocialPlayerAction &spa);
    NetworkRequestOutcome send_rider_list_entries(const std::list<protobuf::RiderListEntry> &list);
    bool pop_world_attribute(protobuf::WorldAttribute *dest);
    NetworkRequestOutcome setTeleportingAllowed(bool a);
    std::list<ValidateProperty> parse_validation_error_message(const std::string &msg);
    bool motion_data(Motion *dest);
    time_t from_iso_8601(const std::string &sdt);
    std::string to_iso_8601(time_t dt);
    std::future<NetworkResponse<protobuf::DropInWorldList>> fetch_drop_in_world_list();
    std::future<NetworkResponse<protobuf::DropInWorldList>> fetch_worlds_counts_and_capacities();
    ValidateProperty validateProperty(ProfileProperties pp, const std::string &ppval);
    std::future<NetworkResponse<protobuf::Goals>> get_goals(int64_t playerId);
    std::future<NetworkResponse<protobuf::Goal>> save_goal(const protobuf::Goal &);
    std::future<NetworkResponse<std::string>> log_out();
    std::future<NetworkResponse<void>> reset_password(const std::string &newPwd);
    std::future<NetworkResponse<protobuf::PlayerState>> latest_player_state(int64_t worldId, int64_t playerId);
    std::future<NetworkResponse<Json::Value>> get_activity_recommendations(const std::string &aGoal);
    std::future<NetworkResponse<protobuf::PowerCurveAggregationMsg>> get_best_efforts_power_curve_from_all_time();
    NetworkRequestOutcome send_activate_power_up_command(int powerupId, int powerupParam);
    NetworkRequestOutcome unsubscribe_from_segment(int64_t id);
    NetworkRequestOutcome send_customize_action_button_command(uint32_t a2, uint32_t a3, char *a4, char *a5, bool a6);
    bool pop_server_to_client(std::shared_ptr<protobuf::ServerToClient> &dest);
    std::future<NetworkResponse<bool>> create_subgroup_registration(int64_t id);
    std::future<NetworkResponse<int64_t>> create_activity_ride_on(int64_t playerIdSender, int64_t playerIdTarget);
    std::future<NetworkResponse<void>> accept_private_event_invitation(int64_t id);
    std::future<NetworkResponse<void>> create_race_result_entry(const protobuf::RaceResultEntrySaveRequest &rq);
    std::future<NetworkResponse<protobuf::ZFileProto>> create_zfile(const protobuf::ZFileProto &p);
    std::future<NetworkResponse<protobuf_bytes>> download_zfile(int64_t id);
    std::future<NetworkResponse<void>> erase_zfile(int64_t id);
    std::future<NetworkResponse<protobuf::Achievements>> get_achievements();
    std::future<NetworkResponse<protobuf::ListPublicActiveCampaignResponse>> get_active_campaigns();
    std::future<NetworkResponse<protobuf::EventsProtobuf>> get_events(const model::EventsSearch &es);
    std::future<NetworkResponse<protobuf::FeatureResponse>> get_feature_response(const protobuf::FeatureRequest &rq);
    std::future<NetworkResponse<protobuf::LateJoinInformation>> get_late_join_information(int64_t meetupId);
    std::future<NetworkResponse<protobuf::PlaybackMetadataList>> get_my_playbacks(int64_t a2);
    std::future<NetworkResponse<protobuf::PlaybackData>> get_playback_data(const protobuf::PlaybackMetadata &md);
    std::future<NetworkResponse<protobuf::PrivateEventProto>> get_private_event(int64_t id);
    std::future<NetworkResponse<protobuf::ZFilesProto>> list_zfiles(const std::string &folder);
    std::future<NetworkResponse<protobuf::ProfileEntitlements>> my_profile_entitlements();
    std::future<NetworkResponse<void>> reject_private_event_invitation(int64_t id);
    std::future<NetworkResponse<void>> reset_my_active_club();
    std::future<NetworkResponse<std::string>> save_playback(const protobuf::PlaybackData &data);
    std::future<NetworkResponse<void>> save_route_result(const protobuf::RouteResultSaveRequest &r);
    std::future<NetworkResponse<int64_t>> save_segment_result(const protobuf::SegmentResult &sr);
    std::future<NetworkResponse<void>> set_my_active_club(const protobuf::UUID &id);
    std::future<NetworkResponse<void>> unlock_achievements(const protobuf::AchievementUnlockRequest &rq);
    std::future<NetworkResponse<protobuf::CampaignRegistrationResponse>> enroll_in_campaign_v2(const std::string &sn);
    std::future<NetworkResponse<protobuf::ListCampaignRegistrationSummaryResponse>> get_campaigns_v2();
    std::future<NetworkResponse<protobuf::CampaignRegistrationResponse>> get_registration_in_campaign_v2(const std::string &sn);
    std::future<NetworkResponse<protobuf::EventsProtobuf>> get_events_in_interval(const std::string &start, const std::string &end, int limit);
    std::future<NetworkResponse<protobuf::PlaybackMetadata>> get_my_playback_latest(int64_t a2, uint64_t after, uint64_t before);
    std::future<NetworkResponse<protobuf::PlaybackMetadata>> get_my_playback_pr(int64_t a2, uint64_t after, uint64_t before);
    std::future<NetworkResponse<protobuf::Clubs>> list_my_clubs(Optional<protobuf::Membership_Status> status, Optional<int> start, Optional<int> limit);
    std::future<NetworkResponse<protobuf::PrivateEventFeedListProto>> private_event_feed(int64_t start_date, int64_t end_date, Optional<protobuf::EventInviteStatus> status, bool organizer_only_past_events);
    std::future<NetworkResponse<void>> create_user(const std::string &email, const std::string &pwd, const std::string &firstN, const std::string &lastN);
    std::future<NetworkResponse<protobuf::PlayerProfile>> my_profile();
    std::future<NetworkResponse<int64_t>> save_world_attribute(const protobuf::WorldAttribute &wa);
    std::future<NetworkResponse<protobuf::EventProtobuf>> get_event(int64_t id);
    std::future<NetworkResponse<protobuf::PlayerSocialNetwork>> get_followees(int64_t profileId, bool followRequests);
    std::future<NetworkResponse<protobuf::RaceResultSummary>> get_subgroup_race_result_summary(int64_t sid);
    std::future<NetworkResponse<bool>> register_for_event_subgroup(int64_t id);
    std::future<NetworkResponse<std::string>> log_in_with_oauth2_credentials(const std::string &sOauth, const std::vector<std::string> &a4, const std::string &oauthClient);
    std::future<NetworkResponse<std::string>> log_in_with_email_and_password(const std::string &email, const std::string &pwd, const std::vector<std::string> &anEventProps, bool reserved, const std::string &oauthClient);
    std::future<NetworkResponse<void>> remove_followee(int64_t playerId, int64_t followeeId);
    std::future<NetworkResponse<protobuf::SocialNetworkStatus>> add_followee(int64_t playerId, int64_t followeeId, bool a5, protobuf::ProfileFollowStatus pfs);
    std::future<NetworkResponse<model::EventSignupResponse>> create_subgroup_signup(int64_t id);
    std::future<NetworkResponse<protobuf::ZFileProto>> create_zfile_gzip(const std::string &name, const std::string &folder, const std::string &filePath);
    std::future<NetworkResponse<void>> delete_activity(int64_t playerId, int64_t actId);
    std::future<NetworkResponse<bool>> delete_subgroup_signup(int64_t id);
    std::future<NetworkResponse<bool>> remove_signup_for_event(int64_t id);
    std::future<NetworkResponse<void>> save_time_crossing_start_line(int64_t eventId, const protobuf::CrossingStartingLineProto &csl);
    std::future<NetworkResponse<model::EventSignupResponse>> signup_for_event_subgroup(int64_t eventId);
    std::future<NetworkResponse<protobuf::PlayerProfiles>> get_event_subgroup_entrants(protobuf::EventParticipation ep, int64_t eventId, uint32_t limit);
    std::future<NetworkResponse<int64_t>> save_activity_image(int64_t profileId, const protobuf::ActivityImage &img, const std::string &imgPath);
    std::future<NetworkResponse<void>> remove_goal(int64_t playerId, int64_t goalId);
    std::future<NetworkResponse<int64_t>> save_activity(const protobuf::Activity &act, bool uploadToStrava, const std::string &fitPath);
    std::future<NetworkResponse<protobuf::SegmentResults>> query_segment_results(int64_t serverRealm, int64_t segmentId, const std::string &from, const std::string &to, bool full);
    std::future<NetworkResponse<void>> update_profile(bool inGameFields, const protobuf::PlayerProfile &prof, bool udp);
    std::future<NetworkResponse<protobuf::ActivityList>> get_activities(int64_t profileId, const Optional<int64_t> &startsAfter, const Optional<int64_t> &startsBefore, bool fetchSnapshots);
    std::future<NetworkResponse<protobuf::PlayerProfile>> profile(int64_t profileId, bool bSocial);
    std::future<NetworkResponse<protobuf::SegmentResults>> subscribe_to_segment_and_get_leaderboard(int64_t sid);
    std::future<NetworkResponse<protobuf::SegmentResults>> get_segment_jersey_leaders();
    uint64_t world_time();
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
struct NetworkClient {
    NetworkClientImpl *m_pImpl;
    NetworkClient();
    ~NetworkClient();
    static void globalInitialize();
    static void globalCleanup();
    void initialize(const std::string &server, const std::string &certs, const std::string &version);
};
namespace ZNet {
    using RequestId = uint64_t;
    inline static const size_t MAX_IDS = 100;
    struct Error {
        Error(std::string_view msg, NetworkRequestOutcome netReqOutcome) : m_msg(msg), m_netReqOutcome(netReqOutcome), m_hasNetReqOutcome(true) {}
        Error(std::string_view msg) : m_msg(msg) {}
        std::string_view m_msg;
        NetworkRequestOutcome m_netReqOutcome = NRO_OK;
        bool m_hasNetReqOutcome = false;
    };
    struct RetryParams {
        int (*m_backoffMultiplier)(int) = nullptr;
        uint32_t m_count = 0, m_timeout = 0;
    };
    struct Params { //0x70 bytes
        std::string_view m_funcName;
        std::function<void(Error)> m_onError;
        RetryParams m_retry{};
        uint32_t m_timeout = 0;
        bool m_has_retry = false;
    };
    template<typename T> struct TOnReady { using type = std::function<void(const T &)>; };
    template<> struct TOnReady<void> { using type = std::function<void(void)>; };
    struct RPCBase {
        virtual std::pair<NetworkRequestOutcome, bool> Post(bool blocking) = 0;
        virtual bool ShouldRemove(uint32_t dt) = 0;
        virtual ~RPCBase() {}
    };
    /* OMIT: used in analytics and campaigns only; template<typename T>
    struct NetworkFetcher<T> {
        void Update();
        bool WaitDone(uint32_t to) {
            if (!*((_QWORD *)this + 7))
                return false;
            v4 = std::chrono::steady_clock::now(this);
            v5 = Update();
            v9 = 0LL;
            v11 = 1000000 * to;
            if (to >= 1) {
                do {
                    if (v5 != 1)
                        break;
                    sched_yield();
                    v12 = Update();
                    v5 = (int)v12;
                    v9 = std::chrono::steady_clock::now(v12) - v4;
                } while (v9 < v11);
            }
            if (v9 >= v11)
                Log("[ZNet] Future timed out after [%ums]!", v9, v6, v7, v8);
            return v5 == 2;
        }
    };*/
    template<typename T> struct RPC : public RPCBase {
        std::function<std::future<NetworkResponse<T>>(void)> m_futureCreator;
        TOnReady<T>::type m_onSuccess;
        std::string_view m_parFuncName;
        std::function<void(Error)> m_onError;
        std::future<NetworkResponse<T>> m_future;
        uint32_t m_wrkTime = 0, m_timeout = 0;
        RPC(std::function<std::future<NetworkResponse<T>>(void)> &&fc, TOnReady<T>::type &&fsucc, Params *p) :
            m_futureCreator(fc), m_onSuccess(fsucc), m_parFuncName(p->m_funcName), m_onError(std::move(p->m_onError)), m_timeout(p->m_timeout) { 
            m_future = m_futureCreator();
            zassert(m_parFuncName.length() || "RPC enqueued without a name, expect inferior debugging"); 
        }
        std::pair<NetworkRequestOutcome, bool> Post(bool blocking) override  { //vptr[2]
            /*if (m_future == nullptr) {
                Log("[ZNet] Invalid future! [%s]", m_parFuncName.data());
LABEL_56:
                if (m_onError)
                    m_onError(Error{ "Invalid Network Future" });
LABEL_57:
                return { 0, false };
            }*/
            if (blocking)
                if(!m_future._Ptr()->_Is_ready()) m_future.wait();
            if (!m_future._Ptr()->_Is_ready() && !is_ready(m_future)) {
                if (!m_timeout || m_wrkTime < m_timeout)
                    zassert(!"Must have timed out if the future is valid but not ready."); //UT indulgence
                Log("[ZNet] Timed Out! [%s] [%d ms]", m_parFuncName.data(), m_wrkTime);
                if (m_onError)
                    m_onError(Error{ "Timeout" });
                return { NRO_OK, false };
            }
            auto &result = m_future._Ptr()->_Get_value(false);
            /*if (result == nullptr ) - not possible now
            {
                Log("[ZNet] NetworkResponsePtr is null! [%s]", this->m_parFuncName.m_str, v11, v12, 0);
                v14 = this->m_onError.Ptrs;
                if (v14)
                {
                    v33 = *&off_7FF779BCE830;
                    BYTE4(v34) = 0;
                    v31 = *&off_7FF779BCE830;
                    v32 = v34;
                    (*(*v14 + 2))(v14, &v31);
                }
                *(ret + 4) = 0;
                return ret;
            }*/
            //OMIT CrashReporting::Instance()->AddBreadcrumb(v16, "[ZNet] get_outcome: ", this->m_parFuncName.m_str);
            if (result.m_errCode) {
                auto s = NetworkRequestOutcomeToString(result.m_errCode);
                Log("[ZNet] Unsuccessful network call %s! [%s] [%d ms]", s.data(), m_parFuncName.data(), m_wrkTime);
                if (m_onError)
                    m_onError(Error{ result.m_msg, result.m_errCode });
                return { result.m_errCode, true};
            }
            //nop("[ZNet] [%s] RPC completed successfully");
            zassert(result.m_errCode == 0 && "Invalid response!")
            if (m_onSuccess)
                if constexpr (std::is_same<T, void>::value)
                    m_onSuccess();
                else
                    m_onSuccess(result.m_T);
            return { NRO_OK, true };
        }
        bool ShouldRemove(uint32_t dt) override { //vptr[1]
            m_wrkTime += dt;
            if (!m_future.valid() || is_ready(m_future))
                return true;
            return m_timeout && m_wrkTime >= m_timeout;
        }
        virtual ~RPC() {} //vptr[0]
    };
    template<typename T>
    struct RetryableRPC : public RPC<T> {
        int32_t (*m_backoffMultiplier)(int32_t) = nullptr;
        int32_t m_backoffDelayInit = 5000, m_maxRetries = 5, m_remainRetries = 5, m_backoffDelay = 0;
        bool IsRetryable() {
            bool valid = this->m_future.valid();
            bool timedOut = this->m_timeout && (this->m_wrkTime >= this->m_timeout);
            bool notReady = !is_ready(this->m_future);
            bool ret = true;
            if (!valid || timedOut || notReady) {
                Log("[ZNet] RRPC::IsRetryable (%s) future in error state: (valid: %d) (timed out: %d) (not ready: %d)", this->m_parFuncName.data(), valid, timedOut, notReady);
            } else {
                auto outcome = PeekAtResponse();
                //if (outcome) {
                    if (!outcome || outcome == 400) {
                        ret = false;
                    } else {
                        Log("[ZNet] RRPC::IsRetryable (%s) outcome not ok: %d", this->m_parFuncName.data(), outcome);
                        ret = true;
                    }
                /*} else {
                    Log("[ZNet] RRPC::IsRetryable (%s) null future value", this->m_base.m_parFuncName.m_str, v5, v6, not_ready);
                    v7 = 1;
                }*/
            }
            return ret;
        }
        NetworkRequestOutcome PeekAtResponse() {
            return this->m_future._Ptr()->_Get_value(false).m_errCode;
        }
        bool ShouldRemove(uint32_t dt) override {
            if (m_backoffDelay > 0) {
                m_backoffDelay -= dt;
                if (m_backoffDelay > 0)
                    return false;
                Log("[ZNet] RRPC (%s) retrying...", this->m_parFuncName.data());
                this->m_future = this->m_futureCreator();
                return false;
            }
            this->m_wrkTime += dt;
            if (this->m_future.valid() && !is_ready(this->m_future))
                if (!this->m_timeout || this->m_wrkTime < this->m_timeout)
                    return false;
            if (!IsRetryable() || m_remainRetries <= 0)
                return true;
            Log("[ZNet] RRPC (%s) in error state, and initiating retry logic...", this->m_parFuncName.data());
            this->m_wrkTime = 0;
            --m_remainRetries;
            int backoff_index = 1;
            if (m_maxRetries - m_remainRetries > 1)
                backoff_index = m_maxRetries - m_remainRetries;
            m_backoffDelay = m_backoffDelayInit * (m_backoffMultiplier ? m_backoffMultiplier(backoff_index) : 1);
            Log("[ZNet] RRPC backoff index: %d, backoff delay milliseconds: %d", backoff_index, m_backoffDelay);
            return false;
        }
        RetryableRPC(std::function<std::future<NetworkResponse<T>>()> &&fc, TOnReady<T>::type &&fsucc, Params *p) : RPC<T>(std::move(fc), std::move(fsucc), p) {
            if (p->m_has_retry)
                m_backoffMultiplier = p->m_retry.m_backoffMultiplier;
        }
        ~RetryableRPC() {}
    };
    struct NetworkService {
        inline static std::unique_ptr<NetworkService> g_NetworkService;
        NetworkService() {}
        ~NetworkService() {} //vptr[0][0]
        static void Initialize() { g_NetworkService.reset(new NetworkService()); }
        static bool IsInitialized() { return g_NetworkService.get() != nullptr; }
        static NetworkService *Instance() { zassert(g_NetworkService.get() != nullptr); return g_NetworkService.get(); }
        static void Shutdown() { g_NetworkService.reset(); }
        RequestId GetProfile(int64_t playerId, std::function<void(const protobuf::PlayerProfile &)> &&func, Params *pParams); //vptr[0][1]
        RequestId UpdateProfile(bool inGameFields, const protobuf::PlayerProfile &prof, bool udp, std::function<void(void)> &&func, Params *pParams); //vptr[0][2]
        RequestId GetAllTimeBestEffortsPowerCurve(std::function<void(const protobuf::PowerCurveAggregationMsg &)> &&f, Params *pParams); //vptr[2][1]
        /* OMIT RequestId GetWorkoutSummaries(std::optional<std::string>, std::optional<std::string>, std::function<void(const protobuf::WorkoutSummaries &)>, Params *pParams)
OMIT RequestId GetAssetSummary(const std::string &, const std::function<void(protobuf::AssetSummary const &)> &, Params *pParams)
OMIT RequestId GetWorkout(const std::string const&,std::function<void (const std::string &)>, Params *pParams) */
    };
    struct API {
        std::mutex m_mutex;
        std::unordered_map<RequestId, std::unique_ptr<RPCBase>> m_map;
        uint64_t m_counter = 0;
        static API *Inst() { static API s_inst; return &s_inst; }
        template<typename T>
        RequestId Enqueue(std::function<std::future<NetworkResponse<T>>(void)> &&f, TOnReady<T>::type &&ready_f, Params *p) {
            std::unique_ptr<RPCBase> r;
            if (p->m_has_retry)
                r.reset(new RetryableRPC<T>(std::move(f), std::move(ready_f), p));
            else
                r.reset(new RPC<T>(std::move(f), std::move(ready_f), p));
            RequestId ret = 0;
            if (r) {
                std::lock_guard l(m_mutex);
                ret = ++m_counter;
                m_map.emplace(ret, std::move(r));
            }
            return ret;
        }
        bool Dequeue(RequestId req) {
            std::lock_guard l(m_mutex);
            return m_map.erase(req) > 0;
        }
        //absent in PC? void Blocking_Send(const std::function<std::future<NetworkResponse<void>(void)>> &f, const Params &);
        bool GetStatus(RequestId r) {
            std::lock_guard l(m_mutex);
            auto f = m_map.find(r);
            if (f == m_map.end())
                return false;
            return true;
        }
        bool IsBusy() {
            std::lock_guard l(m_mutex);
            return !m_map.empty();
        }
        void Update(uint32_t dt);
    };
    bool WaitForPendingRequests(const std::vector<RequestId> &vec, std::string_view sv);
    RequestId UpdateProfile(bool inGameFields, const protobuf::PlayerProfile &prof, bool udp, std::function<void(void)> &&f, Params *pParams);
    RequestId GetProfile(int64_t playerId, std::function<void(const protobuf::PlayerProfile &)> &&func, Params *pParams);
    RequestId GetProfiles(const std::unordered_set<int64_t> &ids, std::function<void (const protobuf::PlayerProfiles &)> &&f, Params *pParams);
    RequestId DeleteActivity(int64_t, uint64_t, std::function<void (void)> &&f, std::function<void (Error)> &&ef);
    RequestId DownloadPlayback(const protobuf::PlaybackMetadata &proto, std::function<void (const protobuf::PlaybackData &)> &&f, std::function<void (Error)> &&ef);
    RequestId EnrollInCampaign(std::string &proto, std::function<void (const protobuf::CampaignRegistrationResponse &)> &&f, std::function<void (Error)> &&ef, Params *pParams);
    RequestId FetchSegmentJerseyLeaders(std::function<void (const protobuf::SegmentResults &)> &&f, std::function<void (Error)> &&ef);
    RequestId GetAchievements(std::function<void (const protobuf::Achievements &)> &&f, std::function<void (Error)> &&ef);
    RequestId GetActiveCampaigns(std::function<void (const protobuf::ListPublicActiveCampaignResponse &)> &&f, std::function<void (Error)> &&ef);
    RequestId GetActivities(int64_t, std::function<void (const protobuf::ActivityList &)> &&f, Params *pParams);
    RequestId GetCampaignRegistration(std::string &proto, std::function<void (const protobuf::CampaignRegistrationResponse &)> &&f, std::function<void (Error)> &&ef, Params *pParams);
    RequestId GetCampaigns(std::function<void (const protobuf::ListCampaignRegistrationSummaryResponse &)> &&f, Params *pParams);
    RequestId GetClubList(std::function<void (const protobuf::Clubs &)> &&f, Params *pParams);
    RequestId GetDropInWorldList(std::function<void (const protobuf::DropInWorldList &)> &&f, Params *pParams);
    RequestId GetFeatureVariant(const protobuf::FeatureRequest &&, std::function<void (const protobuf::FeatureResponse)> &&f, std::function<void (Error)> &&ef);
    RequestId GetFollowees(int64_t, bool, std::function<void (const protobuf::PlayerSocialNetwork &)> &&f, Params *pParams);
    RequestId GetGroupEvent(int64_t, std::function<void (const protobuf::EventProtobuf &)> &&f, std::function<void (Error)> &&ef);
    RequestId GetMyPlaybackLatest(int64_t, uint64_t, uint64_t, std::function<void (const protobuf::PlaybackMetadata &)> &&f, std::function<void (Error)> &&ef);
    RequestId GetMyPlaybackPr(int64_t, uint64_t, uint64_t, std::function<void (const protobuf::PlaybackMetadata &)> &&f, std::function<void (Error)> &&ef);
    RequestId GetMyPlaybacks(int64_t, std::function<void (const protobuf::PlaybackMetadataList &)> &&f, std::function<void (Error)> &&ef);
    RequestId GetPlayerState(int64_t, int64_t, std::function<void (const protobuf::PlayerState &)> &&f, Params *pParams);
    RequestId GetProfile(std::function<void (const protobuf::PlayerProfile &)> &&f, Params *pParams);
    RequestId GetProfile(std::string &proto, std::function<void (const protobuf::PlayerProfile &)> &&f, Params *pParams);
    RequestId GetProfile(std::string_view, std::function<void (const protobuf::PlayerProfile &)> &&f, Params *pParams);
    RequestId GetProgressInCampaign(std::string &proto, std::function<void (const protobuf::CampaignRegistrationDetailResponse &)> &&f, std::function<void (Error)> &&ef, Params *pParams);
    RequestId RegisterInCampaign(std::string &proto, std::function<void (const protobuf::CampaignRegistrationResponse &)> &&f, std::function<void (Error)> &&ef, Params *pParams);
    RequestId SaveActivity(const protobuf::Activity &, bool, const std::string &s, std::function<void (int64_t)> &&f, std::function<void (Error)> &&ef);
    RequestId SavePlayback(const protobuf::PlaybackData &proto, std::function<void (const std::string &)> &&f, std::function<void (Error)> &&ef);
    RequestId SaveSegmentResult(const protobuf::SegmentResult &proto, std::function<void (const int64_t &)> &&f, std::function<void (Error)> &&ef);
    RequestId SubscribeToRouteSegment(int64_t, std::function<void (const protobuf::SegmentResults &)> &&f, std::function<void (Error)> &&ef);
    RequestId UnlockAchievements(const std::vector<int> &proto, std::function<void (void)> &&f, std::function<void (Error)> &&ef);
    RequestId WithdrawFromCampaign(std::string &proto, std::function<void (const protobuf::CampaignRegistrationResponse &)> &&f, std::function<void (Error)> &&ef, Params *pParams);
    /* OMIT: analytics, campaign template<typename T>
    NetworkResponse<T> TryGet(std::future<NetworkResponse<T>> &fut) {
        if (fut.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            return fut.get();
        } else {
            ret[10] = 1; return ret;// NetworkResponse<T>{NRO_}
        }
    }*/
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
NetworkRequestOutcome ZNETWORK_ClearPlayerPowerups();
enum PLAYER_FLAGGED_REASONS { PFR_FLIER = 1, PFR_HARASSER = 2, PFR_POTTY_MOUTH = 3, PFR_SANDBAGGER = 4, PFR_CNT };
void ZNETWORK_BroadcastLocalPlayerFlagged(PLAYER_FLAGGED_REASONS);
enum RideLeaderAction : int { RLA_1 = 1 /*SaveGame::TryPromoCode, GAME_QuitGroupEvent*/, RLA_2 = 2, RLA_3 = 3, RLA_4 = 4, RLA_5 = 5, RLA_6 = 6 /*GAME_QuitGroupEvent*/ };
void ZNETWORK_BroadcastRideLeaderAction(RideLeaderAction, uint32_t, uint64_t);
struct ZNETWORK_RouteHashRequest {
    int16_t m_ver, m_len;
    char m_kind, field_5, field_6, field_7;
    int64_t m_otherPlayerId, m_playerIdTx;
    uint32_t m_routeHash, m_decisionIndex;
};
struct ZNETWORK_LateJoinRequest { //0x28 bytes
    uint16_t m_ver, m_len;
    enum cmd { LJC_1 = 1, LJC_2 = 2 };
    cmd m_cmd;
    int64_t m_lateJoinPlayerId, m_playerId;
    int32_t m_decisionIndex, m_cb_a3;
    uint32_t m_cb_a4;
};
struct ZNETWORK_GRFenceRiderStats { //0x20 bytes
    uint16_t m_ver, m_len;
    int field_4;
    uint64_t m_field_8;
    int64_t m_playerIdTx;
    bool m_field_18, m_field_19;
    int16_t field_1A[2];
    int16_t field_1E;
};
struct ZNETWORK_GRFenceConfig { //48 bytes
    uint16_t m_ver, m_len;
    int field_4;
    uint64_t m_field_8;
    float m_field_10, m_field_14, m_field_18, m_field_1C, m_field_20;
    uint32_t m_field_24, m_field_28;
    bool m_field_2C;
    char field_2D[3];
};
struct ZNETWORK_BibNumberForGroupEvent {
    int16_t m_ver, m_len;
    char m_data[4];
    int64_t m_key, m_eventId;
    uint32_t m_val, m_1C;
    double m_wt_sec;
};
struct ZNETWORK_PacePartnerInfo { //32 bytes
    int64_t m_playerIdTx, m_playerId;
    uint16_t m_ver, m_len;
    float m_float;
    enum BroadcastState : int16_t { BS_1 = 1, BS_2 = 2, BS_5 = 5, BS_10 = 10 };
    BroadcastState m_bcs;
    uint16_t field_1A = 0; //UT indulgence
    int field_1C = 0; //UT indulgence
};
struct ZNETWORK_LateJoinResponse { //0x28 bytes - maybe, = ZNETWORK_LateJoinRequest
    uint16_t m_ver, m_len;
    uint8_t m_field_4;
    //then 4 byte-gap
    int64_t m_playerId, m_lateJoinPlayerId;
    int m_decisionIndex, m_field_1C, m_field_20; //then 4 byte-gap
};
struct ZNETWORK_TextMessage { //128*9+48=1200 bytes
    VEC3 m_msgPos;
    float m_msgRadius;
    int64_t m_srcProfileId, m_destProfileId, m_worldTime, m_xxx;
    UChar m_msg[512]; //not sure, looks like 2-bytes char, but sent to printf as %s
    char field_430[128]; //not sure
};
struct WebEventStartMsg { //0x308 bytes
    uint16_t m_ver, m_len;
};
struct BroadcastRideLeaderAction { //0x68 bytes
    int16_t m_ver, m_len;
    int64_t m_leaderId;
    time_t m_worldTime;
    uint64_t m_eventId;
    RideLeaderAction m_rideLeaderAction;
    char field_24[68];
};
struct BroadcastLocalPlayerNotableMoment {
    int16_t m_ver, m_len;
    int64_t m_playerIdTx;
    time_t m_worldTime;
    NOTABLEMOMENT_TYPE m_nmt;
    uint32_t m_field_1C;
    int gap20;
    float m_field_24;
    uint64_t field_28;
};
struct RideLeaderActionInfo { //0x20 bytes
    uint64_t m_eventId;
    int64_t m_leaderId;
    time_t m_worldTime, m_worldTimeExpire;
};
inline std::list<RideLeaderActionInfo *> g_RideLeaderActions;
void ZNETWORK_RegisterRideLeader(uint64_t eventId, int64_t leaderId, time_t world_time, time_t world_time_expire);
//inlined void ZNETWORK_INTERNAL_HandlePacePartnerInfo(const ZNETWORK_PacePartnerInfo &);
void ZNETWORK_FlagLocalPlayer(PLAYER_FLAGGED_REASONS, bool = true);
//android-only? void ZNETWORK_SetPreferredNetwork(int32_t, int32_t);
//android-only? void ZNETWORK_GetPreferredNetwork();
//android-only? void ZNETWORK_INTERNAL_GetHoursToAdd();
inline int  ZNETWORK_GetServerCount() { return 6; } //android-only and/or inlined?
//simplified void ZNETWORK_INTERNAL_Log(const char *);
//android-only? void ZNETWORK_IsOnProductionServer();
void ZNETWORK_Initialize();
//android-only? void ZNETWORK_GetServerURL(int32_t);
//simplified void ZNETWORK_ShutdownLogging();
//simplified void ZNETWORK_RestoreLogging();
//OMIT void ZNETWORK_GetAnalytics(uint32_t *, uint32_t *, uint32_t *);
//inlined void ZNETWORK_INTERNAL_HandleRouteHashRequest(const ZNETWORK_RouteHashRequest &);
void ZNETWORK_INTERNAL_HandleLateJoinRequest(const ZNETWORK_LateJoinRequest &ljr, const VEC3 &pos);
//inlined void ZNETWORK_INTERNAL_DelayLateJoinResponse(ZNETWORK_LateJoinRequest);
//inlined to {UI_FriendsList::SendFriendlistToPhone} void ZNETWORK_UpdateRiderList(std::list<protobuf::RiderListEntry> &);
inline int g_nPhantomRidesOnReceived, g_FlaggedAsPottyMouth, g_FlaggedAsHarasser, g_FlaggedAsFlier, g_currentEventTotalRiders;
inline float g_sandbaggerPenaltyScale;
inline uint32_t g_routehashCt;
inline int32_t g_ServerReportedPlayerCount;
inline bool g_ServerReportedRacePlacements;
inline bool ZNETWORK_RacePlacementsAvailable() { return g_ServerReportedRacePlacements; }
inline int ZNETWORK_RacePlacementTotalRiders() { return g_currentEventTotalRiders; }
inline int32_t ZNETWORK_GetTotalPlayerCount() { return g_ServerReportedPlayerCount; }
void ZNETWORK_INTERNAL_ProcessPlayerPackets();
void ZNETWORK_BroadcastGRFenceStats(uint64_t, int64_t, bool, bool /*, uint32_t*/);
void ZNETWORK_BroadcastGRFenceConfig(uint64_t, uint32_t, float, float, float, float, float, uint32_t, uint32_t, bool);
void ZNETWORK_BroadcastLocalPlayerNotableMoment(NOTABLEMOMENT_TYPE, uint32_t, uint32_t, float);
void ZNETWORK_BroadcastRegisterForGroupEvent(uint64_t, uint32_t, bool, bool);
void ZNETWORK_BroadcastBibNumberForGroupEvent(int64_t, uint32_t, uint32_t);
void ZNETWORK_SendPacePartnerInfo(int64_t, uint16_t, float, uint32_t);
void ZNETWORK_BroadcastAreaText(int64_t, const UChar *, const VEC3 &, float);
enum SubscriptionMode {
    SM_UNDEFINED = 0x0,
    SM_TRIAL = 0x1,
    SM_UNKNWN_SRC = 0x2,
    SM_NAMED_SRC = 0x3,
    SM_INACTIVE = 0x4,
};
inline SubscriptionMode g_subscriptionMode;
inline SubscriptionMode ZNETWORK_CalculateSubscriptionMode() { /*OMIT*/ return SM_NAMED_SRC; }
inline float ZNETWORK_GetTrialKMLeft() { return 100000.0f; }
inline int ZNETWORK_GetSubscriptionDaysLeft() { return -1; }
inline SubscriptionMode ZNETWORK_GetSubscriptionMode() { return g_subscriptionMode; }
void ZNETWORK_SendRouteHashRequest(int64_t toPlayerId);
void ZNETWORK_RespondToRouteHashRequest(int64_t);
void ZNETWORK_SendLateJoinRequest(int64_t);
void ZNETWORK_RespondToLateJoinRequest(int64_t);
void ZNETWORK_SendSPA(protobuf::SocialPlayerAction *, const VEC3 &, float, uint64_t);
void ZNETWORK_SendPlayerFlag(int64_t, int64_t, protobuf::SocialFlagType);
inline void ZNETWORK_UpdatePlayerEntitlement() { /*OMIT*/ }
inline SubscriptionMode ZNETWORK_CalculateThenGetSubscriptionMode() { return g_subscriptionMode; }
void ZNETWORK_SendPrivateText(int64_t, int64_t, const UChar *);
void ZNETWORK_INTERNAL_ProcessPhoneInput();
void ZNETWORK_INTERNAL_ProcessReceivedWorldAttribute(const protobuf::WorldAttribute &);
void ZNETWORK_Update(float);
bool ZNETWORK_IsLoggedIn();
void ZNETWORK_GivePlayerPowerup(protobuf::POWERUP_TYPE);
struct SegmentResultsWrapper;
struct TimingArchEntity;
SegmentResultsWrapper *ZNETWORK_RegisterSegmentID(int64_t hash, TimingArchEntity *tae = nullptr);
void ZNETWORK_Shutdown();
uint64_t ZNETWORK_GetNetworkSyncedTimeGMT();
struct RouteFinishData {
    uint64_t m_eventId = 0, m_world_time = 0, m_elapsed_ms = 0;
    float m_avg_power = 0.0f, m_max_power = 0.0f, m_avg_hr = 0.0f, m_max_hr = 0.0f, m_calories = 0.0;
    uint32_t m_routeHash = 0, m_height_in_cm = 0, m_weight_in_grams = 0, m_ftp = 0;
    protobuf::Sport m_sport = protobuf::CYCLING;
    protobuf::PlayerType m_player_type = protobuf::NORMAL;
    bool m_isMale = true, m_powerMeter = false, m_steering = false;
};
inline int64_t g_CurrentServerRealmID, g_ActivityID;
void ZNETWORK_RegisterLocalPlayersRouteResult(const RouteFinishData &rfd);
//inlined void ZNETWORK_INTERNAL_GetCPValues(float *, float *, float *, float *, float);
struct BikeEntity;
std::future<NetworkResponse<void>> ZNETWORK_RaceResultEntrySaveRequest(double time, float duration, BikeEntity *pBike, bool lj, float a6);
void ZNETWORK_JoinWorld(int64_t, bool);
void ZNETWORK_ResetSessionData();
inline const char *ZNETWORK_GetPromoNameOfCurrentRideEntitlement() { return "Ursoft premium"; }
inline const char *ZNETWORK_GetPromoNameOfNextRideEntitlement() { return "Ursoft platinum"; }
void ZNETWORK_GiveRideOn(int64_t playerTo, bool);
//inlined void ZNETWORK_INTERNAL_IsPEZwiftTrial(const protobuf::ProfileEntitlement &);
//inlined void ZNETWORK_INTERNAL_IsPEBlockedForPlatform(const protobuf::ProfileEntitlement &);
//OMIT void ZNETWORK_GetNextUnusedRideEntitlement(protobuf::ProfileEntitlement *);
//OMIT void ZNETWORK_UnlockDigitalEntitlements();
//OMIT void ZNETWORK_INTERNAL_GetCurrentEntitlement(protobuf::ProfileEntitlement *);
//OMIT void ZNETWORK_GetCurrentEntitlement(protobuf::ProfileEntitlement *);
//OMIT void ZNETWORK_GetNextTrialStartDate();
//OMIT void ZNETWORK_IsOnNewTrialSystem(const protobuf::ProfileEntitlement *);
//OMIT void ZNETWORK_SubscribedFromApple();
//OMIT void ZNETWORK_HasPromoAvailable();
//inlined void ZNETWORK_INTERNAL_ProcessMotionData();
//inlined void ZNETWORK_INTERNAL_ProcessGlobalMessages();
//inlined void ZNETWORK_INTERNAL_UpdateSubscriptionCache(float);
//inlined void ZNETWORK_INTERNAL_HandleDelayedRouteHashResponse(float);
//inlined void ZNETWORK_ClearLateJoinRequest();
//inlined void ZNETWORK_INTERNAL_HandleDelayedLateJoinResponse(float);
//inlined void ZNETWORK_GetUpcomingWorkouts();
//inlined void ZNETWORK_INTERNAL_ProcessPendingWorkouts();
//inlined void ZNETWORK_ResetTotalRideOnsGiven();
//inlined void ZNETWORK_TotalRideOnsGiven();
//inlined void ZNETWORK_ReceivedRideOns();
//ANDROID only ??? void ZNETWORK_GiveRideOnToSomebodyElse(int64_t);
//not called void ZNETWORK_INTERNAL_CalcSig(int64_t);
//inlined void ZNETWORK_INTERNAL_ProcessProfileUpdates();
//inlined void ZNETWORK_UnRegisterRideLeader(uint64_t, int64_t, uint64_t, uint64_t);
//inlined void ZNETWORK_IsInRaceEvent();
//inlined void ZNETWORK_IsInGroupWorkoutEvent();
//inlined void ZNETWORK_ReducedFlagging(int64_t);
//inlined void ZNETWORK_GetWeightedFlags(int64_t);
//inlined void ZNETWORK_GetReceivedRideOnFromUsers();
//inlined void ZNETWORK_ActivatePlayerPowerup(uint32_t);
bool ZNETWORK_HasGivenRideOnToPlayer(int64_t playerId);
//not found void ZNETWORK_GetServerNickname(int32_t);
//not found void ZNETWORK_GetOAuthClient();
//inlined void ZNETWORK_GetDropInWorlds();
//not found void ZNETWORK_InitializeNetworkSyncedGMT(int64_t);
//not found void ZNETWORK_GetNetworkSyncedDateTimeGMT();
//inlined void ZNETWORK_LastRideOnReceivedFromPlayerId();
void ZNETWORK_RegisterLocalPlayersSegmentResult(int64_t, float, float, bool, float);
void ZNETWORK_RegisterLocalPlayersSegmentResult(int64_t, double, float, float, bool, float);
protobuf::ActivityList *ZNETWORK_GetActivities();
void ZNETWORK_INTERNAL_ProcessUpcomingWorkouts();
void ZNETWORK_UpdateDropInWorldsStatus();

inline bool g_IsOnProductionServer = true;
inline NetworkClient *g_networkClient;
inline time_t g_lastBroadcastLocalPlayerFlagged, g_CachedWorldTime;
inline float g_startLineRoadTime = 0.95f;
inline uint32_t g_LastScreenshotTime;
inline int g_PhoneAPIVersion, g_farFanviewFocusRequestTimeout;
inline int64_t g_farFanviewFocusRequestID;
