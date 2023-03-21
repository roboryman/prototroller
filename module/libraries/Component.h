#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"

static const bool LOGGING = true;

class Component {
    public:
        Component();
};