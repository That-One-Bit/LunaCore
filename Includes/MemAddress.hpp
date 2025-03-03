#pragma once

#include <3ds.h>

enum value_address : u32
{
    cameraFOV = 0x100000 + 0x2CEE80,
    playerReachDistance = 0x100000 + 0x54FB68,
    playerSwimSpeed = 0x100000 + 0x3EA090,
    cloudsHeight = 0x100000 + 0x2C5398
};