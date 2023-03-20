#pragma once
struct CMD_AutoCompleteParamSearchResults {
    std::string m_descr;
    std::vector<std::string> m_field_20;
    bool        m_field_38 = false;
};
typedef bool (*CMD_bool)(const char *);
typedef char * (*CMD_static_str)(const char *);
typedef void (*CMD_ac_search)(CMD_AutoCompleteParamSearchResults *, const char *);
typedef std::string (*CMD_string)(const char *);

void SetupConsoleCommands();
void CONSOLE_Init();
void CONSOLE_AddCommand(const char *name, CMD_bool f1 = nullptr, CMD_static_str f2 = nullptr, CMD_ac_search f3 = nullptr, CMD_string f4 = nullptr);
bool CMD_LoadConfig(const char *);
bool CMD_SetLanguage(const char *);
bool CMD_Time(const char *);
bool CMD_PairHr(const char *);
bool CMD_PairPower(const char *);
bool CMD_AntStartSearch(const char *);
bool CMD_AntStopSearch(const char *);
bool CMD_TrainerSetSimMode(const char *);
bool CMD_ChangeRes(const char *);
bool CMD_ChangeShadowRes(const char *);
bool CMD_TrainerSetSimGrade(const char *);
bool CMD_ListDevices(const char *);
inline int g_trainerDelay;
bool CMD_SetTrainerDelay(const char *);
bool CMD_Focus(const char *);
bool CMD_RaceResults(const char *);
bool CMD_Time_to_tp_workout(const char *);
bool CMD_EvFin(const char *);
bool CMD_SetObjectVisible(const char *);
bool CMD_Benchmark(const char *);
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
    const static inline float    LargeFontScale = 0.4f;

    bool        m_logVisible = true;
    float       m_cmdX1 = 3.0f;
    float       m_cmdX2 = 15.0f;
    float       m_cmdScale = LargeFontScale;
    std::string m_logBanner; //used in autocomplete = "Zwift Debug Console (toggle with ` key and Ctrl+F12)";
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

bool COMMAND_RunCommandsFromFile(const char *name);
bool COMMAND_RunCommand(const char *cmd);
void CONSOLE_Draw(float atY, float dt);
void CONSOLE_KeyFilter(uint32_t codePoint, int keyModifiers);
void CONSOLE_KeyPress(int codePoint, int keyModifiers);
void CONSOLE_Paste(int cmdLen);

//non-zwift: console redirection (useful for debugging and unit testing)
namespace non_zwift {
class ConsoleHandler {
    bool m_releaseNeed;
public:
    ConsoleHandler(int16_t minLength);
    bool LaunchUnitTests(int argc, char **argv);
    ~ConsoleHandler();
};
}