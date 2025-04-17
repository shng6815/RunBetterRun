#include "MapManager.h"

HRESULT MapManager::Init()
{
	// 24x24 크기 기본 맵 생성
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

    // 파일 핸들 닫기
    CloseHandle(hFile);
    return true; 
}

void MapManager::GenerateMaze(int startX, int startY, int width, int height)
{
    // 방문 여부를 저장하는 임시 배열
    vector<vector<bool>> visited(height, vector<bool>(width, false));

    // 방향 정의 (상, 하, 좌, 우)
    int dx[] = { 0, 0, -2, 2 };
    int dy[] = { -2, 2, 0, 0 };

    // 스택을 이용한 깊이 우선 탐색
    stack<pair<int, int>> cellStack;

    // 시작점
    SetTile(startX, startY, RoomType::FLOOR, 10);
    visited[startY][startX] = true;
    cellStack.push(make_pair(startX, startY));

    // 랜덤 시드 설정
    srand(static_cast<unsigned int>(time(NULL)));

    // 미로 생성
    while (!cellStack.empty())
    {
        // 현재 위치
        int x = cellStack.top().first;
        int y = cellStack.top().second;

        // 방향 배열
        int directions[4] = { 0, 1, 2, 3 };
        ShuffleDirections(directions);

        bool foundNext = false;

        // 4방향 시도
        for (int i = 0; i < 4; i++)
        {
            int dir = directions[i];
            int nx = x + dx[dir];
            int ny = y + dy[dir];

            // 맵 경계 및 방문 여부 확인
            if (nx > 0 && nx < width - 1 && ny > 0 && ny < height - 1 && !visited[ny][nx])
            {
                // 중간 벽 제거 (길 만들기)
                SetTile(x + dx[dir] / 2, y + dy[dir] / 2, RoomType::FLOOR, 10);

                // 다음 위치로 이동
                SetTile(nx, ny, RoomType::FLOOR, 10);
                visited[ny][nx] = true;

                // 스택에 추가
                cellStack.push(make_pair(nx, ny));
                foundNext = true;
                break;
            }
        }

        // 더 이상 이동할 수 없으면 스택에서 제거
        if (!foundNext)
        {
            cellStack.pop();
        }
    }
}

void MapManager::ShuffleDirections(int directions[4])
{
    // Fisher-Yates 셔플 알고리즘
    for (int i = 3; i > 0; i--)
    {
        int j = rand() % (i + 1);

        int temp = directions[i];
        directions[i] = directions[j];
        directions[j] = temp;
    }
}

bool MapManager::CreateMazeMap(int width, int height)
{
    // 가로 세로 크기가 홀수여야 미로가 더 잘 생성됨
    if (width % 2 == 0) width++;
    if (height % 2 == 0) height++;

    // 맵 크기는 최소 5x5
    if (width < 5) width = 5;
    if (height < 5) height = 5;

    // 맵 생성 (모든 타일을 벽으로 설정)
    CreateNewMap(width, height);

    // 모든 타일을 벽으로 설정
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            mapData.tiles[y * width + x].roomType = RoomType::WALL;
            mapData.tiles[y * width + x].tilePos = 4;
        }
    }

    // 미로 생성 시작 (1, 1부터 시작)
    GenerateMaze(1, 1, width, height);

    // 시작 위치 설정 (미로 중앙 근처)
    SetTile(width / 2, height / 2, RoomType::START, 3);

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