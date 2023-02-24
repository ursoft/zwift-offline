#pragma once
class VideoCaptureEvent {
public:
    static void OnDisabled();
};
class VideoCapture {
public:
    static void ShutdownVideoCapture();
    static void InitVideoCapture();
    inline static std::unique_ptr<VideoCapture> g_pVideoCapture;
};