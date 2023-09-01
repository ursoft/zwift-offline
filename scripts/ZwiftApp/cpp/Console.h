#pragma once //READY for testing
struct CMD_AutoCompleteParamSearchResults {
    std::string m_descr;
    std::vector<std::string> m_field_20;
    bool        m_field_38 = false;
};
typedef bool (*CMD_bool)(const char *);
typedef char * (*CMD_static_str)(const char *);
typedef void (*CMD_ac_search)(CMD_AutoCompleteParamSearchResults *, const char *);
typedef std::string (*CMD_string)(const char *);
enum TweakDataType { TWD_BOOL = 1, TWD_INT, TWD_UINT, TWD_FLOAT, TWD_STRING };
const inline float FMIN = -1.0e17f, FMAX = 1.0e16f;
const inline int IMIN = -2147483647, IMAX = 2147483647;
struct TweakInfo {
    uint32_t      m_valueUnion = 0;
    float         m_floatMin = FMIN, m_floatMax = FMAX;
    uint32_t      m_uintMin = 0, m_uintMax = 0xFFFFFFFF;
    int           m_intMin = IMIN, m_intMax = IMAX, field_1C = 0;
    const char    *field_20 = nullptr, *m_name = nullptr, *m_file = nullptr;
    int           m_lineNo = 0;
    TweakDataType m_dataType = TWD_BOOL;
    std::string   m_str;
    float &FloatValue() { return *(float *)&m_valueUnion; }
    int &IntValue() { return *(int *)&m_valueUnion; }
    bool SetValue(const std::string &val) {
        std::string lowVal;
        int         ival;
        uint32_t    uval;
        float       fval;
        switch (m_dataType) {
        case TWD_BOOL:
            lowVal.reserve(val.size());
            for (auto c : val)
                lowVal.push_back(std::tolower(c));
            if (lowVal == "true" || lowVal == "t" || lowVal == "1" || lowVal == "y") {
                m_valueUnion = 1;
                return true;
            }
            if (lowVal != "false" && lowVal != "f" && lowVal != "0" && lowVal != "n")
                return false;
            m_valueUnion = 0;
            return true;
        case TWD_INT:
            if (sscanf(val.c_str(), "%d", &ival) != 1)
                return false;
            IntValue() = std::clamp(ival, m_intMin, m_intMax);
            return true;
        case TWD_UINT:
            if (sscanf(val.c_str(), "%u", &uval) != 1)
                return false;
            m_valueUnion = std::clamp(uval, m_uintMin, m_uintMax);
            return true;
        case TWD_FLOAT:
            if (sscanf(val.c_str(), "%f", &fval) == 1) {
                FloatValue() = std::clamp(fval, m_floatMin, m_floatMax);
                return true;
            }
            return false;
        case TWD_STRING:
            m_str.assign(val);
            return true;
        default:
            zassert(0);
            return false;
        }
    }
};
enum TweakIdx {
    TWI_AUTOBRAKINGMODE, TWI_AB_ONROADVISUALSMODE, TWI_STRIDEDISTANCE, TWI_TESTUPDATEREALTIME, TWI_MINPOWERPAUSETHRESHOLD,
    TWI_BIKESOUNDDISTCUTOFFCENTEMETERS, TWI_BIKERAINPARTICLEFORWARDOFFSET, TWI_BIKEDUSTPARTICLEFORWARDOFFSET, TWI_BSHOWEVENTPLACEMENT, TWI_BSHOWSPEED,
    TWI_MAXPACKETAGEINSECONDS, TWI_MAXSIDEPROJECTIONINCM, TWI_AIMFORCEMULTIPLIER, TWI_SIDEFORCEMULTIPLIER, TWI_DIRECTIONFORCEMULTIPLIER,
    TWI_NETWORKQUALITYAMPLIFIER, TWI_PROJECTIONTIMEMULTIPLIER_AUTOSTEERING, TWI_PROJECTIONTIMEMULTIPLIER_MANUALSTEERING, TWI_DEBUGROADWIDTH,
    TWI_SHOWWORLDCOORDINATE, TWI_DEBUG_LOGSPLINEDISTNETWORKID, TWI_ARCH_DIR_MALE, TWI_F3DEFFECTSTRENGTH, TWI_POSITIONACCURACYOVERNETWORK, TWI_SPAWNGAP,
    TWI_AIGROUPINGTEST, TWI_BALLOWAVATARSTOOVERFLOW, TWI_MASS, TWI_MAXDAMP, TWI_MINDAMP, TWI_LERPTIME, TWI_ALLOWPLAYERSONGRASS, TWI_DEFAULTBUTTONDELAY_MS,
    TWI_CULLDIST, TWI_SHOWHISTOGRAM, TWI_SHOWLUMINANCE, TWI_DEFAULTLUMINANCEMEAN, TWI_DEFAULTLUMINANCEMAX, TWI_HISTOGRAMLUMINANCECUTOFF, TWI_LUMINANCESUBRECTCOUNTX,
    TWI_LUMINANCESUBRECTCOUNTY, TWI_MAXBLOOMSCALE, TWI_GETTRIGGER, TWI_SHIFTINGINPUTDELAY_MS, TWI_FTMS_ENABLESIMBIKETUNING, TWI_FTMS_GRADELOOKAHEADTIME,
    TWI_COMPUTRAINERSENDTIME, TWI_ELITESENDTIME, TWI_BSHOWSTEERINGOUTRO, TWI_BAWARDMTB, TWI_IDEALBIKESPERMETER, TWI_BFORCESTEERINGUI, TWI_BFORCEQUITEVENTBUTTON,
    TWI_POSTRIDEGRAPHDISPLAYLIMIT, TWI_WORKOUTAUTOPAUSETIME, TWI_RAMPUPTIME, TWI_MINERGMODEDISABLETIME, TWI_REENABLEERGPOWERTHRESHOLD, TWI_STEADYSTATEFAILURETHRESHOLD,
    TWI_INTERVALONFAILURETHRESHOLD, TWI_INTERVALOFFFAILURETHRESHOLD, TWI_BENABLEOCULUS, TWI_BIKEMAXSHADOWCASCADE, TWI_3DTVENABLED, TWI_MINIMALUI, TWI_SSAO, TWI_FXAA, 
    TWI_BLOOM, TWI_RADIALBLUR, TWI_VIGNETTE, TWI_COLORCORRECTION, TWI_AUTOEXPOSURE, TWI_TONEMAP, TWI_SSR, TWI_DISTORTION, TWI_HEADLIGHT, TWI_HEADLIGHTINTENSITY, TWI_SUNRAYS,
    TWI_SIMPLEREFLECTIONS, TWI_BLOOMSTRENGTH, TWI_BLOOMTHRESHOLD, TWI_RAINEXPOSUREMULTIPLIER, TWI_BLURMULTIPLIER, TWI_TONEMAPEXPONENTBIAS, TWI_TONEMAPEXPONENTSCALE, 
    TWI_EXPOSURERATE, TWI_SHOWFPS, TWI_LODBIAS, TWI_3DTVSCREENDISTANCE, TWI_3DTVEFFECT, TWI_3DTVSIZEINCHES, TWI_3DTVEYESPACINGCM, TWI_FOLIAGEPERCENT, TWI_BSHOWPACKETDELAY,
    TWI_CNT
};
extern TweakInfo g_tweakArray[];

