#include <Windows.h>
#include <cstdlib>
#include <fstream>
#include <psapi.h>
#include <string>
#include <thread>
#include <tlhelp32.h>

#include "Injector.hpp"
#include "R3nzUI.hpp"
#include "lazy_importer.hpp"

// 查找符合名称的进程
proclist_t WINAPI Injector::findProcesses(const std::wstring name) noexcept
{
	// 获取指定进程的快照，以及这些进程使用的堆、模块和线程
	auto process_snap{ LI_FN(CreateToolhelp32Snapshot)(TH32CS_SNAPPROCESS, 0) };
	// 创建进程列表
	proclist_t list;

	// 如果快照无效，则返回空列表
	if (process_snap == INVALID_HANDLE_VALUE)
		return list;
	// 创建一个结构体，用于存储进程信息
	PROCESSENTRY32W pe32{};
	pe32.dwSize = sizeof(PROCESSENTRY32W);

	// 遍历进程列表，将符合名称的进程添加到列表中
	if (LI_FN(Process32FirstW).get()(process_snap, &pe32)) {
		if (pe32.szExeFile == name)
			list.push_back(pe32.th32ProcessID);

		while (LI_FN(Process32NextW).get()(process_snap, &pe32)) {
			if (pe32.szExeFile == name)
				list.push_back(pe32.th32ProcessID);
		}
	}

	// 关闭快照句柄
	LI_FN(CloseHandle)(process_snap);
	return list;
}

