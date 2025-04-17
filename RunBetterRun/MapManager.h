#pragma once
#include "Singleton.h"
#include "structs.h"
#include <string>
#include <vector>
#include <map>

#define MAP_COLUME 24
#define MAP_ROW 24

typedef struct tagMapData
{
	int width;
	int height;
	vector<Room> tiles;
	Texture* texture;
} MapData;

class MapManager : public Singleton<MapManager>
{
private:
	MapData mapData;

public:
	HRESULT Init();
	HRESULT Init(LPCWCH filePath);
	void Release();

	bool LoadMap(const LPCWCH filePath);
	bool SaveMap(const LPCWCH filePath);
	bool CreateEmptyMap(int width, int height);

	MapData* GetMapData();

	// 타일 조작
	void SetTile(int x, int y, RoomType tileType, int index);
};