#pragma once

#include <cstdint>

constexpr uint32_t hash(const char *str)
{
    uint32_t hash_ = 0;
    while (*str)
    {
        hash_ += *str;
        hash_ &= 0xFFFFFFFF;
        hash_ += (hash_ << 10);
        hash_ &= 0xFFFFFFFF;
        hash_ ^= (hash_ >> 6);
        str++;
    }
    hash_ += (hash_ << 3);
    hash_ &= 0xFFFFFFFF;
    hash_ ^= (hash_ >> 11);
    hash_ += (hash_ << 15);
    return hash_ & 0xFFFFFFFF;
}