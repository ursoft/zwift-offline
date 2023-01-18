#include "ZwiftApp.h"
#include "FitnessDeviceManager.h"

void SetupConsoleCommands() {
	CONSOLE_Init();
	CONSOLE_AddCommand("loadconfig", CMD_LoadConfig);
	CONSOLE_AddCommand("time", CMD_Time);
	CONSOLE_AddCommand("pairhr", CMD_PairHr);
	CONSOLE_AddCommand("pairpower", CMD_PairPower);
	CONSOLE_AddCommand("antstartsearch", CMD_AntStartSearch);
	CONSOLE_AddCommand("antstopsearch", CMD_AntStopSearch);
	CONSOLE_AddCommand("trainersetsimmode", CMD_TrainerSetSimMode);
	CONSOLE_AddCommand("trainersetsimgrade", CMD_TrainerSetSimGrade);
	CONSOLE_AddCommand("listdevices", CMD_ListDevices);
	CONSOLE_AddCommand("settrainerdelay", CMD_SetTrainerDelay);
	CONSOLE_AddCommand("focus", CMD_Focus);
	CONSOLE_AddCommand("raceresults", CMD_RaceResults);
	CONSOLE_AddCommand("time_to_tp_workout", CMD_Time_to_tp_workout);
	CONSOLE_AddCommand("evfin", CMD_EvFin);
	CONSOLE_AddCommand("set_object_visible", CMD_SetObjectVisible);
	CONSOLE_AddCommand("benchmark", CMD_Benchmark);
	CONSOLE_AddCommand("enroll_in_trainingplan", CMD_EnrollInTrainingPlan);
	CONSOLE_AddCommand("show_ui", CMD_ShowUI);
}

void CONSOLE_Init() {
	CONSOLE_AddCommand("help", CMD_Help);
	CONSOLE_AddCommand("togglelog", CMD_ToggleLog);
	CONSOLE_AddCommand("set", CMD_Set, nullptr, CMD_Set3, CMD_Set4);
	CONSOLE_AddCommand("listvars", CMD_ListVars, nullptr, nullptr, CMD_ListVars4);
}

int g_knownCommandsCounter;
ConsoleCommandFuncs g_knownCommands[256];
void CONSOLE_AddCommand(const char *name, CMD_bool f1, CMD_static_str f2, CMD_ac_search f3, CMD_string f4) {
	assert(g_knownCommandsCounter < sizeof(g_knownCommands) / sizeof(g_knownCommands[0]));
	ConsoleCommandFuncs newObj { name, f1, f2, f3, f4 };
	for (int i = 0; i < g_knownCommandsCounter; i++)
		if (g_knownCommands[i] == newObj)
			return;
	g_knownCommands[g_knownCommandsCounter++];
}

DWORD g_MainThread;
const char *GAMEPATH(const char *path) {
	assert(g_MainThread == GetCurrentThreadId());
	return path;
}

bool COMMAND_RunCommandsFromFile(const char *name) {
	char buf[1024 + 4];
	sprintf_s(buf, "data/configs/%s.txt", name);
	FILE *f = nullptr;
	fopen_s(&f, buf, "r");
	if (nullptr == f)
		return false;
	while (!feof(f)) {
		buf[0] = 0;
		fgets(buf, sizeof(buf) - 1, f);
		if(*buf)
			COMMAND_RunCommand(buf);
	}
	fclose(f);
	return true;
}
bool COMMAND_RunCommand(const char *cmd) {
	/*TODO*/
	return true;
}
bool CMD_LoadConfig(const char *par) {
	return COMMAND_RunCommandsFromFile(par);
}
bool CMD_Time(const char *) {
	return true;
}
bool CMD_PairHr(const char *) {
	return true;
}
bool CMD_PairPower(const char *) {
	return true;
}
bool CMD_AntStartSearch(const char *) {
	return true;
}
bool CMD_AntStopSearch(const char *) {
	return true;
}
bool CMD_TrainerSetSimMode(const char *) {
	return true;
}
bool CMD_ChangeRes(const char *) {
	return true;
}
bool CMD_ChangeShadowRes(const char *) {
	return true;
}
bool CMD_TrainerSetSimGrade(const char *arg) {
	float v = 0.0;
	sscanf_s(arg, "%f", &v);
	if (v < -1.0 || v >= 1.0)
		return false;
	FitnessDeviceManager::TrainerSetSimGrade(v);
	return true;
}
bool CMD_ListDevices(const char *) {
	return true;
}
int g_TrainerDelay;
bool CMD_SetTrainerDelay(const char *arg) {
	int v;
	if (sscanf_s(arg, "%d", &v) != 1)
		return false;
	g_TrainerDelay = v;
	return true;
}
bool CMD_Focus(const char *) {
	return true;
}
bool CMD_RaceResults(const char *) {
	return true;
}
bool CMD_Time_to_tp_workout(const char *) {
	return true;
}
bool CMD_EvFin(const char *) {
	return true;
}
bool CMD_SetObjectVisible(const char *) {
	return true;
}
bool CMD_Benchmark(const char *) {
	return true;
}
bool CMD_EnrollInTrainingPlan(const char *) {
	return true;
}
bool CMD_ShowUI(const char *) {
	return true;
}
bool CMD_Help(const char *) {
	LogTyped(LT_NONAMED, "Known Commands: ");
	for (int i = 0; i < g_knownCommandsCounter; i++)
		LogTyped(LT_NONAMED, "%s", g_knownCommands[i].m_name);
	return true;
}
bool g_ShowLog;
bool CMD_ToggleLog(const char *) {
	g_ShowLog = !g_ShowLog;
	return true;
}
bool CMD_Set(const char *) {
	return true;
}
CMD_AutoCompleteParamSearchResults CMD_Set3(const char *) {
	return TODO;
}
std::string CMD_Set4(const char *) {
	return "";
}
bool CMD_ListVars(const char *) {
	return true;
}
std::string CMD_ListVars4(const char *) {
	return "";
}
