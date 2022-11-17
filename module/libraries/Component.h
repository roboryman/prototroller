#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"

#define BUTTON_MODULE_ID   0xFF
#define JOYSTICK_MODULE_ID 0xFE

static const bool LOGGING = true;

class Component {
    public:
        Component();
};