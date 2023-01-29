#include "ZwiftApp.h"
std::unique_ptr<PlayerAchievementService> sPlayerAchievementServicePtr;
void PlayerAchievementService::Initialize(EventSystem *ev) {
	zassert(sPlayerAchievementServicePtr.get() == nullptr);
	sPlayerAchievementServicePtr.reset(new PlayerAchievementService(ev));
}
PlayerAchievementService *PlayerAchievementService::Instance() {
	zassert(sPlayerAchievementServicePtr.get() != nullptr);
	return sPlayerAchievementServicePtr.get();
}
bool PlayerAchievementService::IsInitialized() {
	return !!sPlayerAchievementServicePtr.get();
}
PlayerAchievementService::PlayerAchievementService(EventSystem *ev) : EventObject(ev) {
	//TODO
	ev->Subscribe(EV_RESET, this);
	m_start = timeGetTime();
}
void PlayerAchievementService::HandleEvent(EVENT_ID e, va_list va) {
	if (e == EV_RESET) {
		//TODO
	}
}
void PlayerAchievementService::HandleLogout() {
	//TODO
}
void PlayerAchievementService::LoadAchievements() {
	//TODO
}
void PlayerAchievementService::LoadAchievementsFailure(ZNet::Error, uint32_t) {
	//TODO
}
void PlayerAchievementService::LoadAchievementsSuccess(const protobuf::Achievements &a, uint32_t) {
	//TODO
}
void PlayerAchievementService::LoadRideHistory() {
	//TODO
}
void PlayerAchievementService::PersistAchievements() {
	//TODO
}
void PlayerAchievementService::PersistAchievementsFailure(std::vector<int>, ZNet::Error) {
	//TODO
}
void PlayerAchievementService::PersistAchievementsSuccess() {
	//TODO
}
void PlayerAchievementService::SetAchievement(Achievement) {
	//TODO
}
void PlayerAchievementService::Shutdown() {
	//TODO
}
void PlayerAchievementService::Update(float dt) {
	//TODO
}
void PlayerAchievementService::listPlayerAchievements() {
	//TODO
}
PlayerAchievementService::~PlayerAchievementService() {
	m_eventSystem->Unsubscribe(EV_RESET, this);
	/*TODO
	  ZNet::WaitForPendingRequests<std::vector<ZNet::RequestId>>(&this->field_28, "PlayerAchievementService", 24LL);
  std::__tree<std::__value_type<unsigned int,std::bitset<512ul>>,std::__map_value_compare<unsigned int,std::__value_type<unsigned int,std::bitset<512ul>>,std::less<unsigned int>,true>,std::allocator<std::__value_type<unsigned int,std::bitset<512ul>>>>::destroy(
    &this->field_A0,
    *(_QWORD *)&this->field_A8);
  std::__tree<std::__value_type<unsigned int,std::vector<unsigned int>>,std::__map_value_compare<unsigned int,std::__value_type<unsigned int,std::vector<unsigned int>>,std::less<unsigned int>,true>,std::allocator<std::__value_type<unsigned int,std::vector<unsigned int>>>>::destroy(
    &this->field_88,
    *(_QWORD *)&this->field_90);
  v3 = *(void **)&this->field_28;
  if ( v3 )
  {
    *(_QWORD *)&this->field_30 = v3;
    operator delete(v3);
  }
*/
}
