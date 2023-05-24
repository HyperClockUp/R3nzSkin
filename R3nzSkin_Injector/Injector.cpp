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

// ���ҷ������ƵĽ���
proclist_t WINAPI Injector::findProcesses(const std::wstring name) noexcept
{
	// ��ȡָ�����̵Ŀ��գ��Լ���Щ����ʹ�õĶѡ�ģ����߳�
	auto process_snap{ LI_FN(CreateToolhelp32Snapshot)(TH32CS_SNAPPROCESS, 0) };
	// ���������б�
	proclist_t list;

	// ���������Ч���򷵻ؿ��б�
	if (process_snap == INVALID_HANDLE_VALUE)
		return list;
	// ����һ���ṹ�壬���ڴ洢������Ϣ
	PROCESSENTRY32W pe32{};
	pe32.dwSize = sizeof(PROCESSENTRY32W);

	// ���������б����������ƵĽ�����ӵ��б���
	if (LI_FN(Process32FirstW).get()(process_snap, &pe32)) {
		if (pe32.szExeFile == name)
			list.push_back(pe32.th32ProcessID);

		while (LI_FN(Process32NextW).get()(process_snap, &pe32)) {
			if (pe32.szExeFile == name)
				list.push_back(pe32.th32ProcessID);
		}
	}

	// �رտ��վ��
	LI_FN(CloseHandle)(process_snap);
	return list;
}

// �жϽ����Ƿ���ע��
bool WINAPI Injector::isInjected(const std::uint32_t pid) noexcept
{
	// �򿪽��̣���ȡ���̵���ϸ��Ϣ�Ϳɶ�Ȩ��
	auto hProcess{ LI_FN(OpenProcess)(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid) };

	// ����򿪽���ʧ�ܣ��򷵻�false
	if (NULL == hProcess)
		return false;

	HMODULE hMods[1024];
	DWORD cbNeeded{};

	// ��ȡ����������DLL���
	if (LI_FN(K32EnumProcessModules)(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
		for (auto i{ 0u }; i < (cbNeeded / sizeof(HMODULE)); ++i) {
			// ��ȡDLL������
			TCHAR szModName[MAX_PATH]{};
			// �����ȡDLL���Ƴɹ������ж�DLL�����Ƿ�ΪR3nzSkin.dll
			if (LI_FN(K32GetModuleBaseNameW)(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR))) {
				if (std::wcscmp(szModName, L"R3nzSkin.dll") == 0) {
					// �رս��̾��������true
					LI_FN(CloseHandle)(hProcess);
					return true;
				}
			}
		}
	}
	// �رս��̾��������false
	LI_FN(CloseHandle)(hProcess);
	return false;
}

