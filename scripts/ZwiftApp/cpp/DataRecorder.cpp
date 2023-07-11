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
    m_graphers[0] = new Grapher(GetComponent(RecorderComponent::T_RIDE_SPEED));
    m_graphers[1] = new Grapher(GetComponent(RecorderComponent::T_RUN_SPEED));
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
                    v14 = dp[i].m_val;
                    v9 += v14;
                } else {
                    v9 += dp[i].m_val - dp[i - 30].m_val;
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
    auto num = (int)src.size();
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
bool check_float(float f) {
    int fpclass = _fpclass(f);
    return (fpclass == _FPCLASS_PN) || (fpclass & _FPCLASS_NN) || (fpclass & _FPCLASS_NZ) || (fpclass & _FPCLASS_PZ);
}
void RecorderComponent::Update(float) {
    auto mb = BikeManager::Instance()->m_mainBike;
    auto bc = mb->m_bc;
    if (bc->m_bool) {
        if (m_type == T_1 || !bc->m_bool1) {
            float val = 0.0f;
            if (m_nextPointTime <= 0.0f) {
                switch (m_type) {
                case T_0: case T_1:
                    if (bc->m_power < 2300.0f)
                        val = bc->m_power;
                    AddDataPoint(val, m_curTime);
                    break;
                case T_RIDE_SPEED:
                    if (check_float(bc->m_speed))
                        val = bc->m_speed;
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
                case T_RUN_SPEED:
                    if (bc->m_sport == protobuf::RUNNING) {
                        if (check_float(bc->m_speed))
                            val = bc->m_speed;
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
    if (m_dataPoints.size() == 1 || time < m_dataPoints.front().m_time)
        return m_dataPoints.front();
    if (time > m_dataPoints.back().m_time)
        return m_dataPoints.back();
    //QUEST: why not log2(N) ???
    auto v8 = m_dataPoints.data();
    while (time < v8->m_time || time >= v8[1].m_time) {
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
    for (uint32_t i = 0; i < needCount; i++)
        sum += (*ptr++).m_val;
    ptr = m_dataPoints.data();
    auto maxSum = sum;
    for (uint32_t i = needCount - (uint32_t)m_dataPoints.size(); i > 0; i--) {
        sum = sum - ptr->m_val + (ptr + needCount)->m_val;
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
        auto t = it->m_time;
        auto weight = (secs - m_curTime + t) * 0.06666667f;
        if (m_curTime - t < 1.0f)
            weight *= (m_curTime - t);
        totalTime += weight;
        sum += weight * it->m_val;
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
        if (i.m_time - period < 0.01f)
            return i.m_val;
    return 0.0f;
}
struct JobDataCPC : JobData {
    bool m_hasPowerRqs;
    float m_powerRqs[4];
    float m_eventCPCCheatTimeScale;
} g_cpcJobData;
void CriticalPowerCurve::Update(float dt) {
    if (m_playerId && ZNETWORK_IsLoggedIn()) {
        auto bk = BikeManager::Instance()->m_mainBike;
        if (bk && bk->m_bc->m_sport == protobuf::CYCLING) {
            if (!m_loaded && ZNETWORK_GetNetworkSyncedTimeGMT() > 0)
                Load();
            if (m_dataPoints.size() < 43200.0f / m_timeBetweenPoints) {
                if (m_nextPointTime <= 0.0f) {
                    auto power = bk->m_bc->m_power;
                    if (power >= 2300.0f)
                        power = 0.0f;
                    AddDataPoint(power, m_curTime);
                    m_nextPointTime += m_timeBetweenPoints;
                }
                if (m_threadCalcInProgress || m_curTime - m_lastSearchTime < 10.0f) {
                    g_cpcJobData = {};
                    g_cpcJobData.m_size = sizeof(g_cpcJobData);
                    auto EventID = bk->GetEventID();
                    if (EventID) {
                        auto v9 = GroupEvents::FindSubgroupEvent(EventID);
                        if (v9 && v9->m_field_198 == 8 /* TODO  && *(sub_7FF778AA4B60(v9) + 296) != 0.0*/ &&
                            v9->GetRules()->m_mask & GroupEvents::Rules::GERM_PWR_CHECK) {
                            g_cpcJobData.m_hasPowerRqs = true;
                            g_cpcJobData.m_eventCPCCheatTimeScale = 1.234f; /*TODO *(sub_7FF778AA4B60(v9) + 296);
                            v13 = 0i64;
                            do {
                                *(&g_cpcJobData.m_powerRqs + v13) = v9->GetRules()->m_mask + v13 + 287);
                                v13 += 4i64;
                            } while (v13 < 16);*/
                        }
                    }
                    Log("Starting critical power curve search job (%d)  mTimeStamp = %3.5f  m_lastSearchTime = %3.5f   calculating=%s",
                        timeGetTime(), m_curTime, m_lastSearchTime,
                        m_threadCalcInProgress ? "true" : "false");
                    m_lastSearchTime = m_curTime;
                    JM_AddJob(CriticalPowerCurve::StartThreadedCalculation, &g_cpcJobData, JT_CPC, CriticalPowerCurve::DoneThreadedCalculation, JP_NORMAL);
                }
                m_curTime += dt;
                m_nextPointTime -= dt;
            }
        }
    }
}
bool CriticalPowerCurve::InitializeDrawData(RecorderComponent::TIME_PERIOD tp) {
    m_timePeriod = tp;
    auto to = ZNETWORK_GetNetworkSyncedTimeGMT();
    auto from = to;
    bool v13 = false;
    m_cpDataPoints.clear();
    switch (tp) {
    case TP_5S:
        break;
    case TP_MINUTE:
        from = to - 1209600;
        break;
    case TP_5MIN:
        from = to - 2592000;
        break;
    case TP_HOUR:
        from = 0i64;
        {
            auto e = m_exp->Get<bool>(FID_SERV_PC, "view_enabled"s);
            v13 = e.m_filled && e.m_val;
        }
        break;
    default:
        LogTyped(LOG_ERROR, "CriticalPowerCurve unsupported time period");
        break;
    }
    if (v13)
        GetServerMeanMaximalPower(&m_cpDataPoints, true);
    else
        GetBestTimeAverage(&m_cpDataPoints, true, &from, &to);
    if (m_cpDataPoints.size() < 8)
        return false;
    m_valMin = 10000.0f;
    m_valMax = 0.0f;
    for (auto i : m_cpDataPoints) {
        m_valMin = fminf(i.m_val, m_valMin);
        m_valMax = fmaxf(i.m_val, m_valMax);
    }
    auto imax = int(m_valMax), imin = int(m_valMin);
    m_valMax = imax + (100 - imax % 100);
    m_valMin = imin - (imin % 100);
    m_startTime = 0.0f;
    m_maxTime = m_cpDataPoints.back().m_time;
    return true;
}
void CriticalPowerCurve::GetBestTimeAverage(std::vector<DataPoint> *vec, bool addNotFinished, uint64_t *ioFrom, uint64_t *ioTo) {
    if (vec && m_rideCPC.size() >= 32) {
        if (m_playerId) {
            if (!*ioTo)
                *ioTo = ZNETWORK_GetNetworkSyncedTimeGMT();
            if (*ioFrom > *ioTo) {
                LogTyped(LOG_ERROR, "CriticalPowerCurve::Save Query; inOutTimeStampFrom is larger than inOutTimeStampTo");
                std::swap(*ioFrom, *ioTo);
            }
            int left = 0, fromIdx = 0, toIdx = 0, idmax = addNotFinished ? m_cpIdMax : (m_cpIdMax - 1);
            if (idmax >= 0) {
                auto right = idmax;
                while (1) {
                    fromIdx = (right + left) >> 1;
                    if (m_rideCPC[fromIdx].m_netTime == *ioFrom) {
                        if (fromIdx > 0) {
                            while (m_rideCPC[fromIdx - 1].m_netTime == *ioFrom) {
                                fromIdx--;
                                if (fromIdx == 0) break;
                            }
                        }
                        break;
                    } else if (m_rideCPC[fromIdx].m_netTime > *ioFrom) {
                        right = fromIdx - 1;
                    } else {
                        left = fromIdx + 1;
                    }
                    if (left > right) {
                        left = 0;
                        right = idmax;
                        while (1) {
                            toIdx = (right + left) >> 1;
                            if (m_rideCPC[toIdx].m_netTime == *ioTo) {
                                if (toIdx < m_rideCPC.size() - 1) {
                                    while (m_rideCPC[toIdx + 1].m_netTime == *ioTo) {
                                        toIdx++;
                                        if (toIdx == m_rideCPC.size()) break;
                                    }
                                }
                                break;
                            } else if (m_rideCPC[toIdx].m_netTime >= *ioTo) {
                                right = toIdx - 1;
                            } else {
                                left = toIdx + 1;
                            }
                            if (left > right)
                                break;
                        }
                    }
                }
            }
            int v30 = (int)m_rideCPC.size();
            if (fromIdx >= v30) {
                LogTyped(LOG_ERROR, "CriticalPowerCurve::QueryHistoricalBestTime start value is invalid");
                fromIdx = 0;
            }
            if (toIdx < 0 || toIdx >= v30) {
                LogTyped(LOG_ERROR, "CriticalPowerCurve::QueryHistoricalBestTime end value is invalid");
                toIdx = v30 - 1;
            }
            *ioFrom = m_rideCPC[fromIdx].m_netTime;
            *ioTo = m_rideCPC[toIdx].m_netTime;
            while (fromIdx <= toIdx) {
                DataPoint p{};
                auto &v53 = m_rideCPC[fromIdx].m_data;
                for (int i = 0; i < toIdx - fromIdx + 1; i++) {
                    if (i < v53.size()) {
                        if (v53[i].m_time != 0.0f) {
                            p.m_time = v53[i].m_time;
                            if (p.m_val < v53[i].m_val)
                                p.m_val = v53[i].m_val;
                        }
                    }
                    if (!p.m_time)
                        break;
                }
                if (!p.m_time)
                    vec->push_back(p);
                fromIdx++;
            }
        }
    } else {
        LogTyped(LOG_ERROR, "CriticalPowerCurve::Save Query; pOutDataPoints is NULL");
    }
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
    if (cpc.m_data.empty() || cpc.m_data.front().m_val <= 0.0f)
        return;
    if (m_rideCPC.size() <= m_cpIdMax + 1)
        m_rideCPC.emplace_back();
    auto v33 = m_rideCPC[m_cpIdMax + 1];
    auto v16 = m_rideCPC[m_cpIdMax].m_data;
    auto i = v16.begin();
    for (; i != v16.end(); ++i)
        if (i->m_time == 0.0f)
            break;
    v33.m_data.assign(v16.begin(), i);
    v33.m_netTime = ZNETWORK_GetNetworkSyncedTimeGMT();
    ++m_cpIdMax;
}
void CriticalPowerCurve::Reset() {
    RecorderComponent::Reset();
    PrepareForNextRide();
    m_lastSearchTime = 0.0f;
    m_field_60 = 0;
}
void CriticalPowerCurve::UpdateServerBestPowerCurve() {
    if (m_exp->IsEnabled(FID_SERV_PC)) {
        ZNet::API::Inst()->Dequeue(m_requestId);
        ZNet::Params p{};
        m_requestId = m_net->GetAllTimeBestEffortsPowerCurve([this](const protobuf::PowerCurveAggregationMsg &pcam) {
            this->m_maxPowerAtInterval.clear(); //this[35...38]
            this->m_maxWattsKey = 0; // this[40][0]
            for (auto &i : pcam.watts()) {
                auto key = _strtoi64(i.first.c_str(), nullptr, 10);
                m_maxPowerAtInterval[key] = i.second.power(); //v14
                if (key > this->m_maxWattsKey)
                    this->m_maxWattsKey = key;
            }
        }, &p);
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
        m_loaded = false;
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
        sum += i->m_val;
        if (upToTime >= i->m_time)
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
        auto t = i.m_time;
        cnt += 1.0f;
        sum += i.m_val;
        if (t - savedTime > 10.0f) {
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
        DataPoint v;
        auto va = ce->FindAttribute("v");
        if (va)
            va->QueryFloatValue(&v.m_val);
        auto vt = ce->FindAttribute("t");
        if (vt)
            vt->QueryFloatValue(&v.m_time);
        m_dataPoints.push_back(v);
        ce = ce->NextSiblingElement("d");
    }
}
CriticalPowerCurve::~CriticalPowerCurve() {
    m_eventSystem->Unsubscribe(EV_RESET, this);
    m_eventSystem->Unsubscribe(EV_1f, this);
    ZNet::API::Inst()->Dequeue(m_requestId);
}
constexpr struct CriticalPowerCurveData {
    uint16_t m_from, m_to, m_step;
} g_CriticalPowerCurveData[] = {
  { 0, 60, 1 },
  { 60, 300, 5 },
  { 300, 1200, 10 },
  { 1200, 3600, 30 },
  { 3600, 43200, 60 } };
constexpr float g_MaxPowerOutputTiers[] = { 5.0f, 60.0f, 300.0f, 3600.0f };
int CriticalPowerCurve::StartThreadedCalculation(JobData *jd_) {
    assert(sizeof(JobDataCPC) == jd_->m_size);
    auto *jd = (JobDataCPC *)jd_;
    auto c = (CriticalPowerCurve *)DataRecorder::Instance()->GetComponent(RecorderComponent::T_CPC);
    if (c) {
        c->m_threadCalcInProgress = true;
        if (c->m_rideCPC.size() >= 32) { // CriticalPowerCurve::SearchCriticalPowerInCurrentRide
            uint32_t index = 0;
            for (auto &v6 : g_CriticalPowerCurveData) {
                auto toMax = c->m_dataPoints.size();
                if (toMax >= v6.m_from) {
                    auto to = v6.m_to;
                    if (to > toMax)
                        to = uint16_t(c->m_dataPoints.size());
                    c->SearchCriticalPower(v6.m_from, to, v6.m_step, index);
                    index += (v6.m_to - v6.m_from) / v6.m_step;
                }
            }
            c->m_field_60 = uint32_t(c->m_dataPoints.size());
        }
        auto bk = BikeManager::Instance()->m_mainBike;// CriticalPowerCurve::AntiCheatingCheck
        if (bk && !bk->m_immuneFromCheating && c->m_normMPO.size() > 3 && !g_pDialogs[UID_CUSTOMIZE_BIKE_AND_RIDER]
            && !g_pDialogs[UID_DEVICE_PAIRING] && !g_pDialogs[UID_LOG_IN] && !g_pDialogs[UID_EULA_VIEW] && !g_pDialogs[UID_DROP_IN]
            && !g_pDialogs[UID_SPORT_SELECT_CONTROLLER] && !g_pDialogs[UID_ONBOARDINGCUSTOMIZE]) {
            auto v14 = 1.0f;
            /* TODO if (!*(bk->m_ptr + 284) && FitnessDeviceManager::GetCurrentSpeed() != 0.0)
                v14 = 0.8f;*/
            const float powPerCat[][2] = { { 118.09f, 123.36f }, { 8.73f, 10.79f }, { 6.15f, 6.96f }, { 5.18f, 6.02f } };
            for (int cat = 0; cat < int(c->m_normMPO.size()); ++cat) {
                if (cat <= 3 && g_MaxPowerOutputTiers[cat] > 59.0f) {
                    if (v14 * powPerCat[cat][c->m_isMale ? 1 : 0] * 1.1f <= g_CPCCheatTimeScale * c->m_normMPO[cat]) {
                        if (0 == (c->m_flierBits & (1 << cat))) {
                            c->m_flierBits |= (1 << cat);
                            ZNETWORK_BroadcastLocalPlayerFlagged(PFR_FLIER);
                            if (!bk->m_boolCheatSmth)
                                bk->m_cheatBits |= 2;
                            Log("Flagged by high CPC");
                        }
                    }
                }
            }
            if (c->m_flierBits) {
                EventSystem::GetInst()->TriggerEvent(EV_FLIER, 1, c->m_flierBits);
                for (int cat = 0; cat < _countof(g_MaxPowerOutputTiers); cat++) {
                    if (c->m_flierBits & (1 << cat)) {
                        LogTyped(LOG_WARNING, "Irregular Power Output at: %d-sec MPO interval\nNormalized Maximal Power Output: %7.2f W/Kg\n",
                            (int)g_MaxPowerOutputTiers[cat], c->m_normMPO[cat]);
                    }
                }
            }
        }
        if (bk && jd && !bk->m_immuneFromCheating && c->m_normMPO.size() && jd->m_hasPowerRqs) { // CriticalPowerCurve::AntiSandbaggingCheck
            for (int cat = 0; cat < int(c->m_normMPO.size()); ++cat) {
                if (cat <= 3 && g_MaxPowerOutputTiers[cat] > 59.0f) {
                    auto minCPC = jd->m_powerRqs[cat];
                    if (minCPC > 0.0f) {
                        auto jsc = jd->m_eventCPCCheatTimeScale;
                        if (jsc > 0.0f && minCPC * jsc <= g_CPCCheatTimeScale * c->m_normMPO[cat]) {
                            if (0 == (c->m_sandBits & (1 << cat))) {
                                c->m_sandBits |= (1 << cat);
                                ZNETWORK_BroadcastLocalPlayerFlagged(PFR_SANDBAGGER);
                                c->m_sandbaggerCoeff = minCPC * jsc / (g_CPCCheatTimeScale * c->m_normMPO[cat]);
                                Log("Sandbagger based on CPC");
                            }
                        }
                    }
                }
            }
            if (c->m_sandBits) {
                EventSystem::GetInst()->TriggerEvent(EV_SANDBAGGER, 1, c->m_sandBits);
                for (int cat = 0; cat < _countof(g_MaxPowerOutputTiers); cat++) {
                    if (c->m_sandBits & (1 << cat))
                        LogTyped(LOG_WARNING, "Out Of Category Power Output at: %d-sec MPO interval\n"
                            "Normalized Maximal Power Output: %7.2f W/Kg\n", (int)g_MaxPowerOutputTiers[cat],
                            c->m_normMPO[cat]);
                }
            }
        }
    }
    return 0;
}
void CriticalPowerCurve::SearchCriticalPower(uint16_t from, uint16_t to, uint16_t step, uint32_t index) {
    if (to < from || m_field_60 == m_dataPoints.size()) {
        LogTyped(LOG_ERROR, "CriticalPowerCurve::SearchCriticalPowerAtTimeStep; to is less than from");
    } else {
        auto kg = BikeManager::Instance()->m_mainBike ? BikeManager::Instance()->m_mainBike->m_profile.weight_in_grams() * 0.001f : 0.0f;
        if (m_cpIdMax >= m_rideCPC.size() || index >= m_rideCPC[m_cpIdMax].m_data.size())
            LogTyped(LOG_ERROR, "CriticalPowerCurve::SearchCriticalPower; invalid index into critical power records");
        else 
            do {
                if (from) {
                    auto v16 = m_dataPoints.size() - 1;
                    auto &valref = m_rideCPC[m_cpIdMax].m_data[index++];
                    float newVal = valref.m_val;
                    if (v16 >= m_field_60) {
                        auto v22 = v16 - from;
                        if (v22 >= 0) do {
                            auto sum = 0.0f;
                            auto v25 = v16;
                            while (v25 > v22)
                                sum += m_dataPoints[v25--].m_val;
                            --v16;
                            --v22;
                            newVal = fmaxf(sum / from + 0.5f, newVal);
                        } while (v16 >= m_field_60);
                    }
                    if (kg > 0.0f && newVal > valref.m_val) {
                        for (auto v30 = 0; v30 < _countof(g_MaxPowerOutputTiers); v30++) {
                            if (g_MaxPowerOutputTiers[v30] == float(from)) {
                                m_normMPO[v30] = fmaxf(newVal / kg, m_normMPO[v30]);
                                break;
                            }
                        }
                    }
                    valref = { newVal, float(from) };
                }
                from += step;
            } while (from <= to);
    }
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
    for (auto cpId = m_cpIdMin; cpId < m_cpIdMax; ++cpId) {
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
                    if (i.m_time == 0.0f)
                        break;
                    uint16_t b[] = { uint16_t(i.m_time), uint16_t(i.m_val) };
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
    if (!m_playerId)
        return;
    m_field_68 = 0;
    for(auto v2 : g_CriticalPowerCurveData)
        m_field_68 += (v2.m_to - v2.m_from) / v2.m_step;
    char pathName[272], buf[272];
    const char *userPath = OS_GetUserPath();
    if (!userPath)
        return;
    sprintf_s(pathName, "%s\\Zwift\\cp\\", userPath);
    CreateDirectoryA(pathName, nullptr);
    sprintf_s(pathName, "%suser%lld\\", pathName, m_playerId);
    CreateDirectoryA(pathName, nullptr);
    int v9 = 0;
    do {
        sprintf_s(buf, sizeof(buf), "%scp2_%d", pathName, v9++);
    } while (std::filesystem::exists(buf));
    errno = 0;
    auto v44 = std::max(0, v9 - m_field_64);
    m_rideCPC.resize(v9 - v44 + 1);
    for (; v44 < v9; v44++) {
        sprintf_s(buf, sizeof(buf), "%scp2_%d", pathName, v44);
        auto v22 = fopen(buf, "rb");
        if (v22) {
            /*QUEST:what for ??? fseek(v22, 0, SEEK_END);
            ftell(v22);
            rewind(v22);*/
            int v46 = 0;
            fread(&v46, sizeof(v46), 1, v22);
            int v24 = 0;
            if (!errno) {
                const int expVer = 1;
                if (v46 == expVer) {
                    static_assert(sizeof(m_rideCPC[m_cpIdMax].m_netTime) == 8);
                    fread(&m_rideCPC[m_cpIdMax].m_netTime, 8, 1, v22);
                    if (!errno) {
                        m_rideCPC[m_cpIdMax].m_data.clear();
                        uint16_t val0, val1;
                        if (fread(&val0, sizeof(val0), 1, v22)) {
                            while (!errno) {
                                if (fread(&val1, sizeof(val1), 1, v22) && !errno) {
                                    m_rideCPC[m_cpIdMax].m_data.push_back({float(val1), float(val0)});
                                    ++v24;
                                    if (!fread(&val0, sizeof(val0), 1, v22))
                                        goto succ_load;
                                } else {
                                    errno = 0;
                                    LogTyped(LOG_ERROR, "CriticalPowerCurve cannot load entry %d power value", v24);
                                }
                            }
                            if (errno) {
                                LogTyped(LOG_ERROR, "CriticalPowerCurve cannot load entry %d time step", v24);
                            }
                        } else {
succ_load:
                            ++m_cpIdMax;
                            fclose(v22);
                            continue;
                        }
                    } else {
                        LogTyped(LOG_ERROR, "CriticalPowerCurve cannot load time stamp");
                    }
                } else {
                    LogTyped(LOG_ERROR, "CriticalPowerCurve is version %d, expecting %d", v46, expVer);
                }
            } else {
                LogTyped(LOG_ERROR, "CriticalPowerCurve cannot load version number");
            }
            ++m_cpIdOffset;
            m_rideCPC.pop_back();
            fclose(v22);
        } else {
            ++m_cpIdOffset;
            m_rideCPC.pop_back();
            continue;
        }
    }
    auto pMax = &m_rideCPC[m_cpIdMax];
    pMax->m_netTime = ZNETWORK_GetNetworkSyncedTimeGMT();
    pMax->m_data.resize(m_field_68);
    m_normMPO.resize(4);
    m_cpIdMin = m_cpIdMax;
    m_loaded = true;
}
uint16_t CriticalPowerCurve::GetServerMeanMaximalPowerAtInterval(uint16_t interval) {
    auto f = m_maxPowerAtInterval.find(interval);
    if (f == m_maxPowerAtInterval.end())
        return 0;
    return f->second;
}
void CriticalPowerCurve::GetServerMeanMaximalPower(std::vector<DataPoint> *vec, bool a3 /*use unfinished activity? */) {
    if (!m_playerId)
        return;
    std::unordered_map<uint16_t, uint16_t> v4(m_maxPowerAtInterval);
    if(a3 && m_rideCPC.size()) {
        auto lastDataCopy = m_rideCPC.back().m_data;
        for (auto i : lastDataCopy) {
            auto f = v4.find(uint16_t(i.m_time));
            if (f != v4.end()) {
                if (f->second < i.m_val)
                    f->second = i.m_val;
            } else {
                if (i.m_time > m_maxWattsKey)
                    v4[i.m_time] = i.m_val;
            }
        }
    }
    for (auto &i : v4)
        vec->push_back({ float(i.second), float(i.first) });
}
float CriticalPowerCurve::GetMaxPowerOutputTiers(uint32_t sel) {
    if (sel > _countof(g_MaxPowerOutputTiers))
        return 0.0f;
    return g_MaxPowerOutputTiers[sel];
}
void CriticalPowerCurve::DoneThreadedCalculation(int32_t, JobData *) {
    auto c = (CriticalPowerCurve *)DataRecorder::Instance()->GetComponent(RecorderComponent::T_CPC);
    if (c) {
        c->m_lastSearchTime = c->m_curTime;
        c->m_threadCalcInProgress = false;
    }
}
CriticalPowerCurve::CriticalPowerCurve(ZNet::NetworkService *net, Experimentation *exp, EventSystem *ev) : RecorderComponent(T_CPC), m_net(net), m_exp(exp), EventObject(ev) {
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
