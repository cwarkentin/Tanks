#pragma once
#include <cstdint>

struct InputPacket {
    uint32_t frame;
    bool up, down, left, right, fire;
};