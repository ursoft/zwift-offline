#include "Joystick.h" //READY for testing
float JoyY2() {
    int cnt;
    if (g_enableJoystick) {
        auto JoystickAxes = glfwGetJoystickAxes(-1, &cnt);
        if (cnt > 4) {
            auto v1 = JoystickAxes[3]; //GLFW_GAMEPAD_AXIS_RIGHT_Y ?
            if (v1 < -0.176f)
                return v1 * 1.2135923f + 0.21359225f;
            if (v1 > 0.176f)
                return (v1 - 0.176f) * 1.2135923f;
        }
    }
    return 0.0f;
}
float JoyTrg1() {
    int cnt;
    if (g_enableJoystick) {
        auto JoystickAxes = glfwGetJoystickAxes(-1, &cnt);
        if (cnt > 4) {
            auto v1 = JoystickAxes[4]; //GLFW_GAMEPAD_AXIS_LEFT_TRIGGER ?
            if (v1 < -0.176f)
                return -v1 * 1.2135923f - 0.21359225f;
            if (v1 > 0.176f)
                return (0.176f - v1) * 1.2135923f;
        }
    }
    return 0.0f;
}
float JoyX2m() {
    int cnt;
    if (g_enableJoystick) {
        auto JoystickAxes = glfwGetJoystickAxes(-1, &cnt);
        if (cnt > 4) {
            auto v1 = JoystickAxes[2]; //GLFW_GAMEPAD_AXIS_RIGHT_X ?
            if (v1 < -0.176f)
                return -1.2135923f * (v1 + 0.176f);
        }
    }
    return 0.0f;
}
float JoyX2p() {
    int cnt;
    if (g_enableJoystick) {
        auto JoystickAxes = glfwGetJoystickAxes(-1, &cnt);
        if (cnt > 4) {
            auto v1 = JoystickAxes[2]; //GLFW_GAMEPAD_AXIS_RIGHT_X ?
            if (v1 > 0.176f)
                return 1.2135923f * (v1 - 0.176f);
        }
    }
    return 0.0f;
}
float JoyX1() {
    int cnt;
    if (g_enableJoystick) {
        auto JoystickAxes = glfwGetJoystickAxes(-1, &cnt);
        if (cnt > 0) {
            auto v1 = JoystickAxes[0]; //GLFW_GAMEPAD_AXIS_LEFT_X?
            if (v1 > -0.176f)
                return (v1 > 0.176f) ? 1.2135923f * (v1 - 0.176f) : 0.0f;
            else 
                return 1.2135923f * v1 + 0.21359225f;
        }
    }
    return 0.0f;
}
float JoyY1() {
    int cnt;
    if (g_enableJoystick) {
        auto JoystickAxes = glfwGetJoystickAxes(-1, &cnt);
        if (cnt > 0) {
            auto v1 = JoystickAxes[1]; //GLFW_GAMEPAD_AXIS_LEFT_Y?
            if (v1 >= -0.176f)
                return (v1 > 0.176f) ? 1.2135923f * (0.176f - v1) : 0.0f;
            else 
                return -1.2135923f * v1 - 0.21359225f;
        }
    }
    return 0.0f;
}
uint16_t getJoystickButtons() {
    uint16_t ret = 0;
    if (g_enableJoystick) {
        int jbsCnt;
        auto jbs = glfwGetJoystickButtons(-1, &jbsCnt);
        if (jbsCnt == 20) {
            if ((jbsCnt == 20 && strstr(glfwGetJoystickName(-1), "Microsoft PC-joystick driver")) || jbsCnt >= 14) {
                if (jbs[0] == 1) ret |= 0x1000;
                if (jbs[1] == 1) ret |= 0x2000;
                if (jbs[2] == 1) ret |= 0x4000;
                if (jbs[3] == 1) ret |= 0x8000;
                if (jbs[4] == 1) ret |= 0x100;
                if (jbs[5] == 1) ret |= 0x200;
                if (jbs[6] == 1) ret |= 0x20;
                if (jbs[7] == 1) ret |= 0x10;
                if (jbs[8] == 1) ret |= 0x40;
                if (jbs[9] == 1) ret |= 0x80;
                if (jbs[--jbsCnt] == 1) ret |= 4;
                if (jbs[--jbsCnt] == 1) ret |= 2;
                if (jbs[--jbsCnt] == 1) ret |= 8;
                if (jbs[--jbsCnt] == 1) ret |= 1;
            }
        }
    }
    return ret;
}