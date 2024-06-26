#pragma once
enum FeatureID { FID_PPDATA=0x1D, FID_RLOG=0x1E, FID_FQUERY=0x1F, FID_VID_CAP=0x3D, FID_SERV_PC=0x65, FID_HIDE_HW=0x2E, FID_HIDE_HD=0x2F, FID_FIXLEAD=0x30, FID_REND_P=0x31, 
    FID_AVOID_A=0x32, FID_INTEG_B=0x33, FID_USE_RST=0x34, FID_IGNOREP=0x35, FID_CLEAR_P=0x37, FID_ENAB_BR=0x36, FID_WNS=0x64, FID_ENABLEJ=0x62, FID_MRB_V2 =0x67, FID_NINETYD=0, FID_DL_FIX=1,
    FID_PPV=2, FID_ASSERTD=3, FID_NOESISE=4, FID_SET_REF=5, FID_HOMESCR=6, FID_PERFANA=7, FID_FORYOUC=8, FID_SYSTEMA=9, FID_BLEALT=0xA, FID_TDFFEM=0xB, FID_BLE_NOF=0xC, FID_NEWWELC=0xD, FID_HOLIDAY=0xE,
    FID_PACKDY=0xF, FID_OSXMON=0x10, FID_DEF_RB=0x11, FID_USE_TS=0x12, FID_HWEXP1=0x13, FID_PAIR_DA=0x14, FID_RT_UNL=0x15, FID_BLE_DG2=0x16, FID_MIXP=0x17, FID_ADD_JB=0x18, FID_CLICK_T=0x19,
    FID_ENA_SS=0x1A, FID_HUD_HL=0x1B, FID_GPU_DEP=0x1C, FID_FTMS_RW=0x20, FID_BLE_DL2=0x21, FID_EN_LSUT=0x22, FID_FTMS_SF=0x23, FID_FTMS_V3=0x24, FID_LOG_BLE=0x25, FID_WAHOOD=0x26, FID_WAP=0x27,
    FID_WBE=0x28, FID_UNI_SL=0x29, FID_BLE_CSE=0x2A, FID_DWOW=0x2B, FID_DRWC=0x2C, FID_TR_PART=0x2D, FID_URR=0x38, FID_WDC_ERD=0x39, FID_EXT_PAS=0x3A, FID_FLAT_UI=0x3B,
    FID_AND_FEC=0x3C, FID_WDT=0x3E, FID_CLUBATT=0x3F, FID_FIX_TPS=0x40, FID_EVENTSU=0x41, FID_BLEMIDD=0x42, FID_MEM_SCR=0x43, FID_ATV_RG=0x44, FID_HW_RSTR=0x45, FID_GOAL_NM=0x46, FID_HUB_FWU=0x47,
    FID_FTMS_SD=0x48, FID_DYNA_CF=0x49, FID_BO_TRIG=0x4A, FID_KO_TRIG=0x4B, FID_GS_WKO=0x4C, FID_EV_CSF=0x4D, FID_FTMS_LA=0x4E, FID_WKO_TR2=0x4F, FID_TR_PRTS=0x50,
    FID_HANDCYC=0x51, FID_EVENTCA=0x52, FID_SET_TN=0x53, FID_EVENTQI=0x54, FID_BLE_CSP=0x55, FID_NEW_LTP=0x56, FID_FIX_SZP=0x57, FID_CL_REC=0x58, FID_GWP=0x59, FID_LEVEL60=0x5A,
    FID_PLAYERH=0x5B, FID_POS_G=0x5C, FID_POS_L=0x5D, FID_LO_WM=0x5E, FID_PD_V4=0x5F, FID_CMP_FSM=0x60, FID_MOB_SCA=0x61, FID_MOB_SMA=0x63, FID_PD_4GLB=0x66, FID_CNT=104 };
