#pragma once

#include "GameState.hpp"
#include "Pad.hpp"

class GameClient {
	// 创建一个大小为 0xC 的字节填充
	PAD(0xC)
	// 记录游戏状态
	GGameState_s game_state;
};
