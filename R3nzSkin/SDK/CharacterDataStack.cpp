#include <cstdint>

#include "../CheatManager.hpp"
#include "../Offsets.hpp"

#include "CharacterDataStack.hpp"

void CharacterDataStack::push(const char* model, const std::int32_t skin) const noexcept
{
	// ����һ�������ú�������
	using push_t = __int64(__fastcall*)(std::uintptr_t, const char*, std::int32_t, std::int32_t, bool, bool, bool, bool, bool, bool, std::int8_t, const char*, std::int32_t, const char*, std::int32_t, bool, std::int32_t);
	// ͨ��ƫ�ƻ�ȡ�����ú�����ַ
	static const auto _push{ reinterpret_cast<push_t>(cheatManager.memory->base + offsets::functions::CharacterDataStack__Push) };
	// ���û�������
	_push(std::uintptr_t(this), model, skin, 0, false, false, false, false, true, false, -1, "\x00", 0, "\x00", 0, false, 1);
}

void CharacterDataStack::update(const bool change) const noexcept
{
	// ����һ�������ú�������
	static const auto _update{ reinterpret_cast<__int64(__fastcall*)(std::uintptr_t, bool)>(cheatManager.memory->base + offsets::functions::CharacterDataStack__Update) };
	// ���ø���Ƥ�����ݺ���
	_update(std::uintptr_t(this), change);
}
