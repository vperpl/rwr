#include "client.hpp"

#include <Windows.h>
#include <iomanip>
#include <iostream>
#include <chrono>

static bool IsKeyDown(int vk)
{
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}

class KeyLatch
{
public:
    bool pressed(int vk)
    {
        bool now = IsKeyDown(vk);
        bool result = now && !wasDown;
        wasDown = now;
        return result;
    }

private:
    bool wasDown = false;
};

static DWORD NowMs()
{
    return GetTickCount();
}

static void PrintVector3(const char* name, const Vector3& v)
{
    std::cout << name << ": "
        << std::fixed << std::setprecision(6)
        << v.a << ", "
        << v.b << ", "
        << v.c << std::endl;
}

static void DisableQuickEditMode()
{
    HANDLE hInput = GetStdHandle(STD_INPUT_HANDLE);
    if (hInput == INVALID_HANDLE_VALUE || hInput == NULL)
    {
        std::cout << "[WARN] Could not get console input handle.\n";
        return;
    }
        DWORD mode = 0;
    if (!GetConsoleMode(hInput, &mode))
    {
        std::cout << "[WARN] GetConsoleMode failed. Error: " << GetLastError() << "\n";
        return;
    }

    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode |= ENABLE_EXTENDED_FLAGS;

    if (!SetConsoleMode(hInput, mode))
    {
        std::cout << "[WARN] SetConsoleMode failed. Error: " << GetLastError() << "\n";
    }

}