struct FeatureMetadata { //40 bytes
    FeatureID m_id;
    const char *m_name;
    int32_t m_deprecated;
    int32_t m_unk[4];
    //int m_notused; always 0
};
extern const FeatureMetadata g_featureMetadata[];
struct Feature {
    FeatureID m_id;
    FeatureID ToIndex() { return m_id; }
    const char *c_str();
    static const char *c_str(FeatureID id);
};
template<class T> struct FeatureValue {
    T m_val;
    bool m_filled = false;
};
struct ZwiftDispatcher { //size=8
    char data[8];
public:
    ZwiftDispatcher();
    void Assert(bool bPredicate);
    void Assert(bool bPredicate, const char *errMsg);
};
struct FeatureVariable { //9 int64_t Experiment::Impl::FeatureVariable<zu::ZwiftDispatcher>
    std::string m_name;
    void *m_type;
    union u {
        int64_t m_i64;
        bool m_bool;
        double m_dbl;
    } m_uval;
    std::string m_str;
    operator bool() { return m_uval.m_bool; }
    operator int64_t() { return m_uval.m_i64; }
    operator double() { return m_uval.m_dbl; }
    operator std::string() { return m_str; }
};
struct FeaRequestResult {
    bool m_succ;
    int64_t m_unk;
};
enum ExpVariant { EXP_UNASSIGNED, EXP_ENABLED, EXP_DISABLED, EXP_NONE, EXP_UNKNOWN, EXP_CNT }; //Experiment::Variant
inline const char *g_expVarNames[EXP_CNT] = { "unassigned", "enabled", "disabled", "none", "unknown" };
using FeatureCallback = std::function<void(ExpVariant)>;
struct RegisteredCallback { //size=8*9=72?
    int64_t m_cntId;
    FeatureCallback m_func;
    RegisteredCallback(int64_t cnt, const FeatureCallback &func) : m_cntId(cnt), m_func(func) {}
};
struct FeatureStateMachine { //size=72 Experiment::Impl::FeatureStateMachine<zu::ZwiftDispatcher>
    ExpVariant m_enableStatus;
    std::vector<FeatureVariable> m_variables;
    int m_field20;
    int64_t m_rqCounter;
    std::vector<RegisteredCallback> m_callbacks;
    FeaRequestResult OnRequest(const FeatureCallback &func);
    void OnResponse(ExpVariant res, const std::vector<FeatureVariable> &resExt);
    template<class T> FeatureValue<T> GetFeatureVariable(const std::string &name) {
        for (auto &i : m_variables) {
            if (i.m_name == name) {
                //assert(); //TODO: ������������ ���� ���������� i.m_type � �
                return FeatureValue<T>{ T(i), true };
            }
        }
        return FeatureValue<T>{ T(), false };
    }
};
struct ExpIsEnabledResult {
    FeatureID m_id;
    int64_t m_unk; //ExpVariant?
};
struct UserAttributes {
    void *m_somePointer;
    FeatureValue<std::string> m_eventTypeAttr;
    std::string m_str[5];
};
struct ZNetAdapter {
    protobuf::FeatureRequest *FormFeatureRequest(UserAttributes *ua);
};
inline ZNetAdapter g_znetAdapter;
struct Experimentation : public EventObject { //sizeof=0x1E48; vtblExperimentation+0=DTR
    FeatureStateMachine m_fsms[FID_CNT];
    UserAttributes m_userAttributes;
    ZNetAdapter *m_pNA;
    std::vector<FeatureID> m_ids;
    static inline std::unique_ptr<Experimentation> g_ExperimentationUPtr;
    Experimentation(ZNetAdapter *, EventSystem *ev);
    void HandleEvent(EVENT_ID e, va_list args) override; //vtblEvent
    virtual ~Experimentation() { /* TODO */ }                                        //vtblExp+0
    ExpIsEnabledResult IsEnabled(FeatureID id, const FeatureCallback &func);         //vtblExp+1
    ExpVariant IsEnabled(FeatureID id, ExpVariant overrideIfUn);                     //vtblExp+2
    bool IsEnabled(FeatureID id);                                                    //vtblExp+3
    void HandleLogout();                                                             //vtblExp+4
    //TODO void Unregister(Experiment::CallbackID<Experiment::Feature>)              //vtblExp+5
    template <class T> FeatureValue<T> Get(FeatureID id, const std::string &name) {  //vtblExp+6: bool, vtblExp+7: int64, vtblExp+8: double, vtblExp+9: String
        return m_fsms[id].GetFeatureVariable<T>(name);
    }
    template <FeatureID F>
    static bool IsEnabledCached() {
        static bool g = Experimentation::Instance()->IsEnabled(F);
        return g;
    }
    void SetEventTypeAttribute(const std::string_view &src);                         //vtblExp+10
    void RequestFeatureData(FeatureID id);
    static bool IsInitialized() { return g_ExperimentationUPtr.get() != nullptr; }
    static void Shutdown() { g_ExperimentationUPtr.reset(); }
    static Experimentation *Instance() { zassert(g_ExperimentationUPtr.get() != nullptr); return g_ExperimentationUPtr.get(); }
    static void Initialize(EventSystem *ev);
    void BulkRequestFeatureData(const std::vector<FeatureID> &ids);
};
struct ZFeature { /*TODO*/ };
inline ZFeature gPlayerHighlight, gHoloEffect;
struct ZFeatureManager { //0x40 bytes
    inline static std::unique_ptr<ZFeatureManager> g_FeatureManager;
    ZFeatureManager() { /*TODO*/ }
    static void Initialize() { g_FeatureManager.reset(new ZFeatureManager()); }
    static bool IsInitialized() { return g_FeatureManager.get() != nullptr; }
    static ZFeatureManager *Instance() { zassert(g_FeatureManager.get() != nullptr); return g_FeatureManager.get(); }
    static void Shutdown() { g_FeatureManager.reset(); }
    void AddFeature(ZFeature *) { /*TODO*/ }
    void Find(const ZFeature &, std::vector<ZFeature *> &) { /*TODO*/ }
    void LoadConfig() { /*TODO*/ }
    void ReloadConfigIfChanged() { /*TODO*/ }
    void Update(float) { /*TODO*/ }
    void Init() { /*TODO*/ }
    ~ZFeatureManager() { /*TODO*/ }
};
void DetermineNoesisFeatureFlags();