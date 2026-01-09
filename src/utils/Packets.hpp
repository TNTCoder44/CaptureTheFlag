#pragma once

#include <cstdint>
#include <raylib.h>

#include "../core/Infantry.hpp"

enum class TroopType : uint8_t
{
    Infantry = 0,
    Cavallry = 1,
    Artillery = 2,
    Change = 3,
    None = 255
};

// Ensure tight packing of the struct for comatibility
#pragma pack(push, 1)

struct PacketData
{
    TroopType type;
    int entityId; // stable ID shared between peers
    int desiredPos[2];
};

#pragma pack(pop)
