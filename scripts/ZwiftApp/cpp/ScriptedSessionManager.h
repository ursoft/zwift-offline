#pragma once
struct HUDOverrides {
    bool m_field_D = false;
};
struct ScriptedSessionManager {
    ScriptedSessionManager();
    HUDOverrides m_hudOverrides;
    bool m_field_C = false;
    static ScriptedSessionManager *Inst() {
        static ScriptedSessionManager *g_ScriptedSessionManager = new ScriptedSessionManager();
        return g_ScriptedSessionManager;
    }
/*ScriptedSessionManager::EndSession(void)
ScriptedSessionManager::GetHUDOverrides(void)
ScriptedSessionManager::GetLocalizedText(ScriptedItem const *)
ScriptedSessionManager::GetSession(ScriptedSessionType, bool)
ScriptedSessionManager::GetSessionTypeFromName(char const *)
ScriptedSessionManager::GetTextOverrides(void)
ScriptedSessionManager::GetTutorialWorkout(void)
ScriptedSessionManager::GetWorldTimeOverrides(void)
ScriptedSessionManager::HasCompletedScriptedSession(char const *)
ScriptedSessionManager::HasUserRiddenMoreThanRequiredDistanceForAutomaticTutorials(void)
ScriptedSessionManager::IsAnySessionActive(void)
ScriptedSessionManager::IsLanguageSupported(LOC_LANGS)
ScriptedSessionManager::IsOnboardingSessionType(ScriptedSessionType)
ScriptedSessionManager::IsSessionActive(ScriptedSessionType)
ScriptedSessionManager::PauseSession(void)
ScriptedSessionManager::ReloadSessionData(void)
ScriptedSessionManager::Render(void)
ScriptedSessionManager::RequestSession(ScriptedSessionType)
ScriptedSessionManager::RequestSession(ScriptedSessionType, bool)
ScriptedSessionManager::RequestSession(char const *)
ScriptedSessionManager::ResumeSession(void)
ScriptedSessionManager::SetEnabled(bool)
ScriptedSessionManager::SetScriptedSessionCompleted(char const *, bool)
ScriptedSessionManager::SetTutorialCompletedFlags(bool)
ScriptedSessionManager::ShouldPlaySessionFromDropInOrPairingScreen(ScriptedSessionType)
ScriptedSessionManager::ShouldPlaySessionFromInGame(ScriptedSessionType)
ScriptedSessionManager::Update(float)
ScriptedSessionManager::WasUserRecentlyDroppedIntoWorld(void)
ScriptedSessionManager::~ScriptedSessionManager()*/
};