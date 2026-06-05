#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <tchar.h>
#include <vector>
#include <string>

class Memory {
public:
	DWORD pID = NULL;
	HANDLE processHandle = NULL;
	ULONG_PTR gameBaseAddress = NULL;

	Memory() {
		pID = get_porcId_by_name("rwr_game.exe");
		if (pID == NULL) {
			std::cout << "Failed to launch!" << std::endl;
			std::cout << "Please launch the game before running the hack!" << std::endl;
			return;
		}
		std::cout << "PID: " << pID << std::endl;

		processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
		if (processHandle == INVALID_HANDLE_VALUE || processHandle == NULL) {
			std::cout << "Failed to open process" << std::endl;
			return;
		}

		char gameName[] = "rwr_game.exe";
		gameBaseAddress = GetModuleBaseAddress(gameName, pID);
		std::cout << "Base address: 0x" << std::hex << gameBaseAddress << std::dec << std::endl;
	}

	template <typename var>
	bool write_mem(ULONG_PTR address, var value) {
		return WriteProcessMemory(processHandle, (LPVOID)address, &value, sizeof(var), NULL);
	}

	template <typename var>
	var read_mem(ULONG_PTR address) {
		var value = {};
		ReadProcessMemory(processHandle, (LPCVOID)address, &value, sizeof(var), NULL);
		return value;
	}

	ULONG_PTR FindPointer(int offset_num, ULONG_PTR baseaddr, int offsets[])
	{
		ULONG_PTR Address = baseaddr;
		for (int i = 0; i < offset_num; i++)
		{
			ReadProcessMemory(processHandle, (LPCVOID)Address, &Address, sizeof(ULONG_PTR), NULL);
			Address += offsets[i];
		}
		return Address;
	}

private:
	ULONG_PTR GetModuleBaseAddress(const char* lpszModuleName, DWORD pID) {
		ULONG_PTR dwModuleBaseAddress = 0;
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pID);
		MODULEENTRY32 ModuleEntry32 = { 0 };
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);

		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do {
				if (strcmp(ModuleEntry32.szModule, lpszModuleName) == 0)
				{
					dwModuleBaseAddress = (ULONG_PTR)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32));
		}
		CloseHandle(hSnapshot);
		return dwModuleBaseAddress;
	}

	static DWORD get_porcId_by_name(const std::string& targetProcess) {
		DWORD procId = 0;
		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hSnap != INVALID_HANDLE_VALUE)
		{
			PROCESSENTRY32 procEntry;
			procEntry.dwSize = sizeof(procEntry);
			if (Process32First(hSnap, &procEntry))
			{
				do
				{
					if (targetProcess == procEntry.szExeFile)
					{
						procId = procEntry.th32ProcessID;
					}
				} while (Process32Next(hSnap, &procEntry));
			}
		}
		CloseHandle(hSnap);
		return procId;
	}
};

struct Vector3 {
	float a;
	float b;
	float c;
};

struct Point {
	float x;
	float z;
	float y;
};

struct TwoPoint {
	float x1;
	float z1;
	float y1;
	float x2;
	float z2;
	float y2;
};
