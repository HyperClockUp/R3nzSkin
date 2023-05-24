#pragma once

#include <cstdint>
#include <vector>

#include "CharacterStackData.hpp"

class CharacterDataStack {
public:
	// ���е�Ƥ������
	std::vector<CharacterStackData> stack;
	// ����Ƥ��
	CharacterStackData base_skin;
	// ����Ƥ������
	void update(const bool change) const noexcept;
	// ��Ƥ������ѹ��ջ
	void push(const char* model, const std::int32_t skin) const noexcept;
};
