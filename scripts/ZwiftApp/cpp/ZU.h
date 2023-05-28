#pragma once
void ZU_ConvertScreenPosToRay(const VEC2 &, VEC3 &);
void ZU_ConvertScreenPosToWorldPos(const VEC2 &, VEC3 &);
void ZU_ConvertWorldPosToScreenPos(const VEC3 &, VEC3 &);
void ZU_GetTimeStampVars(int *, int *, int *, int *, int *, int *);
bool ZU_IsDateInPast(uint64_t, uint64_t, uint64_t);
bool ZU_IsInPreReleaseRestrictedMode(int courseId);
bool ZU_IsInPreReleaseRestrictedMode();
void ZU_RayTriangleIntersection(const VEC3 &, const VEC3 &, const VEC3 &, const VEC3 &, const VEC3 &, VEC3 &);
