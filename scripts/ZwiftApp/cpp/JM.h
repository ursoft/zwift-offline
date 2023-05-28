#pragma once
enum JobType { JT_CPC = 2, JT_CNT };
enum JobPriority { JP_NORMAL };
struct JobData {
    int m_size; //in bytes
    //payload here
};
void JM_AddJob(int (*)(JobData *), JobData *, JobType, void (*)(int, JobData *), JobPriority);
void JM_BeginAddJobs();
void JM_BlockOnJobTypeComplete(JobType, bool);
void JM_EndAddJobs();
void JM_EndOfFrameCleanup();
void JM_GetHWThreadAssignment(uint32_t);
void JM_GetNextJob_internal(JobType);
void JM_Initialize();
void JM_IsComplete(uint32_t);
void JM_Shutdown();
void JM_Worker_Thread(void *);