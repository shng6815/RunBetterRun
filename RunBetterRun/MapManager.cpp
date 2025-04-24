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
		return DataManager::GetInstance()->GetMapData(mapData);
	}
	return false;
}

bool MapManager::SaveMap(const LPCWCH filePath) {
	// 현재 맵 데이터를 DataManager에 설정 후 저장
	DataManager::GetInstance()->ClearAllData();
	DataManager::GetInstance()->SetMapData(mapData.tiles,mapData.width,mapData.height);

	// 텍스처 정보 설정
	DataManager::GetInstance()->SetTextureInfo(L"Image/tiles.bmp",
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
	mapData.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/tiles32x32.bmp"));
	if(!mapData.texture)
		return E_FAIL;
	mapData.textureTileSize = 128;
	mapData.textureTileRowSize = 7;
	mapData.textureTileColumnSize = 4;

	return S_OK;
}

MapData* MapManager::GetMapData()
{
	return &mapData;
}