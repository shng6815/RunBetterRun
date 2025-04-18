#include "MapManager.h"
#include <fstream>

HRESULT MapManager::Init()
{
	mapData.height = 24;
	mapData.width = 24;
	mapData.tiles.resize(mapData.height * mapData.width);

	for (int i = 0; i < 24; i++)
	{
		for (int j = 0; j < 24; j++)
		{
			if (i == 0 || i == 23 || j == 0 || j == 23)
			{
				SetTile(i, j, RoomType::WALL,4);
			}
			else if (i == 10 && j == 10)
			{
				SetTile(i, j, RoomType::WALL, 3);
			}
			else
			{
				SetTile(i, j, RoomType::FLOOR,10);
			}
		}
	}
	return S_OK;
}

HRESULT MapManager::Init(LPCWCH filePath)
{
	return S_OK;
}

void MapManager::Release()
{
}

bool MapManager::LoadMap(const LPCWCH filePath)
{

	/*HANDLE hFile = CreateFile(
		filePath, GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	MapData newMap;
	DWORD bytesRead = 0;
	BOOL readResult = FALSE;

	int nameLength = 0;
	readResult = ReadFile(hFile, &nameLength, sizeof(int), &bytesRead, NULL);

	if (!readResult || bytesRead != sizeof(int) || nameLength <= 0)
	{
		CloseHandle(hFile);
		return false;
	}

	char* nameBuffer = new char[nameLength + 1];
	readResult = ReadFile(hFile, nameBuffer, nameLength, &bytesRead, NULL);

	if (!readResult || bytesRead != nameLength)
	{
		delete[] nameBuffer;
		CloseHandle(hFile);
		return false;
	}

	nameBuffer[nameLength] = '\0';
	newMap.name = nameBuffer;
	delete[] nameBuffer;

	readResult = ReadFile(hFile, &newMap.width, sizeof(int), &bytesRead, NULL);
	if (!readResult || bytesRead != sizeof(int))
	{
		CloseHandle(hFile);
		return false;
	}

	readResult = ReadFile(hFile, &newMap.height, sizeof(int), &bytesRead, NULL);
	if (!readResult || bytesRead != sizeof(int))
	{
		CloseHandle(hFile);
		return false;
	}

	int tileCount = 0;
	readResult = ReadFile(hFile, &tileCount, sizeof(int), &bytesRead, NULL);
	if (!readResult || bytesRead != sizeof(int))
	{
		CloseHandle(hFile);
		return false;
	}

	if (tileCount > 0)
	{
		newMap.tiles.resize(tileCount);
		readResult = ReadFile(hFile, newMap.tiles.data(), sizeof(int) * tileCount, &bytesRead, NULL);
		if (!readResult || bytesRead != sizeof(int) * tileCount)
		{
			CloseHandle(hFile);
			return false;
		}
	}
	CloseHandle(hFile);*/

	/*  mapDataList[mapName] = newMap;
	  currMapName = mapName;*/

	return true;
}

bool MapManager::SaveMap(const LPCWCH filePath)
{
	/*auto it = mapDataList.find(mapName);
	if (it == mapDataList.end())
	{
		return false;
	}

	const MapData& mapData = it->second;

	HANDLE hFile = CreateFile(
		filePath, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD bytesWritten = 0;
	BOOL writeResult = FALSE;

	int nameLength = mapData.name.length();
	writeResult = WriteFile(hFile, &nameLength, sizeof(int), &bytesWritten, NULL);
	if (!writeResult || bytesWritten != sizeof(int))
	{
		CloseHandle(hFile);
		return false;
	}

	writeResult = WriteFile(hFile, mapData.name.c_str(), nameLength, &bytesWritten, NULL);
	if (!writeResult || bytesWritten != nameLength)
	{
		CloseHandle(hFile);
		return false;
	}

	writeResult = WriteFile(hFile, &mapData.width, sizeof(int), &bytesWritten, NULL);
	if (!writeResult || bytesWritten != sizeof(int))
	{
		CloseHandle(hFile);
		return false;
	}

	writeResult = WriteFile(hFile, &mapData.height, sizeof(int), &bytesWritten, NULL);
	if (!writeResult || bytesWritten != sizeof(int))
	{
		CloseHandle(hFile);
		return false;
	}

	int tileCount = mapData.tiles.size();
	writeResult = WriteFile(hFile, &tileCount, sizeof(int), &bytesWritten, NULL);
	if (!writeResult || bytesWritten != sizeof(int))
	{
		CloseHandle(hFile);
		return false;
	}

	if (tileCount > 0)
	{
		writeResult = WriteFile(hFile, mapData.tiles.data(), sizeof(int) * tileCount, &bytesWritten, NULL);
		if (!writeResult || bytesWritten != sizeof(int) * tileCount)
		{
			CloseHandle(hFile);
			return false;
		}
	}

	CloseHandle(hFile);

	*/
	return true;
}

bool MapManager::CreateEmptyMap(int width, int height)
{

	return true;
}

MapData* MapManager::GetMapData()
{
	return &mapData;
}

void MapManager::SetTile(int x, int y, RoomType tileType, int index)
{
	mapData.tiles[y * mapData.height + x].roomType = tileType;
	mapData.tiles[y * mapData.height + x].tilePos = index;
}