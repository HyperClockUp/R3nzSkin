#pragma once

#include <cstdint>

// 游戏状态
enum class GGameState_s : std::int32_t {
	// 加载中
	LoadingScreen = 0,
	// 连接中
	Connecting = 1,
	// 运行中
	Running = 2,
	// 暂停中
	Paused = 3,
	// 结束
	Finished = 4,
	// 退出
	Exiting = 5
};
