#include "ZwiftApp.h"
void VideoCaptureEvent::OnDisabled() {
}
void VideoCapture::ShutdownVideoCapture() {
    g_pVideoCapture.reset();
    VideoCaptureEvent::OnDisabled();
}
