#pragma once

#include <cstddef>

// ����������ʶ����a##b �ᱻԤ�������滻Ϊ ab
#define CONCAT(a, b) a##b
// Ϊÿ�����������һ��Ψһ�ı�ʶ������pad��ͷ������pad1��pad2��pad3��
#define PAD_NAME(n) CONCAT(pad, n)
// ����ָ����С���ֽ����
#define PAD(size) \
private: \
    std::byte PAD_NAME(__LINE__)[size]; \
public:

// ����ƫ������ȡ���Ա��ȡ��this�ĵ�ַ������ƫ�������ٽ�����
#define CLASS_GETTER(returnType, name, offset) \
[[nodiscard]] inline returnType name() const noexcept \
{ \
	return *reinterpret_cast<returnType*>(std::uintptr_t(this) + offset); \
}

// ����ƫ������ȡ���Աָ�룬ȡ��this�ĵ�ַ������ƫ���������ָ��ƫ�����ĵ�ַ
#define CLASS_GETTER_P(returnType, name, offset) \
[[nodiscard]] inline returnType* name() const noexcept \
{ \
	return reinterpret_cast<returnType*>(std::uintptr_t(this) + offset); \
}

// ģ�庯�������ڵ����麯��
// �麯����C++�Ķ�̬�л��õ����麯���ĵ�����ͨ�������ʵ�ֵģ������һ������ָ�����飬����д�ŵ����麯���ĵ�ַ
// ��������д�˸�����麯��ʱ������д�ŵľ���������麯����ַ
template <std::size_t Index, typename ReturnType, typename... Args>
ReturnType CallVirtual(std::uintptr_t instance, Args... args)
{
	// ����һ������ָ�룬���ڵ����麯��
	using Fn = ReturnType(__fastcall*)(std::uintptr_t, Args...);
	// ͨ��instance��ȡ�����ͨ������ȡ�麯��
	const auto function{ (*reinterpret_cast<Fn**>(instance))[Index] };
	// �����麯���ĵ��ý��
	return function(instance, args...);
}