#pragma once

#include <cstdint>

#include "AString.hpp"
#include "Pad.hpp"

// 角色的皮肤数据
class CharacterStackData {
public:
    // 模型
    AString model;
    PAD(0x10)
    // 皮肤id
    std::int32_t skin;
    PAD(0x60)
    // 武器
    std::int8_t gear;
    PAD(0x7)
};
