#pragma once

enum CMD_AutoCompleteParamSearchResults { TODO };

typedef bool (*CMD_bool)(const char *);
typedef char * (*CMD_static_str)(const char *);
typedef CMD_AutoCompleteParamSearchResults (*CMD_ac_search)(const char *);
typedef std::string (*CMD_string)(const char *);

void SetupConsoleCommands();
void CONSOLE_Init();
void CONSOLE_AddCommand(const char *name, CMD_bool f1 = nullptr, CMD_static_str f2 = nullptr, CMD_ac_search f3 = nullptr, CMD_string f4 = nullptr);
bool CMD_LoadConfig(const char *);
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
extern int g_trainerDelay;
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
extern bool g_ShowLog;
bool CMD_ToggleLog(const char *);
bool CMD_Set(const char *);
CMD_AutoCompleteParamSearchResults CMD_Set3(const char *);
std::string CMD_Set4(const char *);
bool CMD_ListVars(const char *);
std::string CMD_ListVars4(const char *);

//extern int g_knownCommandsCounter;
struct ConsoleCommandFuncs {
	const char *m_name;
	CMD_bool m_bool;
	CMD_static_str m_static_str;
	CMD_ac_search m_ac_search;
	CMD_string m_string;
	bool operator==(ConsoleCommandFuncs const &) const = default;
};
//extern ConsoleCommandFuncs g_knownCommands[];
bool  COMMAND_RunCommandsFromFile(const char *name);
bool COMMAND_RunCommand(const char *cmd);