#pragma once
struct GroupEvent { //39 bytes
    //TODO
};
struct GroupEvents {
    static std::vector<GroupEvent> g_GroupEvents;
    static void Initialize(Experimentation *exp);
    static void Shutdown();
    static void *FindSubgroupEvent(int64_t id);
};