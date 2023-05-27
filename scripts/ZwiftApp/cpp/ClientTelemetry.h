#pragma once
struct ClientTelemetry : public EventObject { //640 bytes
    ClientTelemetry(Experimentation *exp, EventSystem *eventSystem);
    static void Initialize(Experimentation *exp, EventSystem *eventSystem);
    static void Shutdown() { g_sClientTelemetryUPtr.reset(); }
    static inline std::unique_ptr<ClientTelemetry> g_sClientTelemetryUPtr;
    void HandleEvent(EVENT_ID, va_list) override;
    ~ClientTelemetry();
};