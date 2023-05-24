#pragma once

#include <cstddef>

// 连接两个标识符，a##b 会被预处理器替换为 ab
#define CONCAT(a, b) a##b
// 为每个宏调用生成一个唯一的标识符，以pad开头，例如pad1、pad2、pad3等
#define PAD_NAME(n) CONCAT(pad, n)
// 生成指定大小的字节填充
#define PAD(size) \
private: \
    std::byte PAD_NAME(__LINE__)[size]; \
public:

// 按照偏移量获取类成员，取得this的地址，加上偏移量，再解引用
#define CLASS_GETTER(returnType, name, offset) \
[[nodiscard]] inline returnType name() const noexcept \
{ \
	return *reinterpret_cast<returnType*>(std::uintptr_t(this) + offset); \
}

// 按照偏移量获取类成员指针，取得this的地址，加上偏移量，获得指定偏移量的地址
#define CLASS_GETTER_P(returnType, name, offset) \
[[nodiscard]] inline returnType* name() const noexcept \
{ \
	return reinterpret_cast<returnType*>(std::uintptr_t(this) + offset); \
}

// 模板函数，用于调用虚函数
// 虚函数：C++的多态中会用到，虚函数的调用是通过虚表来实现的，虚表是一个函数指针数组，虚表中存放的是虚函数的地址
// 当子类重写了父类的虚函数时，虚表中存放的就是子类的虚函数地址
template <std::size_t Index, typename ReturnType, typename... Args>
ReturnType CallVirtual(std::uintptr_t instance, Args... args)
{
	// 声明一个函数指针，用于调用虚函数
	using Fn = ReturnType(__fastcall*)(std::uintptr_t, Args...);
	// 通过instance获取虚表，再通过虚表获取虚函数
	const auto function{ (*reinterpret_cast<Fn**>(instance))[Index] };
	// 返回虚函数的调用结果
	return function(instance, args...);
}