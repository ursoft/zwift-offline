namespace PlayerStateHelper {
    inline uint32_t getMapRevisionId(const protobuf::PlayerState &state, bool *pSuccess) {
        *pSuccess = false;
        if (state.has_f19()) {
            *pSuccess = true;
            return HIWORD(state.f19());
        }
        auto result = (uint32_t)state.world();
        if (!result)
            return HIWORD(state.f19());
        *pSuccess = true;
        return result;
    }
    inline char getWorkoutMode(const protobuf::PlayerState &state) { return state.progress() & 0xF; }
    inline bool getHasPhoneConnected(const protobuf::PlayerState &state) { return (state.f19() & 2) != 0; }
    inline std::string getPosition(const protobuf::PlayerState &state) {
        char buf[128];
        sprintf_s(buf, "%f,%f", state.z(), state.x());
        return buf;
    }
    inline bool hasPosition(const protobuf::PlayerState &state) { return state.has_x() && state.has_z(); }
}