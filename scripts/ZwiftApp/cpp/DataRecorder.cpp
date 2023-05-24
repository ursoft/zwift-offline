#include "ZwiftApp.h"
DataRecorder::DataRecorder(Experimentation *exp, ZNet::NetworkService *net, EventSystem *ev) : EventObject(ev), m_exp(exp), m_net(net) {
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
void DataRecorder::ExpensiveCalculateTSS() {
    //TODO
}
void DataRecorder::ExpensiveCalculateTSS_EstimatedPower(const std::vector<float> &) {
    //TODO
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
void DataRecorder::Init() {
    //TODO
}
void DataRecorder::ResetSessionData() {
    //TODO
}
void DataRecorder::Update(float dt) {
    //TODO
}
DataRecorder::~DataRecorder() {
    //TODO
}
void DataRecorder::HandleEvent(EVENT_ID, va_list) {
    //TODO
}
void RecorderComponent::Init(float a2, uint32_t reserve) {
    m_init_a2 = a2;
    m_field_94 = 0.0f;
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
            if (m_field_94 <= 0.0f) {
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
                m_field_94 += m_init_a2;
            }
            auto v10 = float(m_type == T_1 ? bc->m_field_118 : bc->m_field_128) * 0.001f;
            m_field_94 -= (v10 - m_curTime);
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
void RecorderComponent::GetTimeRange(float *, float *) {
    //TODO
}
void RecorderComponent::GetDataPointAtTime(float)
{
}
void RecorderComponent::GetBestTimeAverage(float) {
    //TODO
}
void RecorderComponent::GetAverageFromPastNSeconds_Weighted(float)
{
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
    //TODO
}
void CriticalPowerCurve::Reset() {
    RecorderComponent::Reset();
    PrepareForNextRide();
    m_field_58 = 0;
    m_field_60 = 0;
}
void CriticalPowerCurve::UpdateServerBestPowerCurve() {
    //TODO
}
void CriticalPowerCurve::HandleEvent(EVENT_ID ev, va_list) {
    if (ev == EV_RESET) {
        m_field_50 = BikeManager::Instance()->m_mainBike->m_field_14F0;
        /* TODO     this->field_C6 = ((m_mainBike->m_playerType - 2) & 0xFFFFFFFD) == 0;
    this->field_C7 = m_mainBike->field_15B1[235];
    *(_QWORD *)&this->field_78 = 0i64;
    *(_DWORD *)&this->field_5C = 0;
*/
        m_field_64 = 1000;
        m_field_C4 = false;
        m_c_of_c.clear();
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
void RecorderComponent::ExportXML() {
    //TODO
}
void RecorderComponent::SampleSensor(float, float) {
    //TODO
}
void RecorderComponent::SampleCount() {
    //TODO
}
void RecorderComponent::PreConfigure(float, float, float) {
    //TODO
}
void RecorderComponent::ImportXML(tinyxml2::XMLDocument *) {
    //TODO
}
CriticalPowerCurve::~CriticalPowerCurve() {
    //TODO
}
void CriticalPowerCurve::StartThreadedCalculation(JobData *) {
    //TODO
}
void CriticalPowerCurve::SearchCriticalPowerInCurrentRide() {
    //TODO
}
void CriticalPowerCurve::SearchCriticalPower(uint16_t, uint16_t, uint16_t, uint32_t) {
    //TODO
}
void CriticalPowerCurve::Save() {
    //TODO
}
void CriticalPowerCurve::SandbaggerWarning(const float *, float, float) {
    //TODO
}
void CriticalPowerCurve::LoggedIn(const protobuf::PlayerProfile &, int32_t) {
    //TODO
}
void CriticalPowerCurve::Load() {
    //TODO
}
void CriticalPowerCurve::GetServerMeanMaximalPowerAtInterval(uint16_t) {
    //TODO
}
void CriticalPowerCurve::GetServerMeanMaximalPower(std::vector<DataPoint> &, bool) {
    //TODO
}
void CriticalPowerCurve::GetMaxPowerOutputTiers(int32_t) {
    //TODO
}
void CriticalPowerCurve::DoneThreadedCalculation(int32_t, JobData *) {
    //TODO
}
void CriticalPowerCurve::CreateFolder(const char *, uint32_t) {
    //TODO
}
void CriticalPowerCurve::AntiSandbaggingCheck(AntiSandbaggingParams *ptr) {
    //TODO
}
void CriticalPowerCurve::AntiCheatingCheck() {
    //TODO
}
CriticalPowerCurve::CriticalPowerCurve(ZNet::NetworkService *net, Experimentation *exp, EventSystem *ev) : RecorderComponent(T_CPC), m_net(net), m_exp(exp), EventObject(ev) {
    //TODO
}
Grapher::Grapher(RecorderComponent *rc) {
    //TODO
}
