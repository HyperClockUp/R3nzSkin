#pragma once

#include <cstdint>
#include <vector>

#include "CharacterStackData.hpp"

class CharacterDataStack {
public:
	// 所有的皮肤数据
	std::vector<CharacterStackData> stack;
	// 基础皮肤
	CharacterStackData base_skin;
	// 更新皮肤数据
	void update(const bool change) const noexcept;
	// 把皮肤数据压入栈
	void push(const char* model, const std::int32_t skin) const noexcept;
};
