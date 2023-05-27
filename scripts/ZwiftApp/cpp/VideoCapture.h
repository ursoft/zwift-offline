#pragma once
struct VideoCaptureEvent {
    static void OnDisabled();
};
struct VideoCapture {
    static void ShutdownVideoCapture();
    static void InitVideoCapture();
    inline static std::unique_ptr<VideoCapture> g_pVideoCapture;
};