bool WINAPI Injector::inject(const std::uint32_t pid) noexcept
{
	// ��ȡ��ǰĿ¼
	TCHAR current_dir[MAX_PATH];
	LI_FN(GetCurrentDirectoryW)(MAX_PATH, current_dir);
	// �򿪽��̣���ȡ���̵���ȫ����Ȩ��
	const auto handle{ LI_FN(OpenProcess)(PROCESS_ALL_ACCESS, false, pid) };

	// ����򿪽���ʧ�ܣ��򷵻�false
	if (!handle || handle == INVALID_HANDLE_VALUE)
		return false;

	FILETIME ft{};
	SYSTEMTIME st{};
	// ��ȡ��ǰʱ��
	LI_FN(GetSystemTime)(&st);
	// ����ǰʱ��ת��Ϊ�ļ�ʱ��
	LI_FN(SystemTimeToFileTime)(&st, &ft);
	// ��ȡ���̵Ĵ���ʱ�䡢�˳�ʱ�䡢�ں�ʱ����û�ʱ��
	FILETIME create{}, exit{}, kernel{}, user{};
	LI_FN(GetProcessTimes)(handle, &create, &exit, &kernel, &user);

	// ������̴���ʱ���뵱ǰʱ��Ĳ�ֵ����ֹ���̴���ʱ����̵���ע��ʧ��
	const auto delta{ 10 - static_cast<std::int32_t>((*reinterpret_cast<std::uint64_t*>(&ft) - *reinterpret_cast<std::uint64_t*>(&create.dwLowDateTime)) / 10000000U) };

	// �����ֵ����0����ȴ���ֵ��
	if (delta > 0)
		// �ȴ�delta��
		std::this_thread::sleep_for(std::chrono::seconds(delta));

	// ��ȡR3nzSkin.dll��·��
	const auto dll_path{ std::wstring(current_dir) + L"\\R3nzSkin.dll" };

	// ���R3nzSkin.dll�ļ������ڣ��򷵻�false
	if (auto f{ std::ifstream(dll_path) }; !f.is_open()) {
		LI_FN(MessageBox)(nullptr, L"R3nzSkin.dll file could not be found.\nTry reinstalling the cheat.", L"R3nzSkin", MB_ICONERROR | MB_OK);
		LI_FN(CloseHandle)(handle);
		return false;
	}

	// �ڽ����з����ڴ棬���ڴ洢R3nzSkin.dll��·��
	const auto dll_path_remote{ LI_FN(VirtualAllocEx).get()(handle, nullptr, (dll_path.size() + 1) * sizeof(wchar_t), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE) };

	// ��������ڴ�ʧ�ܣ��򷵻�false
	if (!dll_path_remote) {
		LI_FN(CloseHandle)(handle);
		return false;
	}

	// ��R3nzSkin.dll��·��д�뵽������
	if (!LI_FN(WriteProcessMemory).get()(handle, dll_path_remote, dll_path.data(), (dll_path.size() + 1) * sizeof(wchar_t), nullptr)) {
		// ���д��ʧ�ܣ����ͷ��ڴ棬�رս��̾��������false
		LI_FN(VirtualFreeEx).get()(handle, dll_path_remote, 0u, MEM_RELEASE);
		LI_FN(CloseHandle)(handle);
		return false;
	}

	HANDLE thread{};
	LI_FN(NtCreateThreadEx).nt_cached()(
		// ָ�����߳̾��
		&thread,
		// ���з���Ȩ��
		GENERIC_ALL,
		// Ĭ�ϰ�ȫ������
		NULL,
		// ָ���Ľ��̾��
		handle,
		// �Ȼ�ȡkernel32.dll�Ļ�ַ���ٻ�ȡLoadLibraryW�ĵ�ַ
		reinterpret_cast<LPTHREAD_START_ROUTINE>(LI_FN(GetProcAddress).get()(LI_FN(GetModuleHandleW).get()(L"kernel32.dll"), "LoadLibraryW")),
		// R3nzSkin.dll�ڽ����е�Ԥ�ȴ洢��·��
		dll_path_remote,
		// �����߳�ʱ��������
		FALSE, 
		// �̵߳Ķ�ջ��С
		NULL, 
		// �̵߳ĳ�ʼ��ջ
		NULL,
		// �̵߳ĳ�ʼ������
		NULL,
		// �̵߳���չ��Ϣ
		NULL
		);

	// ��������߳�ʧ�ܣ����ͷ��ڴ棬�رս��̾��������false
	if (!thread || thread == INVALID_HANDLE_VALUE) {
		LI_FN(VirtualFreeEx).get()(handle, dll_path_remote, 0u, MEM_RELEASE);
		LI_FN(CloseHandle)(handle);
		return false;
	}

	// �ȴ��߳̽������ͷ��ڴ棬�رս��̾��������true
	LI_FN(WaitForSingleObject)(thread, INFINITE);
	LI_FN(CloseHandle)(thread);
	LI_FN(VirtualFreeEx).get()(handle, dll_path_remote, 0u, MEM_RELEASE);
	LI_FN(CloseHandle)(handle);
	return true;
}

// �Խ��̿���debug��Ȩ���Կ����Խ��̵���ȫ����Ȩ��
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

// ��������ַ���
std::string Injector::randomString(std::uint32_t size) noexcept
{
	static auto& alphanum = "_0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	std::string tmp_s;
	tmp_s.reserve(size);

	while(size--)
		tmp_s += alphanum[std::rand() % (sizeof(alphanum) - 1)];

	return tmp_s;
}

// �����������exe�ļ�
void Injector::renameExe() noexcept
{
	char szExeFileName[MAX_PATH];
	// nullptr��ʾ��ȡ��ǰ���̵Ŀ�ִ���ļ�����·��
	LI_FN(GetModuleFileNameA)(nullptr, szExeFileName, MAX_PATH);

	const auto path{ std::string(szExeFileName) };
	const auto exe{ path.substr(path.find_last_of("\\") + 1, path.size()) };
	const auto newName{ randomString(std::rand() % (10 - 7 + 1) + 7) + ".exe" };

	std::rename(exe.c_str(), newName.c_str());
}

// ������
void Injector::run() noexcept
{
	// 1������debug��Ȩ
	enableDebugPrivilege();
	// 2������LeagueClient.exe��League of Legends.exe�Ľ���
	while (true) {
		// LOL�ͻ��˽���PID
		const auto& league_client_processes{ Injector::findProcesses(L"LeagueClient.exe") };
		// LOL��Ϸ����PID
		const auto& league_processes{ Injector::findProcesses(L"League of Legends.exe") };

		// LOL�����Ƿ��
		R3nzSkinInjector::gameState = (league_processes.size() > 0) ? true : false;
		// LOL�ͻ����Ƿ��
		R3nzSkinInjector::clientState = (league_client_processes.size() > 0) ? true : false;
		
		// antiviruses don't like endless loops, show them that this loop is a breaking point. (technically still an infinite loop :D)
		if (league_processes.size() > 0xff)
			break;

		// 3�����LOL��Ϸ���̴򿪣���ע��R3nzSkin.dll
		for (auto& pid : league_processes) {
			// �����жϽ����Ƿ��Ѿ�ע��
			if (!Injector::isInjected(pid)) {
				R3nzSkinInjector::cheatState = false;
				// ����û�����cheat����ע��
				if (R3nzSkinInjector::btnState) {
					std::this_thread::sleep_for(1.5s);
					// ע��DLL
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
		// ÿ��ѭ�����1s���ó�CPUʱ��Ƭ
		std::this_thread::sleep_for(1s);
	}
}
