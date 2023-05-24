#pragma once

#include <cstdint>
#include <string>

#include "../offsets.hpp"
#include "Pad.hpp"

// 游戏内对象基类
class GameObject {
public:
	CLASS_GETTER_P(std::string, get_name, offsets::GameObject::Name)
	CLASS_GETTER(std::int32_t, get_team, offsets::GameObject::Team)

	// Returns true for lane minions.
	// 判断是否是线上小兵
	[[nodiscard]] bool isLaneMinion() const noexcept { return CallVirtual<offsets::GameObject::VTable::IsLaneMinion, bool>(std::uintptr_t(this)); }

	// Returns true for blue, red and crab.
	// 对红Buff、蓝Buff、河蟹返回true
	[[nodiscard]] bool isEliteMinion() const noexcept { return CallVirtual<offsets::GameObject::VTable::IsEliteMinion, bool>(std::uintptr_t(this)); }

	// Returns true for dragon, baron, and rift.
	// 对大龙、小龙、男爵、峡谷先锋返回true
	[[nodiscard]] bool isEpicMinion() const noexcept { return CallVirtual<offsets::GameObject::VTable::IsEpicMinion, bool>(std::uintptr_t(this)); }

	// Returns true for minion.
	// 对小兵返回true
	[[nodiscard]] bool isMinion() const noexcept { return CallVirtual<offsets::GameObject::VTable::IsMinion, bool>(std::uintptr_t(this)); }

	// Returns true for objects that both teams can damage, such as jungle objects, gangplain barrels, etc.
	// 对双方都可以攻击的对象返回true，如野怪、炮台、河蟹、水晶、水晶炮、小龙、大龙、男爵、峡谷先锋
	[[nodiscard]] bool isJungle() const noexcept { return CallVirtual<offsets::GameObject::VTable::IsJungle, bool>(std::uintptr_t(this)); }
};
