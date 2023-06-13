#pragma once
struct TimingArchEntity {
    //TODO
};
struct SegmentResultsPair : public std::list<std::pair<protobuf::SegmentResult, protobuf::SegmentResult>> {
    TimingArchEntity *m_tae;
};
struct SegmentResultsWrapper { //0x140 bytes
    int64_t m_hash;
    protobuf::SegmentResults m_srs;
    std::list<protobuf::SegmentResult> m_srList1, m_srList2;
    SegmentResultsPair m_srpList;
    uint32_t m_time = 0;
    char gap[28];
    bool m_hasFuture = false, m_field_21 = false;
    std::future<NetworkResponse<protobuf::SegmentResults>> m_future;
    uint64_t m_gap;
};
struct BadGuyInfo {
    int64_t m_playerId;
    double m_worldTimeSec;
    float m_timeToLive;
    //not used int field_14;
};
struct Leaderboards { //0x68 bytes
    std::list<SegmentResultsWrapper> m_srwList;
    std::list<BadGuyInfo> m_sandbaggers, m_cheaters;
    std::vector<ZNet::RequestId> m_segSaveReqs;
    static void FetchJerseyLeadersForAllSegments();
    void SaveUserSegmentPR(int64_t playerId, int64_t segmentId, double wtSec, float elapsedSec, float avg_power, bool isMale, protobuf::PowerType powTy, uint32_t weightGrams, const std::string &firstName, const std::string &lastName, uint64_t eventId);
    void UserLeftWorld(int64_t playerId);
    void FlagSandbagger(int64_t id, double, float);
    void FlagCheater(int64_t id, double, float);
    static void SetPlayerSegmentResult(protobuf::SegmentResult *sr, int64_t playerIdTx, int64_t segmentId, double wtSec, float durSec, float avgPower, bool isMale, protobuf::PowerType pty,
        uint32_t grams, const char *, const char *, uint64_t eventId, float avgHr, int64_t actId);
} inline g_Leaderboards;
/*
Leaderboards::FetchPlayerSegmentData(long,long long)
Leaderboards::FindUserResult(std::list<zwift::protobuf::SegmentResult> *,long long)
Leaderboards::GetSegmentData(long long)
Leaderboards::GetSegmentLeaderChanges(long long)
Leaderboards::GetSegmentResultsLocalPlayer(long long)
Leaderboards::GetSegmentResultsMapSynced(long long)
Leaderboards::GetSegmentResultsSynced(long long)
Leaderboards::IsCheater(long long)
Leaderboards::IsSandbagger(long long)
Leaderboards::Leaderboards(void)
Leaderboards::QueueSegmentLeaderChange(long long,zwift::protobuf::SegmentResult,zwift::protobuf::SegmentResult)
Leaderboards::RegisterLocalPlayersSegmentResult(long long,double,float,float,bool,float,ulong long)
Leaderboards::ReportPlayerSegmentResult(long long,double,float,float,bool,bool,ulong long)
Leaderboards::Update(long long)
Leaderboards::UpdateJerseys(Entity *)
Leaderboards::~Leaderboards()
*/