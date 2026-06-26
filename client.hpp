#pragma once
#include "memory.hpp"
class Client
{
public:
	Client(Memory memory)
	{
		this->memory = memory;
	}
	int get_crosshair_status()
	{
		ULONG_PTR offsetToGameBaseAdress = 0x004FF5A0;
		int crosshairStatusOffsets[] = {0x8, 0x24, 0x2c, 0x8, 0x8, 0x1494, 0x8};

		ULONG_PTR crosshairStatusAddress = NULL;

		crosshairStatusAddress = memory.FindPointer(7, memory.gameBaseAddress + offsetToGameBaseAdress, crosshairStatusOffsets);

		int crosshair_status = memory.read_mem<int>(crosshairStatusAddress);
		return crosshair_status;
	}

	void shoot()
	{
		mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
		Sleep(20);
		mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
	}

	Vector3 get_camera_angles()
	{
		ULONG_PTR offsetToGameBaseAdress = 0x004FF5A0;
		int offsets[] = {0x8, 0x24, 0x2c, 0x8, 0x94, 0x3c};

		ULONG_PTR camera_angles_addr = memory.FindPointer(6, memory.gameBaseAddress + offsetToGameBaseAdress, offsets);

		return memory.read_mem<Vector3>(camera_angles_addr);
	}

	void set_camera_angles(Vector3 angles)
	{
		ULONG_PTR offsetToGameBaseAdress = 0x004FF5A0;
		int offsets[] = {0x8, 0x24, 0x2c, 0x8, 0x94, 0x3c};

		ULONG_PTR camera_angles_addr = memory.FindPointer(6, memory.gameBaseAddress + offsetToGameBaseAdress, offsets);

		memory.write_mem(camera_angles_addr, angles);
	}

	Point get_current_position()
	{
		ULONG_PTR offsetToGameBaseAdress = 0x004FF5A0;
		int playerPositonPoint_Offsets[] = {0x2c, 0x20, 0x24, 0x20, 0x0, 0x34, 0xD18, 0x4, 0xB4, 0x4};

		ULONG_PTR playerPositionPoint_addr = memory.FindPointer(10, memory.gameBaseAddress + offsetToGameBaseAdress, playerPositonPoint_Offsets);

		TwoPoint twoPoint = memory.read_mem<TwoPoint>(playerPositionPoint_addr);
		return Point{twoPoint.x1, twoPoint.z1, twoPoint.y1};
	}

	void teleport_to(Point point)
	{
		teleport_to(point.x, point.z, point.y);
	}

	Point get_player_position()
	{
		ULONG_PTR offsetToGameBaseAdress = 0x004FF5A0;
		int playerPositonPoint_Offsets[] = {0x8, 0x24, 0x2c, 0x8, 0x8, 0xD18, 0x4, 0xB4, 0x4 };
		ULONG_PTR addr = memory.FindPointer(9, memory.gameBaseAddress + offsetToGameBaseAdress, playerPositonPoint_Offsets);
		TwoPoint tp = memory.read_mem<TwoPoint>(addr);
		return Point{tp.x1, tp.z1, tp.y1};
	}

	void teleport_to(float x, float z, float y)
	{
		TwoPoint to_point = {x, z, y, x, z, y};

		ULONG_PTR offsetToGameBaseAdress = 0x004FF5A0;
		int playerPositonPoint_Offsets[] = {0x8, 0x24, 0x2c, 0x8, 0x8, 0xD18, 0x4, 0xB4, 0x4 };

		ULONG_PTR playerPositionPoint_addr = memory.FindPointer(9, memory.gameBaseAddress + offsetToGameBaseAdress, playerPositonPoint_Offsets);

		memory.write_mem(playerPositionPoint_addr, to_point);
	}

	Point get_crosshair_position()
	{
		ULONG_PTR offsetToGameBaseAdress = 0x004FF5A0;
		int crosshairPositionOffsets[] = {0x8, 0x24, 0x2c, 0x8, 0x8, 0x1494, 0x28};

		ULONG_PTR crosshairPosition_addr = memory.FindPointer(7, memory.gameBaseAddress + offsetToGameBaseAdress, crosshairPositionOffsets);

		Point crosshairPosition = memory.read_mem<Point>(crosshairPosition_addr);

		return crosshairPosition;
	}

private:
	Memory memory;
};