int main()
{
    SetConsoleTitleA("RWR tool debug build");
    SetConsoleOutputCP(CP_UTF8);
    DisableQuickEditMode();

    std::cout << "Running with Rifles Xan-Hack\n";
    std::cout << "###################################\n";
    std::cout << "Keys:\n";
    std::cout << std::setw(28) << std::left << "Home" << "- Disables Fog of War - MAINMENU!\n";
    std::cout << std::setw(28) << std::left << "Insert" << "- Toggle super camera\n";
    std::cout << std::setw(28) << std::left << "Page Up / Page Down" << "- Adjust camera height\n";
    std::cout << std::setw(28) << std::left << "Right Click" << "- Triggerbot (auto-shoot)\n";
    std::cout << std::setw(28) << std::left << "Left Alt" << "- Teleport to crosshair\n";
    std::cout << std::setw(28) << std::left << "F1" << "- Toggle debug logs\n";
    std::cout << std::setw(28) << std::left << "End" << "- Exit\n";
    std::cout << "###################################\n\n";

    std::cout << "[INFO] Tool pointer size: " << sizeof(void*) * 8 << "-bit\n";
    std::cout << "[INFO] If game is 64-bit, build this as x64.\n";
    std::cout << "[INFO] If game is 32-bit, build this as Win32/x86.\n\n";

    Memory memory;

    if (memory.pID == NULL)
    {
        std::cout << "[FATAL] Game process not found.\n";
        std::cout << "Make sure rwr_game.exe is running before starting this tool.\n";
        system("pause");
        return 1;
    }

    if (memory.processHandle == NULL || memory.processHandle == INVALID_HANDLE_VALUE)
    {
        std::cout << "[FATAL] Invalid process handle.\n";
        std::cout << "OpenProcess failed or returned invalid handle.\n";
        system("pause");
        return 1;
    }

    if (memory.gameBaseAddress == NULL)
    {
        std::cout << "[FATAL] Base address is 0.\n";
        std::cout << "Module rwr_game.exe was not found inside the target process.\n";
        system("pause");
        return 1;
    }

    std::cout << "[OK] Process initialized.\n";
    std::cout << "[OK] PID: " << memory.pID << "\n";
    std::cout << "[OK] Base address: 0x"
        << std::hex << memory.gameBaseAddress
        << std::dec << "\n\n";

    Client client(memory);

    ULONG_PTR fowAddress = memory.gameBaseAddress + 0xB7EAF + 8;
    std::cout << "[OK] FOW patch address (offset): 0x"
        << std::hex << fowAddress << std::dec << "\n";

    BYTE fowPatchEnable[8] = {
        0xC6, 0x83, 0xD9, 0x12, 0x00, 0x00, 0x00,
        0x90
    };

    memory.patch_bytes(fowAddress, fowPatchEnable, 8);
    std::cout << "[OK] FOW disabled at startup.\n";

    bool superCameraEnabled = false;
    bool debugLogs = true;

    KeyLatch homeKey;
    KeyLatch insertKey;
    KeyLatch f1Key;
    KeyLatch endKey;

    DWORD lastAltLog = 0;
    DWORD lastShootLog = 0;
    DWORD lastTeleport = 0;
    DWORD lastPageAdjust = 0;
    DWORD lastHeartbeat = 0;

    std::cout << "  [HOME]  Disable fog of war\n";
    std::cout << "[INSERT]  Super camera toggle\n";
    std::cout << "  [F1]    Debug logs\n";
    std::cout << "  [END]   Exit\n\n";

    while (true)
    {
        Sleep(10);

        DWORD now = NowMs();

        if (endKey.pressed(VK_END))
        {
            std::cout << "[INFO] END pressed. Exiting.\n";
            break;
        }

        if (f1Key.pressed(VK_F1))
        {
            debugLogs = !debugLogs;
            std::cout << "[INFO] Debug logs: "
                << (debugLogs ? "ON" : "OFF")
                << std::endl;
        }

        if (homeKey.pressed(VK_HOME))
        {
            memory.patch_bytes(fowAddress, fowPatchEnable, 8);
            std::cout << "[FOW] Fog of war disabled (patch re-applied)\n";
        }

        if (insertKey.pressed(VK_INSERT))
        {
            superCameraEnabled = !superCameraEnabled;
            if (superCameraEnabled)
            {
                client.set_camera_angles(Vector3{0.0f, -2.2f, 0.0000001f});
                std::cout << "[CAMERA] Super camera ON\n";
            }
            else
            {
                client.set_camera_angles(Vector3{-0.15f, -0.85f, 0.5f});
                std::cout << "[CAMERA] Super camera OFF\n";
            }
        }

        if (debugLogs && now - lastHeartbeat > 3000)
        {
            std::cout << "[ALIVE] Loop running. "
                << "ALT=" << IsKeyDown(VK_LMENU)
                << " SHIFT=" << IsKeyDown(VK_LSHIFT)
                << std::endl;

            lastHeartbeat = now;
        }

        if (IsKeyDown(VK_RBUTTON))
        {
            int crosshair_status = client.get_crosshair_status();

            if (debugLogs && now - lastAltLog > 250)
            {
                std::cout << "[KEY] Right Click held. crosshair_status = "
                    << crosshair_status << std::endl;
                lastAltLog = now;
            }

            if (crosshair_status == 1)
            {
                client.shoot();

                if (debugLogs && now - lastShootLog > 250)
                {
                    std::cout << "[ACTION] shoot() called.\n";
                    lastShootLog = now;
                }
            }
        }

        if (IsKeyDown(VK_LMENU))
        {
            if (now - lastTeleport > 150)
            {
                if (debugLogs)
                {
                    std::cout << "[KEY] Left Alt held. teleport_to(crosshair_position) called.\n";
                }

                auto crosshairPos = client.get_crosshair_position();
                client.teleport_to(crosshairPos);

                lastTeleport = now;
            }
        }

        if (superCameraEnabled)
        {
            if (now - lastPageAdjust > 50)
            {
                if (IsKeyDown(VK_PRIOR))
                {
                    Vector3 camera_angles = client.get_camera_angles();

                    if (debugLogs)
                    {
                        PrintVector3("[PAGEUP] Current camera", camera_angles);
                    }

                    camera_angles.b -= 0.05f;
                    client.set_camera_angles(camera_angles);

                    if (debugLogs)
                    {
                        PrintVector3("[PAGEUP] New camera    ", camera_angles);
                    }

                    lastPageAdjust = now;
                }
                else if (IsKeyDown(VK_NEXT))
                {
                    Vector3 camera_angles = client.get_camera_angles();

                    if (debugLogs)
                    {
                        PrintVector3("[PAGEDOWN] Current camera", camera_angles);
                    }

                    camera_angles.b += 0.05f;
                    client.set_camera_angles(camera_angles);

                    if (debugLogs)
                    {
                        PrintVector3("[PAGEDOWN] New camera    ", camera_angles);
                    }

                    lastPageAdjust = now;
                }
            }
        }
    }

    return 0;

}
