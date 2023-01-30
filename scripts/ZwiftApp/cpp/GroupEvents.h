#pragma once
class GroupEvent { //39 bytes
public:

};
class GroupEvents {
public:
    static std::vector<GroupEvent> g_GroupEvents;
    static void Initialize(Experimentation *exp);
    static void Shutdown();
};