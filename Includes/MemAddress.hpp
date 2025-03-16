#pragma once

#include <3ds.h>

enum value_address : u32
{
    cameraFOV = 0x100000 + 0x2CEE80,
    playerReachDistance = 0x100000 + 0x54FB68,
    playerSwimSpeed = 0x100000 + 0x3EA090,
    playerPositionX = 0xAC1E48,
    playerPositionY = 0xAC1E4C,
    playerPositionZ = 0xAC1E50,
    cloudsHeight = 0x100000 + 0x2C5398
};