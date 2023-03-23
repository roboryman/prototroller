#pragma once

// --- Module ID Type ---
typedef enum {
    DISCONNECTED = 0,
    ERROR_STATE,
    BUTTON_MAINTAINED,
    BUTTON_MOMENTARY,
    XYAB,
    DPAD,
    JOYSTICK,
    SLIDER,
    TWIST_SWITCH,
    ACCEL,
    GYRO,
    LED
} moduleID_t;

// --- Module String Descriptors ---
const char module_names[][25] =
{
    "DISCONNECTED",
    "BUTTON MAINTAINED MODULE",
    "BUTTON MOMENTARY MODULE",
    "XYAB MODULE",
    "DPAD MODULE",
    "JOYSTICK MODULE",
    "SLIDER MODULE",
    "TWIST SWITCH MODULE",
    "ACCELEROMETER MODULE",
    "GYROSCOPE MODULE",
    "LED MODULE"
};