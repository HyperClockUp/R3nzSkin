#pragma once

#include <cstdint>

// 用于记录游戏内各数值的偏移量
namespace offsets {
	// 游戏内对象基址和偏移量
	namespace GameObject {
		// 基址
		namespace VTable {
			enum {
				IsLaneMinion = 0xE3, // 48 8B 06 48 8B CE FF 90 ? ? 00 00 48 8B ? 24 ? ? 00 00
				IsEliteMinion = IsLaneMinion + 0x1,
				IsEpicMinion = IsEliteMinion + 0x1,
				IsMinion = IsEpicMinion + 0x4,
				IsJungle = IsMinion + 0x1
			};
		};
		enum {
			// 队伍偏移量
			Team = 0x3C,
			// ID偏移量
			Name = 0x60
		};
	};

	namespace global {
		inline std::uint64_t Player{ 0 };
		inline std::uint64_t ChampionManager{ 0 };
		inline std::uint64_t Riot__g_window{ 0 };
		inline std::uint64_t ManagerTemplate_AIMinionClient_{ 0 };
		inline std::uint64_t ManagerTemplate_AIHero_{ 0 };
		inline std::uint64_t ManagerTemplate_AITurret_{ 0 };
		inline std::uint64_t GameClient{ 0 };
	};

	namespace AIBaseCommon {
		// 人物皮肤偏移量
		inline std::uint64_t CharacterDataStack{ 0 };
		// 皮肤ID偏移量
		inline std::uint64_t SkinId{ 0 };
	};

	namespace MaterialRegistry {
		inline std::uint64_t D3DDevice{ 0 };
		inline std::uint64_t SwapChain{ 0 };
	};

	namespace functions {
		inline std::uint64_t Riot__Renderer__MaterialRegistry__GetSingletonPtr{ 0 };
		inline std::uint64_t translateString_UNSAFE_DONOTUSE{ 0 };
		inline std::uint64_t CharacterDataStack__Push{ 0 };
		inline std::uint64_t CharacterDataStack__Update{ 0 };
		inline std::uint64_t GetGoldRedirectTarget{ 0 };
	};
};
