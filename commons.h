#pragma once

// --- Module ID Type ---
typedef enum {
    DISCONNECTED = 0,
    ERROR_STATE,
    BUTTON_MAINTAINED,  // DIN
    BUTTON_MOMENTARY,   // DIN
    XYAB,               // DIN x4
    DPAD,               // DIN x4
    JOYSTICK,           // AIN x2 Prefers X/Y, Z/RX
    SLIDER,             // AIN Prefers Slider0/Slider1
    TWIST_SWITCH,       // AIN Prefers Ry/Rz
    ACCEL,              // AIN x3 Prefers XYZ/RxRyRz
    GYRO,               // AIN x3 Prefers XYZ/RxRyRz
    LED                 // DOUT
} moduleID_t;

// --- HID Gamepad Axis Flags ---
// typedef enum {
//     NONE    = 0,
//     X       = 1,
//     Y       = 1 << 1,
//     Z       = 1 << 2,
//     RX      = 1 << 3,
//     RY      = 1 << 4,
//     RZ      = 1 << 5,
//     SLIDER0 = 1 << 6,
//     SLIDER1 = 1 << 7
// } axis_flags_t;

// --- HID Gamepad Axis Indices (Must Match HID Report) ---
typedef enum {
    X = 0,
    Y,
    Z,
    RX,
    RY,
    RZ,
    SLIDER0,
    SLIDER1
} axis_idx_t;

// --- HID Gamepad Button Indices (Must Match HID Report) ---
typedef uint8_t button_idx_t;

typedef struct {
    axis_idx_t axis0;
    axis_idx_t axis1;
    button_idx_t button0;
    button_idx_t button1;
    button_idx_t button2;
    button_idx_t button3;
    uint8_t report_num;
} assigned_t;

// --- Module String Descriptors ---
const char module_names[][25] =
{
    "DISCONNECTED",
    "ERROR STATE",
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