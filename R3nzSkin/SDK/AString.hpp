#pragma once

#include <cstdint>

class AString {
public:
	// ×Ö·û´®ÄÚÈÝ
	const char* str;
	// ×Ö·û´®³¤¶È
	std::int32_t length;
	// ×Ö·û´®ÈÝÁ¿
	std::int32_t capacity;
};
