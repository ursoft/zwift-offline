#include "ZwiftApp.h"
DataRecorder::DataRecorder(Experimentation *exp, ZNet::NetworkService *net, EventSystem *ev) : m_ev(ev), m_exp(exp), m_net(net) {
    auto c1 = GetComponent(RecorderComponent::T_1);
    if (c1)
        c1->Init(1.0f, 7200);
    auto c7 = GetComponent(RecorderComponent::T_CPC);
    if (c7)
        c7->Init(1.0f, 43201);
    auto c8 = GetComponent(RecorderComponent::T_8);
    if (c8)
        c8->Init(1.0f, 7200);
    m_graphers[0] = new Grapher(GetComponent(RecorderComponent::T_2));
    m_graphers[1] = new Grapher(GetComponent(RecorderComponent::T_9));
    m_graphers[3] = new Grapher(GetComponent(RecorderComponent::T_3));
    m_graphers[2] = new Grapher(GetComponent(RecorderComponent::T_0));
    m_graphers[4] = new Grapher(GetComponent(RecorderComponent::T_4));
    m_graphers[5] = new Grapher(GetComponent(RecorderComponent::T_CPC));
    GetComponent(RecorderComponent::T_DISTANCE);
}
float DataRecorder::ExpensiveCalculateTSS() {
    auto c1 = DataRecorder::Instance()->GetComponent(RecorderComponent::T_1);
    auto v3 = 0.0f;
    size_t num = 0;
    if (c1)
    {
        auto &dp = c1->GetDataPoints();
        num = dp.size();
        if (!num)
            return 0.0f;
        float *tmp = (float *)malloc(sizeof(float) * num);
        if (tmp) {
            int i = 0;
            float v9 = 0.0f, v14;
            do {
                if (i < 30) {
                    v14 = dp[i].m_data[0];
                    v9 += v14;
                } else {
                    v9 += dp[i].m_data[0] - dp[i - 30].m_data[0];
                    v14 = v9 / 30.0f;
                }
                tmp[i++] = v14;
            } while (i < num);
            i = 0;
            do {
                auto t = tmp[i++];
                v3 += 1.0 / (double)num * (t * t * t * t);
            } while (i < num);
            free(tmp);
        }
    }
    auto mainBike = BikeManager::Instance()->m_mainBike;
    auto ftp = mainBike->GetFTP();
    auto np = pow(v3, 0.25);
    auto if_ = np / ftp;
    auto tss = 0.02777777777777778 / ftp * num * if_ * np;
    Log("num=%d NP = %3.2f   IF = %3.2f   TSS = %3.2f", num, np, if_, tss);
    return tss;
}
float DataRecorder::ExpensiveCalculateTSS_EstimatedPower(const std::vector<float> &src) {
    int num = src.size();
    if (!num)
        return 0.0f;
    auto v5 = (float *)malloc(num * sizeof(float));
    int v6 = 0;
    float v8 = 0.0f;
    for (auto it : src) {
        if (v6 < 30) {
            v8 += it;
            v5[v6++] = it;
        } else {
            v8 += it - src[v6 - 30];
            v5[v6++] = v8 / 30.0f;
        }
    }
    double v12 = 0.0;
    for (int i = 0; i < num; i++) {
        auto t = v5[i];
        v12 += 1.0 / (double)num * (t * t * t * t);
    }
    free(v5);
    auto mainBike = BikeManager::Instance()->m_mainBike;
    auto ftp = mainBike->GetFTP();
    auto v16 = pow(v12, 0.25);
    return v16 * (double)num * (v16 / ftp) / (ftp * 3600.0) * 100.0;
}
RecorderComponent *DataRecorder::GetComponent(RecorderComponent::TYPE ty) {
    for (auto rc : m_components)
        if (rc->m_type == ty)
            return rc;
    RecorderComponent *rc = nullptr;
    if (ty == RecorderComponent::T_CPC)
        rc = new CriticalPowerCurve(m_net /*ZNet::ICriticalPowerCurveNetworkService &*/, m_exp, m_ev);
    else
        rc = new RecorderComponent(ty);
    if (rc)
        m_components.push_back(rc);
    else
        zassert(rc && "DataRecorder::GetComponent Asserted; Cannot allocated new RecorderComponent");
    return rc;
}
void DataRecorder::Init() { Grapher::s_reducedDataPointsCache = 6; }
void DataRecorder::ResetSessionData() {
    for (auto i : m_components)
        i->Reset();
    for (auto &i : m_graphers)
        i->Reset();
    /*TODO   v11 = qword_7FF76B05E120;
  if ( qword_7FF76B05E120 )
  {
    v12 = qword_7FF76B05E118;
    do
    {
      if ( --v11 )
        ++v12;
      else
        v12 = 0i64;
      qword_7FF76B05E118 = v12;
    }
    while ( v11 );
    qword_7FF76B05E120 = 0i64;
  }
  v13 = qword_7FF76B05E0E8;                     // unsigned int,std::pair<std::vector<DataPoint>,unsigned int>>
                                                // 
  std::_Tree_val<std::_Tree_simple_types<std::pair<int const,std::string>>>::_Erase_tree<std::allocator<std::_Tree_node<std::pair<int const,std::string>,void *>>>(
    &qword_7FF76B05E0E8,
    &qword_7FF76B05E0E8,
    *(qword_7FF76B05E0E8 + 8));
  v13[1] = v13;
  *v13 = v13;
  v13[2] = v13;
  qword_7FF76B05E0F0 = 0i64;*/
}
void DataRecorder::Update(float dt) {
    for (auto i : m_components)
        i->Update(dt);
}
DataRecorder::~DataRecorder() {
    for (auto i : m_components)
        delete i;
    m_components.clear();
    for (auto &i : m_graphers)
        FreeAndNil(i);
}
void RecorderComponent::Init(float timeBetweenPoints, uint32_t reserve) {
    m_timeBetweenPoints = timeBetweenPoints;
    m_nextPointTime = 0.0f;
    m_res0 = (reserve == 0);
    if (reserve && reserve > m_dataPoints.capacity())
        m_dataPoints.reserve(reserve);
    m_dataPoints.clear();
}
void RecorderComponent::Update(float) {
    auto mb = BikeManager::Instance()->m_mainBike;
    auto bc = mb->m_bc;
    if (bc->m_bool) {
        if (m_type == T_1 || !bc->m_bool1) {
            float val = 0.0f;
            VEC3 v14;
            int fpclass;
            if (m_nextPointTime <= 0.0f) {
                switch (m_type) {
                case T_0: case T_1:
                    if (bc->m_field_178 < 2300.0f) //looks like power
                        val = bc->m_field_178;
                    AddDataPoint(val, m_curTime);
                    break;
                case T_2:
                    fpclass = _fpclass(bc->m_field_188);
                    if (fpclass == _FPCLASS_PN || fpclass & _FPCLASS_NN || fpclass & _FPCLASS_NZ || fpclass & _FPCLASS_PZ)
                        val = bc->m_field_188;
                    AddDataPoint(val, m_curTime);
                    break;
                case T_3:
                    val = bc->m_field_198;
                    AddDataPoint(val, m_curTime);
                    break;
                case T_4:
                    val = (float)(int)bc->m_field_19C;
                    AddDataPoint(val, m_curTime);
                    break;
                case T_DISTANCE:
                    val = bc->m_distance;
                    AddDataPoint(val, m_curTime);
                    break;
                case T_6:
                    val = bc->m_field_1E0;
                    AddDataPoint(val, m_curTime);
                    break;
                case T_8:
                    AddDataPointVEC3(mb->GetPosition(), m_curTime);
                    break;
                case T_9:
                    if (bc->m_sport == protobuf::RUNNING) {
                        fpclass = _fpclass(bc->m_field_188);
                        if (fpclass == _FPCLASS_PN || fpclass & _FPCLASS_NN || fpclass & _FPCLASS_NZ || fpclass & _FPCLASS_PZ)
                            val = bc->m_field_188;
                        AddDataPoint(val, m_curTime);
                    }
                    break;
                default:
                    break;
                }
                m_nextPointTime += m_timeBetweenPoints;
            }
            auto v10 = float(m_type == T_1 ? bc->m_field_118 : bc->m_field_128) * 0.001f;
            m_nextPointTime -= (v10 - m_curTime);
            m_curTime = v10;
        }
    }
}
void RecorderComponent::Reset() {
    m_dataPoints.clear();
    m_dataPointsVec3.clear();
    m_valMax = m_valMax_init;
    m_valMin = m_valMin_init;
    m_v3max = m_v3max_init;
    m_v3min = m_v3min_init;
    m_startTime = 0.0f;
    m_maxTime = 0.0f;
    m_curTime = 0.0f;
}
void RecorderComponent::AddDataPoint(float val, float time) {
    if (!m_res0 && m_dataPoints.capacity() == m_dataPoints.size())
        m_dataPoints.reserve(m_dataPoints.size() * 3 / 2);
    m_dataPoints.push_back({ val, time });
    if (val > m_valMax)
        m_valMax = val;
    if (val < m_valMin)
        m_valMin = val;
    if (m_startTime == 0.0f) {
        m_startTime = time;
        m_maxTime = time + m_period;
    } else {
        if (time > m_maxTime)
            m_maxTime = time;
        if (time < m_startTime)
            m_startTime = time;
    }
}
void RecorderComponent::AddDataPointVEC3(const VEC3 &val, float time) {
    if (!m_res0 && m_dataPointsVec3.capacity() == m_dataPointsVec3.size())
        m_dataPointsVec3.reserve(m_dataPointsVec3.size() * 3 / 2);
    if (val.m_data[0] > m_v3max.m_data[0])
        m_v3max.m_data[0] = val.m_data[0];
    if (val.m_data[1] > m_v3max.m_data[1])
        m_v3max.m_data[1] = val.m_data[1];
    if (val.m_data[2] > m_v3max.m_data[2])
        m_v3max.m_data[2] = val.m_data[2];
    if (val.m_data[0] < m_v3min.m_data[0])
        m_v3min.m_data[0] = val.m_data[0];
    if (val.m_data[1] < m_v3min.m_data[1])
        m_v3min.m_data[1] = val.m_data[1];
    if (val.m_data[2] < m_v3min.m_data[2])
        m_v3min.m_data[2] = val.m_data[2];
    if (m_startTime == 0.0f) {
        m_startTime = time;
        m_maxTime = time + m_period;
    } else {
        if (time > m_maxTime)
            m_maxTime = time;
        if (time < m_startTime)
            m_startTime = time;
    }
}
void RecorderComponent::GetTimeRange(float *from, float *to) {
    *from = m_startTime;
    auto to_ = m_maxTime;
    if (to_ == 0.0f)
        to_ = m_period;
    *to = to_;
}
DataPoint nullpoint;
const DataPoint &RecorderComponent::GetDataPointAtTime(float time) {
    if (m_dataPoints.size() == 0)
        return nullpoint;
    if (m_dataPoints.size() == 1 || time < m_dataPoints.front().m_data[1])
        return m_dataPoints.front();
    if (time > m_dataPoints.back().m_data[1])
        return m_dataPoints.back();
    //QUEST: why not log2(N) ???
    auto v8 = m_dataPoints.data();
    while (time < v8->m_data[1] || time >= v8[1].m_data[1]) {
        if (++v8 >= &m_dataPoints.back())
            return nullpoint;
    }
    return *v8;
}
float RecorderComponent::GetBestTimeAverage(float dur) {
    uint32_t needCount = dur / m_timeBetweenPoints;
    if (needCount >= m_dataPoints.size())
        return 0.0f;
    auto sum = 0.0f;
    auto ptr = m_dataPoints.data();
    for (int i = 0; i < needCount; i++)
        sum += (*ptr++).m_data[0];
    ptr = m_dataPoints.data();
    auto maxSum = sum;
    for (int i = needCount - m_dataPoints.size(); i > 0; i--) {
        sum = sum - ptr->m_data[0] + (ptr + needCount)->m_data[0];
        ptr++;
        if (sum > maxSum)
            maxSum = sum;
    }
    return maxSum / (float)needCount;
}
float RecorderComponent::GetAverageFromPastNSeconds_Weighted(float secs) {
    auto sum = 0.0f;
    auto totalTime = 0.0f;
    for (auto it = m_dataPoints.crbegin(); it != m_dataPoints.crend(); it++) {
        auto t = it->m_data[1];
        auto weight = (secs - m_curTime + t) * 0.06666667f;
        if (m_curTime - t < 1.0f)
            weight *= (m_curTime - t);
        totalTime += weight;
        sum += weight * it->m_data[0];
        if (t <= m_curTime - secs)
            break;
    }
    if (totalTime <= 0.2f)
        return 0.0f;
    else
        return sum / totalTime;
}
float CriticalPowerCurve::GetBestTimeAverage(float period, float resultTimeSec) {
    auto v10 = 0i64;
    if (resultTimeSec >= 9.223372e18f) {
        resultTimeSec -= 9.223372e18f;
        if (resultTimeSec < 9.223372e18f)
            v10 = 0x8000000000000000ui64;
    }
    auto v19 = ZNETWORK_GetNetworkSyncedTimeGMT() - (v10 + (unsigned int)(int)resultTimeSec);
    auto v18 = ZNETWORK_GetNetworkSyncedTimeGMT();
    std::vector<DataPoint> dest;
    GetBestTimeAverage(&dest, true, &v19, &v18);
    for (auto &i : dest)
        if (i.m_data[1] - period < 0.01f)
            return i.m_data[0];
    return 0.0f;
}
void CriticalPowerCurve::Update(float) {
    //TODO
}
bool CriticalPowerCurve::InitializeDrawData(RecorderComponent::TIME_PERIOD) {
    //TODO
    return false;
}
void CriticalPowerCurve::GetBestTimeAverage(std::vector<DataPoint> *, bool, uint64_t *, uint64_t *) {
    //TODO
}
void CriticalPowerCurve::AddDataPoint(float val, float time) {
    if (!m_res0 && m_dataPoints.capacity() == m_dataPoints.size())
        m_dataPoints.reserve(m_dataPoints.size() * 3 / 2);
    m_dataPoints.push_back({ val, time });
}
void CriticalPowerCurve::PrepareForNextRide() {
    if (m_cpIdMax >= m_rideCPC.size())
        return;
    auto &cpc = m_rideCPC[m_cpIdMax];
    if (cpc.m_data.empty() || cpc.m_data.front().m_data[0] <= 0.0)
        return;
    if (m_rideCPC.size() <= m_cpIdMax + 1)
        m_rideCPC.emplace_back();
    auto v33 = m_rideCPC[m_cpIdMax + 1];
    auto v16 = m_rideCPC[m_cpIdMax].m_data;
    auto i = v16.begin();
    for (; i != v16.end(); ++i)
        if (i->m_data[1] == 0.0f)
            break;
    v33.m_data.assign(v16.begin(), i);
    v33.m_netTime = ZNETWORK_GetNetworkSyncedTimeGMT();
    ++m_cpIdMax;
}
void CriticalPowerCurve::Reset() {
    RecorderComponent::Reset();
    PrepareForNextRide();
    m_threadDoneTime = 0.0f;
    m_field_60 = 0;
}
void CriticalPowerCurve::UpdateServerBestPowerCurve() {
    if (m_exp->IsEnabled(FID_SERV_PC)) {
        /* TODO v2 = ZNet::API::Inst();
        ZNet::API::Dequeue(v2, this->field_170);
        m_net = this->m_net;
        memset(v6, 0, sizeof(v6));
        v4 = *m_net;
        v5[0] = &std::_Func_impl_no_alloc<_lambda_b5040236f1f66d4163eb2b5e34b0808b_, void, zwift::protobuf::powercurve::aggregation::PowerCurveAggregationMsg const &>::`vftable';
        v5[7] = v5;
        v5[1] = this;
        this->field_170 = *(*(v4 + 8))(m_net, &v7, v5, v6);*/
    }
}
void CriticalPowerCurve::HandleEvent(EVENT_ID ev, va_list) {
    if (ev == EV_RESET) {
        auto &prof = BikeManager::Instance()->m_mainBike->m_profile;
        m_playerId = prof.id();
        m_immunToCheatFlag = (prof.player_type() == protobuf::PRO_CYCLIST) || (prof.player_type() == protobuf::AMBASSADOR);
        m_isMale = prof.is_male();
        m_cpIdMax = m_cpIdMin = m_cpIdOffset = 0;
        m_field_64 = 1000;
        m_field_C4 = false;
        m_rideCPC.clear();
        UpdateServerBestPowerCurve();
    } else if (ev == EV_1f) {
        UpdateServerBestPowerCurve();
    }
}
float RecorderComponent::GetAverageFromPastNSeconds(float secs) {
    float sum = 0.0f;
    auto upToTime = m_curTime - secs;
    for (auto i = m_dataPoints.crbegin(); i != m_dataPoints.crend(); i++) {
        sum += i->m_data[0];
        if (upToTime >= i->m_data[1])
            break;
    }
    secs = roundf(secs);
    if (secs <= 0.0f)
        return 0.0f;
    else
        return sum / secs;
}
tinyxml2::XMLDocument *RecorderComponent::ExportXML() {
    auto ret = new tinyxml2::XMLDocument();
    auto fc = ret->NewElement("data");
    ret->InsertFirstChild(fc);
    float savedTime = 0.0f, sum = 0.0f, cnt = 0.0f;
    for (auto &i : m_dataPoints) {
        auto t = i.m_data[1];
        cnt += 1.0f;
        sum += i.m_data[0];
        if (t - savedTime > 10.0) {
            auto v10 = ret->NewElement("d");
            v10->SetAttribute("t", t);
            v10->SetAttribute("v", sum / cnt);
            fc->InsertEndChild(v10);
            savedTime = t;
        }
    }
    return ret;
}
void RecorderComponent::SampleSensor(float val, float time) { AddDataPoint(val, time); }
void RecorderComponent::PreConfigure(float mn, float mx, float period) {
    m_valMin = m_valMin_init = m_v3min.m_data[0] = m_v3min.m_data[1] = m_v3min.m_data[2] = mn;
    m_valMax = m_valMax_init = m_v3max.m_data[0] = m_v3max.m_data[1] = m_v3max.m_data[2] = mx;
    m_period = period;
    m_v3min_init = m_v3min;
    m_v3max_init = m_v3max;
}
void RecorderComponent::ImportXML(const tinyxml2::XMLDocument &src) {
    m_dataPoints.clear();
    auto ce = src.FirstChildElement()->FirstChildElement("d");
    while (ce) {
        VEC2 v{};
        auto va = ce->FindAttribute("v");
        if (va)
            va->QueryFloatValue(&v.m_data[0]);
        auto vt = ce->FindAttribute("t");
        if (vt)
            vt->QueryFloatValue(&v.m_data[1]);
        m_dataPoints.push_back(v);
        ce = ce->NextSiblingElement("d");
    }
}
CriticalPowerCurve::~CriticalPowerCurve() {
    m_eventSystem->Unsubscribe(EV_RESET, this);
    m_eventSystem->Unsubscribe(EV_1f, this);
    /* TODO v4 = ZNet::API::Inst();
    ZNet::API::Dequeue(v4, this->field_170);*/
}
void CriticalPowerCurve::StartThreadedCalculation(JobData *) {
    auto c = (CriticalPowerCurve *)DataRecorder::Instance()->GetComponent(RecorderComponent::T_CPC);
    if (c) {
        c->m_threadCalcInProgress = true;
        //TODO
    }
}
void CriticalPowerCurve::SearchCriticalPowerInCurrentRide() {
    //TODO inlined into StartThreadedCalculation
}
void CriticalPowerCurve::SearchCriticalPower(uint16_t, uint16_t, uint16_t, uint32_t) {
    //TODO
}
bool CriticalPowerCurve::Save() {
    if (!m_playerId || m_rideCPC.size() < 32)
        return false;
    PrepareForNextRide();
    char pathName[272], buf[272];
    const char *userPath = OS_GetUserPath();
    if (!userPath)
        return false;
    sprintf_s(pathName, "%s\\Zwift\\cp\\", userPath);
    CreateDirectoryA(pathName, nullptr);
    sprintf_s(pathName, "%suser%lld\\", pathName, m_playerId);
    CreateDirectoryA(pathName, nullptr);
    bool err = false;
    for (int cpId = m_cpIdMin; cpId < m_cpIdMax; ++cpId) {
        errno = 0;
        sprintf_s(buf, "%scp2_%d", pathName, cpId + m_cpIdOffset);
        auto f = fopen(buf, "wb");
        if (f) {
            fwrite("\x1\0\0\0", 4, 1, f);
            static_assert(sizeof(m_rideCPC[m_cpIdMax].m_netTime) == 8);
            if (errno || (fwrite(&m_rideCPC[m_cpIdMax].m_netTime, 8, 1, f), errno)) {
                err = true;
                break;
            } else {
                auto &v6 = m_rideCPC[cpId];
                for (auto i : v6.m_data) {
                    if (i.m_data[1] == 0.0f)
                        break;
                    uint16_t b[] = { uint16_t(i.m_data[1]), uint16_t(i.m_data[0]) };
                    fwrite(&b, sizeof(b), 1, f);
                    if (errno) {
                        err = true;
                        break;
                    }
                }
            }
            fclose(f);
            if (err)
                break;
        } else {
            err = true;
            break;
        }
    }
    if (err)
        LogTyped(LOG_ERROR, "CriticalPowerCurve::Save; File operation failure");
    return !err;
}
void CriticalPowerCurve::SandbaggerWarning(const float *, float, float) {
    //TODO inlined into GroupEvent_Update
}
void CriticalPowerCurve::LoggedIn(const protobuf::PlayerProfile &, int32_t) {
    //TODO - small, probably inlined
}
void CriticalPowerCurve::Load() {
    //TODO
}
uint16_t CriticalPowerCurve::GetServerMeanMaximalPowerAtInterval(uint16_t interval) {
    auto f = m_maxPowerAtInterval.find(interval);
    if (f == m_maxPowerAtInterval.end())
        return 0;
    return f->second;
}
void CriticalPowerCurve::GetServerMeanMaximalPower(std::vector<DataPoint> &, bool) {
    //TODO
}
const float gMaxPowerOutputTiers[] = { 5.0f, 60.0f, 300.0f, 3600.0f };
float CriticalPowerCurve::GetMaxPowerOutputTiers(uint32_t sel) {
    if (sel > _countof(gMaxPowerOutputTiers))
        return 0.0f;
    return gMaxPowerOutputTiers[sel];
}
void CriticalPowerCurve::DoneThreadedCalculation(int32_t, JobData *) {
    auto c = (CriticalPowerCurve *)DataRecorder::Instance()->GetComponent(RecorderComponent::T_CPC);
    if (c) {
        c->m_threadDoneTime = c->m_curTime;
        c->m_threadCalcInProgress = false;
    }
}
void CriticalPowerCurve::AntiSandbaggingCheck(AntiSandbaggingParams *ptr) {
    //TODO inlined into StartThreadedCalculation
}
void CriticalPowerCurve::AntiCheatingCheck() {
    //TODO inlined into StartThreadedCalculation
}
CriticalPowerCurve::CriticalPowerCurve(ZNet::NetworkService *net, Experimentation *exp, EventSystem *ev) : RecorderComponent(T_CPC), m_net(net), m_exp(exp), EventObject(ev) {
    /*TODO
    *&this->m_ev_part.field_20 = 0i64;
    *&this->m_ev_part.field_28 = 0i64;
    *&this->m_ev_part.field_30 = 0i64;
    *&this->m_ev_part.m_field_60 = 0i64;
    *&this->m_ev_part.field_68 = 0i64;
    *&this->m_ev_part.field_70 = 0;
    *&this->m_ev_part.field_74 = 1065353216i64;
    *&this->m_ev_part.field_C0 = 0i64;
    this->m_ev_part.m_field_C8 = -1;
    this->field_170 = 0i64;*/
    ev->Subscribe(EV_RESET, this);
    ev->Subscribe(EV_1f, this);
}
Grapher::Grapher(RecorderComponent *rc) {
    //TODO
}
void Grapher::Reset() {
    /*  TODO  *&v5->field_48 = *&v5->field_40;
    v5->field_18 = -1.0;
    v5->field_14 = -1.0;
    v5->field_1C = -1.7014118e38;
    v5->field_20 = 4i64;
    *&v5->field_28 = 0;
    *&v5->field_30 = 10;
    *&v5->field_2C = 10;
    *&v5->field_34 = 0i64;
    *&v5->field_58 = 0;
*/
}
