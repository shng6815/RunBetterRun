#include "MapManager.h"
#include "TextureManager.h"
#include <fstream>

HRESULT MapManager::Init()
{
	mapData.height = 24;
	mapData.width = 24;
	mapData.tiles.resize(mapData.height * mapData.width);
	mapData.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/horrorMapTiles.bmp"));
	if (!mapData.texture)
		return E_FAIL;
    mapData.textureTileSize = 32;
    mapData.textureTileRowSize = 20;
    mapData.textureTileColumnSize = 9;
	// �⺻ �� ����
	//return CreateNewMap(MAP_COLUME, MAP_ROW) ? S_OK : E_FAIL;
    return CreateMazeMap(MAP_COLUME, MAP_ROW) ? S_OK : E_FAIL;
}

HRESULT MapManager::Init(LPCWCH filePath)
{
    if (LoadMap(filePath))
    {
        return S_OK;  
    }

    // �ε� ���� �� �⺻ �� ����
    //return CreateNewMap(MAP_COLUME, MAP_ROW) ? S_OK : E_FAIL;
    return CreateMazeMap(MAP_COLUME, MAP_ROW) ? S_OK : E_FAIL;
}

void MapManager::Release()
{
	mapData.tiles.clear();
}

bool MapManager::LoadMap(const LPCWCH filePath)
{
    HANDLE hFile = CreateFile(
        filePath,           
        GENERIC_READ,       
        0,                  
        NULL,               
        OPEN_EXISTING,     
        FILE_ATTRIBUTE_NORMAL, 
        NULL                
    );

    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;  // ���� ���� ����
    }

    DWORD bytesRead = 0;

    // �� �ʺ� �б�
    int width;
    ReadFile(hFile, &width, sizeof(int), &bytesRead, NULL);
    if (bytesRead != sizeof(int))
    {
        CloseHandle(hFile);
        return false;
    }

    // �� ���� �б�
    int height;
    ReadFile(hFile, &height, sizeof(int), &bytesRead, NULL);
    if (bytesRead != sizeof(int))
    {
        CloseHandle(hFile);
        return false;
    }

    // Ÿ�� ���� �б�
    int tileCount;
    ReadFile(hFile, &tileCount, sizeof(int), &bytesRead, NULL);
    if (bytesRead != sizeof(int))
    {
        CloseHandle(hFile);
        return false;
    }

    // ������ ��ȿ�� �˻�
    if (width <= 0 || height <= 0 || tileCount != width * height)
    {
        CloseHandle(hFile);
        return false;  
    }

    // �� ������ �ʱ�ȭ
    mapData.width = width;
    mapData.height = height;
    mapData.tiles.resize(tileCount);

    // Ÿ�� ������ �б�
    ReadFile(hFile, mapData.tiles.data(), sizeof(Room) * tileCount, &bytesRead, NULL);
    if (bytesRead != sizeof(Room) * tileCount)
    {
        CloseHandle(hFile);
        return false;
    }

    // �÷��̾� ��ġ �б�

    CloseHandle(hFile);
    return true;  
}

bool MapManager::SaveMap(const LPCWCH filePath)
{
    if (mapData.tiles.empty())
    {
        return false; 
    }

    // ���� ����
    HANDLE hFile = CreateFile(
        filePath,           
        GENERIC_WRITE,     
        0,                  
        NULL,              
        CREATE_ALWAYS,      
        FILE_ATTRIBUTE_NORMAL, 
        NULL                
    );

    // ���� ���� ���� Ȯ��
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;  
    }

    DWORD bytesWritten = 0;

    // �� �ʺ� ����
    WriteFile(hFile, &mapData.width, sizeof(int), &bytesWritten, NULL);

    // �� ���� ����
    WriteFile(hFile, &mapData.height, sizeof(int), &bytesWritten, NULL);

    // Ÿ�� ���� ����
    int tileCount = mapData.tiles.size();
    WriteFile(hFile, &tileCount, sizeof(int), &bytesWritten, NULL);

    // Ÿ�� ������ ����
    WriteFile(hFile, mapData.tiles.data(), sizeof(Room) * tileCount, &bytesWritten, NULL);

    // �÷��̾� ��ġ ����
    //WriteFile(hFile, &playerPos, sizeof(FPOINT), &bytesWritten, NULL);

    // ���� �ڵ� �ݱ�
    CloseHandle(hFile);
    return true; 
}

bool MapManager::CreateNewMap(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return false;  
    }

    // �� ������ �ʱ�ȭ
    mapData.width = width;
    mapData.height = height;
    mapData.tiles.resize(width * height);

    // �� Ÿ�� �ʱ�ȭ
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // �� �����ڸ��� ������ ����
            if (x == 0 || x == width - 1 || y == 0 || y == height - 1)
            {
                // �� Ÿ�� (�ε��� 4)
                mapData.tiles[y * width + x].roomType = RoomType::WALL;
                mapData.tiles[y * width + x].tilePos = 4;
            }
            else
            {
                // ���δ� �ٴ����� ���� (�ε��� 10)
                mapData.tiles[y * width + x].roomType = RoomType::FLOOR;
                mapData.tiles[y * width + x].tilePos = 10;
            }
        }
    }

    // ���� ��ġ ���� (�߾� ��ó)
    int startX = width / 2;
    int startY = height / 2;
    mapData.tiles[startY * width + startX].roomType = RoomType::START;
    mapData.tiles[startY * width + startX].tilePos = 3;

    return true;  
}

bool MapManager::CreateMazeMap(int width, int height) {
    // �⺻ �� ���� (��� Ÿ���� �ٴ�����)
    CreateNewMap(width, height);

    // ü���� �������� �� ���� (������ �̷� ����)
    for (int y = 2; y < height - 2; y += 2) {
        for (int x = 2; x < width - 2; x += 2) {
            SetTile(x, y, RoomType::WALL, 4);
        }
    }

    // ���� �� ��ġ 
    for (int y = 3; y < height - 2; y += 4) {
        for (int x = 4; x < width - 4; x += 4) {
            SetTile(x, y, RoomType::WALL, 4);
        }
    }

    // ���� �� ��ġ 
    for (int x = 3; x < width - 2; x += 4) {
        for (int y = 4; y < height - 4; y += 4) {
            SetTile(x, y, RoomType::WALL, 4);
        }
    }

    // �������� �� �ձ�
    srand(static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < width * height / 10; i++) {
        int x = 2 + rand() % (width - 4);
        int y = 2 + rand() % (height - 4);
        SetTile(x, y, RoomType::FLOOR, 10);
    }

    return true;
}

MapData* MapManager::GetMapData()
{
	return &mapData;
}

void MapManager::SetTile(int x, int y, RoomType tileType, int index)
{
    if (x >= 0 && x < mapData.width && y >= 0 && y < mapData.height)
    {
        // Ÿ�� ���� ����
        mapData.tiles[y * mapData.width + x].roomType = tileType;
        mapData.tiles[y * mapData.width + x].tilePos = index;
    }
}