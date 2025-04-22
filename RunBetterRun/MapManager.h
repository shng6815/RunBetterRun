#pragma once
#include "Singleton.h"
#include "structs.h"

#define MAP_COLUME 80
#define MAP_ROW 80

class MapManager: public Singleton<MapManager>
{
private:
	MapData mapData;

public:
	HRESULT Init();                     // 기본 맵 생성
	HRESULT Init(LPCWCH filePath);      // 파일 경로로 맵 로드
	void Release();

	bool LoadMap(const LPCWCH filePath);
	bool SaveMap(const LPCWCH filePath); 
	bool CreateNewMap(int width,int height);
	bool CreateMazeMap(int width,int height);
	HRESULT InitializeTexture();
	void SetTile(int x,int y,RoomType tileType,int index);

	MapData* GetMapData(); 
};