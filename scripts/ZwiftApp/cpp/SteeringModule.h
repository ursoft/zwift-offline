#pragma once
struct SteeringModule {
    BikeEntity *m_bike = nullptr;
    float m_field_40 = 0.0f;
    int m_field_20 = 0; //TODO: enum
    static SteeringModule *Self();
    void Init(BikeEntity *mb);
    bool ShouldUseDroneCam();
    /*SteeringModule::~SteeringModule()
SteeringModule::UpdateTimers(float)
SteeringModule::UpdateSteeringTutorial(void)
SteeringModule::UpdateSteeringMode(void)
SteeringModule::UpdateSteeringBranchSelection(void)
SteeringModule::UpdateRidingOffset(float)
SteeringModule::UpdateOnRoadSteering(float,float)
SteeringModule::UpdateFreeSteering(float)
SteeringModule::UpdateFreeRideAssist(float)
SteeringModule::UpdateAutoPilot(void)
SteeringModule::Update(float)
SteeringModule::StopOnRoadMovement(void)
SteeringModule::StopLaneChange(void)
SteeringModule::StartOnRoadMovement(void)
SteeringModule::SetSteeringMode(SteeringMode)
SteeringModule::SetIntersectionOptions(std::vector<IntersectionOption> const*)
SteeringModule::Self(void)
SteeringModule::ResetCollisionVFXTimers(void)
SteeringModule::Render(void)
SteeringModule::ProcessOnRoadDigital(float)
SteeringModule::ProcessOnRoadAnalog(float)
SteeringModule::OnEventEnd(void)
SteeringModule::OnDeviceSignalLost(void)
SteeringModule::ModifyRidingOffset(float)
SteeringModule::IsPackSteeringCollisionReductionEnabled(void)
SteeringModule::InitiateLaneChange(TurnDirection)
SteeringModule::Init(BikeEntity *)
SteeringModule::HandleEvent(EVENT_ID,std::__va_list)
SteeringModule::GetRidingOffsetExtents(float &,float &)
SteeringModule::GetRidableDistance(void)
SteeringModule::GetLateralConstant(void)
SteeringModule::GetAutoPilotUIProgress(void)
SteeringModule::GetAutoPilotTimeToActivate(void)
SteeringModule::DisableSteeringForEvent(void)
SteeringModule::ClampRidingOffset(float &)
SteeringModule::ClampHeadingDelta(float &,float)
SteeringModule::CalculateMaxHeadingDelta(float)
SteeringModule::ApplySteeringLimits(void)
SteeringModule::ApplySpeedAssist(float)
SteeringModule::ApplyLateralCollisionVFX(bool)
SteeringModule::ApplyBoundaryAssist(float,float)*/
};
inline SteeringModule g_SteeringModule;
