#pragma once

#include "GameState.hpp"
#include "Pad.hpp"

class GameClient {
	// ����һ����СΪ 0xC ���ֽ����
	PAD(0xC)
	// ��¼��Ϸ״̬
	GGameState_s game_state;
};
