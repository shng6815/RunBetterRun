#include "MapManager.h"
#include <fstream>

HRESULT MapManager::Init()
{
    mapDataList.clear();
    currMapName = "";
    return S_OK;
}

void MapManager::Release()
{
    for (auto& mapPair : mapDataList)
    {
        mapPair.second.textureMap.clear();
        mapPair.second.tiles.clear();
    }

    mapDataList.clear();
    currMapName = "";
}

bool MapManager::LoadMap(const string& mapName, const wchar_t* filePath)
{
    HANDLE hFile = CreateFile(
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
    CloseHandle(hFile);

    mapDataList[mapName] = newMap;
    currMapName = mapName;

    return true;
}

bool MapManager::SaveMap(const string& mapName, const wchar_t* filePath)
{
    auto it = mapDataList.find(mapName);
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

    return true;
}

bool MapManager::CreateEmptyMap(const string& mapName, int width, int height)
{
    if (mapDataList.find(mapName) != mapDataList.end())
    {
        mapDataList.erase(mapName);
    }

    MapData newMap;
    newMap.name = mapName;
    newMap.width = width;
    newMap.height = height;
    newMap.tiles.resize(width * height, 0);

    mapDataList[mapName] = newMap;
    currMapName = mapName;

    return true;
}

bool MapManager::SetCurrentMap(const string& mapName)
{
    auto it = mapDataList.find(mapName);
    if (it == mapDataList.end())
    {
        return false;
    }

    currMapName = mapName;
    return true;
}

MapData* MapManager::GetMapData(const string& mapName)
{
    auto it = mapDataList.find(mapName);
    if (it == mapDataList.end())
    {
        return nullptr;
    }

    return &(it->second);
}

MapData* MapManager::GetCurrMapData()
{
    if (currMapName.empty())
    {
        return nullptr;
    }

    return GetMapData(currMapName);
}

const int* MapManager::GetMapTiles(const string& mapName) const
{
    auto it = mapDataList.find(mapName);
    if (it == mapDataList.end() || it->second.tiles.empty())
    {
        return nullptr;
    }

    return it->second.tiles.data();
}

bool MapManager::SetTile(int x, int y, int tileValue)
{
    if (currMapName.empty())
        return false;

    return SetTile(currMapName, x, y, tileValue);
}

bool MapManager::SetTile(const string& mapName, int x, int y, int tileValue)
{
    auto it = mapDataList.find(mapName);
    if (it == mapDataList.end())
        return false;

    MapData& mapData = it->second;

    if (x < 0 || x >= mapData.width || y < 0 || y >= mapData.height)
        return false;

    mapData.tiles[y * mapData.width + x] = tileValue;
    return true;
}

bool MapManager::FillTiles(int tileValue)
{
    if (currMapName.empty())
        return false;

    return FillTiles(currMapName, tileValue);
}

bool MapManager::FillTiles(const string& mapName, int tileValue)
{
    auto it = mapDataList.find(mapName);
    if (it == mapDataList.end())
        return false;

    MapData& mapData = it->second;

    std::fill(mapData.tiles.begin(), mapData.tiles.end(), tileValue);
    return true;
}

bool MapManager::SetMapTiles(const vector<int>& newTiles)
{
    if (currMapName.empty())
        return false;

    return SetMapTiles(currMapName, newTiles);
}

bool MapManager::SetMapTiles(const string& mapName, const vector<int>& newTiles)
{
    auto it = mapDataList.find(mapName);
    if (it == mapDataList.end())
        return false;

    MapData& mapData = it->second;

    if (newTiles.size() != static_cast<size_t>(mapData.width * mapData.height))
        return false;

    mapData.tiles = newTiles;
    return true;
}

int MapManager::GetMapWidth(const string& mapName) const
{
    auto it = mapDataList.find(mapName);
    if (it == mapDataList.end())
    {
        return 0;
    }

    return it->second.width;
}

int MapManager::GetMapHeight(const string& mapName) const
{
    auto it = mapDataList.find(mapName);
    if (it == mapDataList.end())
    {
        return 0;
    }

    return it->second.height;
}