// 判断进程是否已注入
bool WINAPI Injector::isInjected(const std::uint32_t pid) noexcept
{
	// 打开进程，获取进程的详细信息和可读权限
	auto hProcess{ LI_FN(OpenProcess)(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid) };

	// 如果打开进程失败，则返回false
	if (NULL == hProcess)
		return false;

	HMODULE hMods[1024];
	DWORD cbNeeded{};

	// 获取进程依赖的DLL句柄
	if (LI_FN(K32EnumProcessModules)(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (auto i{ 0u }; i < (cbNeeded / sizeof(HMODULE)); ++i) {
			// 获取DLL的名称
			TCHAR szModName[MAX_PATH]{};
			// 如果获取DLL名称成功，则判断DLL名称是否为R3nzSkin.dll
			if (LI_FN(K32GetModuleBaseNameW)(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				if (std::wcscmp(szModName, L"R3nzSkin.dll") == 0) {
					// 关闭进程句柄，返回true
					LI_FN(CloseHandle)(hProcess);
					return true;
				}
			}
		}
	}
	// 关闭进程句柄，返回false
	LI_FN(CloseHandle)(hProcess);
	return false;
}

bool WINAPI Injector::inject(const std::uint32_t pid) noexcept
{
	// 获取当前目录
	TCHAR current_dir[MAX_PATH];
	LI_FN(GetCurrentDirectoryW)(MAX_PATH, current_dir);
	// 打开进程，获取进程的完全访问权限
	const auto handle{ LI_FN(OpenProcess)(PROCESS_ALL_ACCESS, false, pid) };

	// 如果打开进程失败，则返回false
	if (!handle || handle == INVALID_HANDLE_VALUE)
		return false;

	FILETIME ft{};
	SYSTEMTIME st{};
	// 获取当前时间
	LI_FN(GetSystemTime)(&st);
	// 将当前时间转换为文件时间
	LI_FN(SystemTimeToFileTime)(&st, &ft);
	// 获取进程的创建时间、退出时间、内核时间和用户时间
	FILETIME create{}, exit{}, kernel{}, user{};
	LI_FN(GetProcessTimes)(handle, &create, &exit, &kernel, &user);

	// 计算进程创建时间与当前时间的差值，防止进程创建时间过短导致注入失败
	const auto delta{ 10 - static_cast<std::int32_t>((*reinterpret_cast<std::uint64_t*>(&ft) - *reinterpret_cast<std::uint64_t*>(&create.dwLowDateTime)) / 10000000U) };

	// 如果差值大于0，则等待差值秒
	if (delta > 0)
		// 等待delta秒
		std::this_thread::sleep_for(std::chrono::seconds(delta));

	// 获取R3nzSkin.dll的路径
	const auto dll_path{ std::wstring(current_dir) + L"\\R3nzSkin.dll" };

	// 如果R3nzSkin.dll文件不存在，则返回false
	if (auto f{ std::ifstream(dll_path) }; !f.is_open()) {
		LI_FN(MessageBox)(nullptr, L"R3nzSkin.dll file could not be found.\nTry reinstalling the cheat.", L"R3nzSkin", MB_ICONERROR | MB_OK);
		LI_FN(CloseHandle)(handle);
		return false;
	}

	// 在进程中分配内存，用于存储R3nzSkin.dll的路径
	const auto dll_path_remote{ LI_FN(VirtualAllocEx).get()(handle, nullptr, (dll_path.size() + 1) * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) };

	// 如果分配内存失败，则返回false
	if (!dll_path_remote) {
		LI_FN(CloseHandle)(handle);
		return false;
	}

	// 将R3nzSkin.dll的路径写入到进程中
	if (!LI_FN(WriteProcessMemory).get()(handle, dll_path_remote, dll_path.data(), (dll_path.size() + 1) * sizeof(wchar_t), nullptr)) {
		// 如果写入失败，则释放内存，关闭进程句柄，返回false
		LI_FN(VirtualFreeEx).get()(handle, dll_path_remote, 0u, MEM_RELEASE);
		LI_FN(CloseHandle)(handle);
		return false;
	}

	HANDLE thread{};
	LI_FN(NtCreateThreadEx).nt_cached()(
		// 指定的线程句柄
		&thread,
		// 所有访问权限
		GENERIC_ALL,
		// 默认安全描述符
		NULL,
		// 指定的进程句柄
		handle,
		// 先获取kernel32.dll的基址，再获取LoadLibraryW的地址
		reinterpret_cast<LPTHREAD_START_ROUTINE>(LI_FN(GetProcAddress).get()(LI_FN(GetModuleHandleW).get()(L"kernel32.dll"), "LoadLibraryW")),
		// R3nzSkin.dll在进程中的预先存储的路径
		dll_path_remote,
		// 创建线程时立即运行
		FALSE, 
		// 线程的堆栈大小
		NULL, 
		// 线程的初始堆栈
		NULL,
		// 线程的初始上下文
		NULL,
		// 线程的扩展信息
		NULL
		);

	// 如果创建线程失败，则释放内存，关闭进程句柄，返回false
	if (!thread || thread == INVALID_HANDLE_VALUE) {
		LI_FN(VirtualFreeEx).get()(handle, dll_path_remote, 0u, MEM_RELEASE);
		LI_FN(CloseHandle)(handle);
		return false;
	}

	// 等待线程结束，释放内存，关闭进程句柄，返回true
	LI_FN(WaitForSingleObject)(thread, INFINITE);
	LI_FN(CloseHandle)(thread);
	LI_FN(VirtualFreeEx).get()(handle, dll_path_remote, 0u, MEM_RELEASE);
	LI_FN(CloseHandle)(handle);
	return true;
}

// 对进程开启debug特权，以开启对进程的完全访问权限
void WINAPI Injector::enableDebugPrivilege() noexcept
{
	HANDLE token{};
	if (OpenProcessToken(LI_FN(GetCurrentProcess).get()(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
		LUID value;
		if (LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &value)) {
			TOKEN_PRIVILEGES tp{};
			tp.PrivilegeCount = 1;
			tp.Privileges[0].Luid = value;
			tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if (AdjustTokenPrivileges(token, FALSE, &tp, sizeof(tp), NULL, NULL))
				LI_FN(CloseHandle)(token);
		}
	}
}

// 生成随机字符串
std::string Injector::randomString(std::uint32_t size) noexcept
{
	static auto& alphanum = "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;
	tmp_s.reserve(size);

	while(size--)
		tmp_s += alphanum[std::rand() % (sizeof(alphanum) - 1)];

	return tmp_s;
}

// 重命名自身的exe文件
void Injector::renameExe() noexcept
{
	char szExeFileName[MAX_PATH];
	// nullptr表示获取当前进程的可执行文件名或路径
	LI_FN(GetModuleFileNameA)(nullptr, szExeFileName, MAX_PATH);

	const auto path{ std::string(szExeFileName) };
	const auto exe{ path.substr(path.find_last_of("\\") + 1, path.size()) };
	const auto newName{ randomString(std::rand() % (10 - 7 + 1) + 7) + ".exe" };

	std::rename(exe.c_str(), newName.c_str());
}

// 主函数
void Injector::run() noexcept
{
	// 1、开启debug特权
	enableDebugPrivilege();
	// 2、监听LeagueClient.exe和League of Legends.exe的进程
	while (true) {
		// LOL客户端进程PID
		const auto& league_client_processes{ Injector::findProcesses(L"LeagueClient.exe") };
		// LOL游戏进程PID
		const auto& league_processes{ Injector::findProcesses(L"League of Legends.exe") };

		// LOL进程是否打开
		R3nzSkinInjector::gameState = (league_processes.size() > 0) ? true : false;
		// LOL客户端是否打开
		R3nzSkinInjector::clientState = (league_client_processes.size() > 0) ? true : false;
		
		// antiviruses don't like endless loops, show them that this loop is a breaking point. (technically still an infinite loop :D)
		if (league_processes.size() > 0xff)
			break;

		// 3、如果LOL游戏进程打开，则注入R3nzSkin.dll
		for (auto& pid : league_processes) {
			// 首先判断进程是否已经注入
			if (!Injector::isInjected(pid)) {
				R3nzSkinInjector::cheatState = false;
				// 如果用户开启cheat，则注入
				if (R3nzSkinInjector::btnState) {
					std::this_thread::sleep_for(1.5s);
					// 注入DLL
					if (Injector::inject(pid))
						R3nzSkinInjector::cheatState = true;
					else
						R3nzSkinInjector::cheatState = false;
				}
				std::this_thread::sleep_for(1s);
			} else {
				R3nzSkinInjector::cheatState = true;
			}
		}
		// 每次循环间隔1s，让出CPU时间片
		std::this_thread::sleep_for(1s);
	}
}