void SetupConsoleCommands();
void CONSOLE_Init();
void CONSOLE_AddCommand(const char *name, CMD_bool f1 = nullptr, CMD_static_str f2 = nullptr, CMD_ac_search f3 = nullptr, CMD_string f4 = nullptr);
bool CMD_LoadConfig(const char *);
bool CMD_SetLanguage(const char *);
bool CMD_ChangeTime(const char *);
bool CMD_PairHr(const char *);
bool CMD_PairPower(const char *);
bool CMD_ANTStartSearch(const char *);
bool CMD_ANTStopSearch(const char *);
bool CMD_TrainerSetSimMode(const char *);
bool CMD_ChangeRes(const char *);
bool CMD_ChangeShadowRes(const char *);
bool CMD_TrainerSetSimGrade(const char *);
bool CMD_ListDevices(const char *);
inline int g_trainerDelay;
bool CMD_SetTrainerDelay(const char *);
bool CMD_FocusOnCyclist(const char *);
bool CMD_ShowRaceResults(const char *);
bool CMD_TimeToTPWorkout(const char *);
bool CMD_ShowEventFinished(const char *par);
bool CMD_SetGUIObjVisible(const char *);
bool CMD_LoadBenchmarkScript(const char *);
bool CMD_EnrollInTrainingPlan(const char *);
bool CMD_ShowUI(const char *);
bool CMD_Help(const char *);
bool CMD_ToggleLog(const char *);
bool CMD_Set(const char *);
void CMD_Set3(CMD_AutoCompleteParamSearchResults *, const char *);
std::string CMD_Set4(const char *);
void StripPaddedSpaces(std::string *dest, const std::string &src);
bool CMD_ListVars(const char *);
std::string CMD_ListVars4(const char *);
void *COMMAND_GetUserdata();
void COMMAND_PopUserdata();
void COMMAND_PushUserdata(void *);
bool COMMAND_RunScript(const char *);
struct CircularVectorData {
    std::string m_name, m_params, m_descr;
    void toString(std::string *dest);
};
struct CircularVector {
    std::vector<CircularVectorData> m_vec;
    std::vector<CircularVectorData>::iterator m_iter;
    CircularVector();
    void clear();
    ~CircularVector();
    CircularVectorData *Current() const;
};
inline int g_knownCommandsCounter;
struct ConsoleCommandFuncs {
    std::string    m_name;
    CMD_bool       m_bool;
    CMD_static_str m_static_str;
    CMD_ac_search  m_ac_search;
    CMD_string     m_string;
    bool operator==(ConsoleCommandFuncs const &peer) const { return m_name == peer.m_name; }
};
inline ConsoleCommandFuncs g_knownCommands[256];
struct ConsoleRenderer {
    const static inline uint32_t TYPE_COLORS[LOG_CNT] = { 0xC8FFFFFF, 0xC837FFFF, 0xC83796FF, 0xC800C8FF, 0xC8AFDCFF, 0x6EC5BBB1, 0xC8FFFFFF, 0xC8FFFFFF, 0xC8FFFFFF, 0xC8FFFFFF, 0xC8FFFFFF, 0xC8FFFFFF, 0xC8FFFFFF, 0xC8FFFFFF };
    const static inline uint32_t LogBGColor = 0xBF332619;
    const static inline float    LargeFontScale = 0.35f; //URSOFT FIX (was 0.4) - но линии в 16.0 накладывались друг на друга

