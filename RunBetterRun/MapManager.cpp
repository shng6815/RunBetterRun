#include "MapManager.h"
#include "TextureManager.h"
#include "DataManager.h"  

HRESULT MapManager::Init()
{
	mapData.height = 100;
	mapData.width = 100;
	mapData.tiles.resize(mapData.height * mapData.width);

	InitializeTexture();
	return CreateMazeMap(MAP_COLUME,MAP_ROW) ? S_OK : E_FAIL;
}

HRESULT MapManager::Init(LPCWCH filePath)
{
	if(LoadMap(filePath))
	{
		return S_OK;
	}
	return CreateMazeMap(MAP_COLUME,MAP_ROW) ? S_OK : E_FAIL;
}

void MapManager::Release()
{
	mapData.tiles.clear();
}

bool MapManager::LoadMap(const LPCWCH filePath)
{
	if(DataManager::GetInstance()->LoadMapFile(filePath))
	{
		if(DataManager::GetInstance()->GetMapData(mapData))
		{
			return true;
		}
	}

	// 윗부분 실패하면 직접 파일 읽기
	HANDLE file = CreateFile(filePath,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(file == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	DWORD bytesRead = 0;

	// 맵 크기 읽기
	int width = 0,height = 0;
	if(!ReadFile(file,&width,sizeof(int),&bytesRead,NULL) || bytesRead != sizeof(int) ||
		!ReadFile(file,&height,sizeof(int),&bytesRead,NULL) || bytesRead != sizeof(int))
	{
		CloseHandle(file);
		return false;
	}

	mapData.width = width;
	mapData.height = height;
	mapData.tiles.resize(width * height);

	// 타일 데이터 읽기
	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			Room tile;
			if(!ReadFile(file,&tile,sizeof(Room),&bytesRead,NULL) || bytesRead != sizeof(Room))
			{
				CloseHandle(file);
				return false;
			}
			mapData.tiles[y * width + x] = tile;
		}
	}

	CloseHandle(file);

	return InitializeTexture() == S_OK;
}

bool MapManager::SaveMap(const LPCWCH filePath)
{
	DataManager::GetInstance()->ClearAllData();
	DataManager::GetInstance()->SetMapData(mapData.tiles,mapData.width,mapData.height);
	DataManager::GetInstance()->SetTextureInfo(L"Image/horrorMapTiles.bmp",
											 mapData.textureTileSize,
											 mapData.textureTileRowSize,
											 mapData.textureTileColumnSize);

	return DataManager::GetInstance()->SaveMapFile(filePath);
}

bool MapManager::CreateNewMap(int width,int height)
{
	if(width <= 0 || height <= 0)
	{
		return false;
	}

	mapData.width = width;
	mapData.height = height;
	mapData.tiles.resize(width * height);

	for(int y = 0; y < height; y++)
	{
		for(int x = 0; x < width; x++)
		{
			if(x == 0 || x == width - 1 || y == 0 || y == height - 1)
			{
				mapData.tiles[y * width + x].roomType = RoomType::WALL;
				mapData.tiles[y * width + x].tilePos = 4;
			} else
			{
				mapData.tiles[y * width + x].roomType = RoomType::FLOOR;
				mapData.tiles[y * width + x].tilePos = 10;
			}
		}
	}

	int startX = width / 2;
	int startY = height / 2;
	mapData.tiles[startY * width + startX].roomType = RoomType::START;
	mapData.tiles[startY * width + startX].tilePos = 3;

	return true;
}

bool MapManager::CreateMazeMap(int width,int height)
{
	CreateNewMap(width,height);

	for(int y = 2; y < height - 2; y += 2)
	{
		for(int x = 2; x < width - 2; x += 2)
		{
			SetTile(x,y,RoomType::WALL,4);
		}
	}

	for(int y = 3; y < height - 2; y += 4)
	{
		for(int x = 4; x < width - 4; x += 4)
		{
			SetTile(x,y,RoomType::WALL,4);
		}
	}

	for(int x = 3; x < width - 2; x += 4)
	{
		for(int y = 4; y < height - 4; y += 4)
		{
			SetTile(x,y,RoomType::WALL,4);
		}
	}

	srand(static_cast<unsigned int>(time(NULL)));
	for(int i = 0; i < width * height / 10; i++)
	{
		int x = 2 + rand() % (width - 4);
		int y = 2 + rand() % (height - 4);
		SetTile(x,y,RoomType::FLOOR,10);
	}

	return true;
}

void MapManager::SetTile(int x,int y,RoomType tileType,int index)
{
	if(x >= 0 && x < mapData.width && y >= 0 && y < mapData.height)
	{
		mapData.tiles[y * mapData.width + x].roomType = tileType;
		mapData.tiles[y * mapData.width + x].tilePos = index;
	}
}

HRESULT MapManager::InitializeTexture()
{
	mapData.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/horrorMapTiles.bmp"));
	if(!mapData.texture)
		return E_FAIL;
	mapData.textureTileSize = 128;
	mapData.textureTileRowSize = 11;
	mapData.textureTileColumnSize = 11;

	return S_OK;
}

MapData* MapManager::GetMapData()
{
	return &mapData;
}