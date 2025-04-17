#include "MapManager.h"

HRESULT MapManager::Init()
{
	// 기본 맵 생성
	//return CreateNewMap(MAP_COLUME, MAP_ROW) ? S_OK : E_FAIL;
    return CreateMazeMap(MAP_COLUME, MAP_ROW) ? S_OK : E_FAIL;
}

HRESULT MapManager::Init(LPCWCH filePath)
{
    if (LoadMap(filePath))
    {
        return S_OK;  
    }

    // 로드 실패 시 기본 맵 생성
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
        return false;  // 파일 열기 실패
    }

    DWORD bytesRead = 0;

    // 맵 너비 읽기
    int width;
    ReadFile(hFile, &width, sizeof(int), &bytesRead, NULL);
    if (bytesRead != sizeof(int))
    {
        CloseHandle(hFile);
        return false;
    }

    // 맵 높이 읽기
    int height;
    ReadFile(hFile, &height, sizeof(int), &bytesRead, NULL);
    if (bytesRead != sizeof(int))
    {
        CloseHandle(hFile);
        return false;
    }

    // 타일 개수 읽기
    int tileCount;
    ReadFile(hFile, &tileCount, sizeof(int), &bytesRead, NULL);
    if (bytesRead != sizeof(int))
    {
        CloseHandle(hFile);
        return false;
    }

    // 데이터 유효성 검사
    if (width <= 0 || height <= 0 || tileCount != width * height)
    {
        CloseHandle(hFile);
        return false;  
    }

    // 맵 데이터 초기화
    mapData.width = width;
    mapData.height = height;
    mapData.tiles.resize(tileCount);

    // 타일 데이터 읽기
    ReadFile(hFile, mapData.tiles.data(), sizeof(Room) * tileCount, &bytesRead, NULL);
    if (bytesRead != sizeof(Room) * tileCount)
    {
        CloseHandle(hFile);
        return false;
    }

    // 플레이어 위치 읽기

    CloseHandle(hFile);
    return true;  
}

bool MapManager::SaveMap(const LPCWCH filePath)
{
    if (mapData.tiles.empty())
    {
        return false; 
    }

    // 파일 생성
    HANDLE hFile = CreateFile(
        filePath,           
        GENERIC_WRITE,     
        0,                  
        NULL,              
        CREATE_ALWAYS,      
        FILE_ATTRIBUTE_NORMAL, 
        NULL                
    );

    // 파일 생성 실패 확인
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return false;  
    }

    DWORD bytesWritten = 0;

    // 맵 너비 저장
    WriteFile(hFile, &mapData.width, sizeof(int), &bytesWritten, NULL);

    // 맵 높이 저장
    WriteFile(hFile, &mapData.height, sizeof(int), &bytesWritten, NULL);

    // 타일 개수 저장
    int tileCount = mapData.tiles.size();
    WriteFile(hFile, &tileCount, sizeof(int), &bytesWritten, NULL);

    // 타일 데이터 저장
    WriteFile(hFile, mapData.tiles.data(), sizeof(Room) * tileCount, &bytesWritten, NULL);

    // 플레이어 위치 저장
    //WriteFile(hFile, &playerPos, sizeof(FPOINT), &bytesWritten, NULL);

    // 파일 핸들 닫기
    CloseHandle(hFile);
    return true; 
}

bool MapManager::CreateNewMap(int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        return false;  
    }

    // 맵 데이터 초기화
    mapData.width = width;
    mapData.height = height;
    mapData.tiles.resize(width * height);

    // 맵 타일 초기화
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            // 맵 가장자리는 벽으로 설정
            if (x == 0 || x == width - 1 || y == 0 || y == height - 1)
            {
                // 벽 타일 (인덱스 4)
                mapData.tiles[y * width + x].roomType = RoomType::WALL;
                mapData.tiles[y * width + x].tilePos = 4;
            }
            else
            {
                // 내부는 바닥으로 설정 (인덱스 10)
                mapData.tiles[y * width + x].roomType = RoomType::FLOOR;
                mapData.tiles[y * width + x].tilePos = 10;
            }
        }
    }

    // 시작 위치 설정 (중앙 근처)
    int startX = width / 2;
    int startY = height / 2;
    mapData.tiles[startY * width + startX].roomType = RoomType::START;
    mapData.tiles[startY * width + startX].tilePos = 3;

    return true;  
}

bool MapManager::CreateMazeMap(int width, int height) {
    // 기본 맵 생성 (모든 타일을 바닥으로)
    CreateNewMap(width, height);

    // 체스판 패턴으로 벽 생성 (간단한 미로 패턴)
    for (int y = 2; y < height - 2; y += 2) {
        for (int x = 2; x < width - 2; x += 2) {
            SetTile(x, y, RoomType::WALL, 4);
        }
    }

    // 세로 벽 배치 
    for (int y = 3; y < height - 2; y += 4) {
        for (int x = 4; x < width - 4; x += 4) {
            SetTile(x, y, RoomType::WALL, 4);
        }
    }

    // 가로 벽 배치 
    for (int x = 3; x < width - 2; x += 4) {
        for (int y = 4; y < height - 4; y += 4) {
            SetTile(x, y, RoomType::WALL, 4);
        }
    }

    // 랜덤으로 벽 뚫기
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
        // 타일 정보 설정
        mapData.tiles[y * mapData.width + x].roomType = tileType;
        mapData.tiles[y * mapData.width + x].tilePos = index;
    }
}