    bool        m_logVisible = true;
    float       m_cmdX1 = 3.0f;
    float       m_cmdX2 = 15.0f;
    float       m_cmdScale = LargeFontScale;
    std::string m_logBanner;
    float       m_height = 720.0f;
    float       m_width = 1280.0f;
    float       m_cmdY;
    float       m_freeHeight;
    float       m_field_40;
    float       m_top;
    float       m_delimHeight;
    float       m_atY;
    bool        m_mirrorY = true;
    void Update(float time);
};
inline ConsoleRenderer g_Console;
inline bool            g_bShowConsole
#ifdef _DEBUG
    = true
#endif
;
struct IKeyProcessor {
    virtual bool ProcessKey(int, int) = 0;
};
struct GUIKeyProcessor : public IKeyProcessor {
    bool ProcessKey(int, int) override;
};
struct GoKeyProcessor : public IKeyProcessor {
    bool ProcessKey(int, int) override;
};
struct KeyProcessorStack { //0x50 bytes
    std::vector<IKeyProcessor *> m_normal, m_locked, m_deferredRemoves;
    bool m_isLocked = false;
    void RemoveAllKeyProcessors();
    void Remove(IKeyProcessor *);
    void Push(IKeyProcessor *);
    bool ProcessKey(int, int);
    //inlined to ProcessKey void Pop();
    //inlined to ProcessKey void Find(IKeyProcessor const*,std::vector<IKeyProcessor*> &)
};
struct ZwiftAppKeyProcessorManager { //0x60 bytes
    GUIKeyProcessor   m_guiKP;
    GoKeyProcessor    m_goKP;
    KeyProcessorStack m_stack;
    //ZwiftAppKeyProcessorManager(); - 0's all
    ~ZwiftAppKeyProcessorManager() {
        Shutdown();
    }
    static ZwiftAppKeyProcessorManager *Instance();
    void Init();
    void Shutdown() {
        m_stack.RemoveAllKeyProcessors();
    }
};
bool COMMAND_RunCommandsFromFile(const char *name);
bool COMMAND_RunCommand(const char *cmd);
void CONSOLE_Draw(float atY, float dt);
void CONSOLE_KeyFilter(uint32_t codePoint, int keyModifiers);
void CONSOLE_KeyPress(int codePoint, int keyModifiers);
void CONSOLE_Paste(int cmdLen);
bool CMD_PlayWem(const char *);
bool CMD_PlayWemLocal(const char *);
