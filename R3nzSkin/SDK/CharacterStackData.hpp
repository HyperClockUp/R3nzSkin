#pragma once

#include <cstdint>

#include "AString.hpp"
#include "Pad.hpp"

// ��ɫ��Ƥ������
class CharacterStackData {
public:
    // ģ��
    AString model;
    PAD(0x10)
    // Ƥ��id
    std::int32_t skin;
    PAD(0x60)
    // ����
    std::int8_t gear;
    PAD(0x7)
};
