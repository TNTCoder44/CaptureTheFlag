#pragma once

#include <cstdint>
#include <raylib.h>

#include "../core/Infantry.hpp"

enum class TroopType : uint8_t
{
    Infantry = 0,
    Cavallry = 1,
    Artillery = 2
};

// Ensure tight packing of the struct for comatibility
#pragma pack(push, 1)

struct PacketData 
{
    TroopType type;
    Vector2 desiredPosition;
};

#pragma pack(pop)

