#pragma once
#include "Singleton.h"
#include "structs.h"

#define MAP_COLUME 200
#define MAP_ROW 200

class MapManager: public Singleton<MapManager>
{
private:
	MapData mapData;
	WCHAR texturePath[MAX_PATH];
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
	void SetTexturePath(LPCWCH path) { wcscpy_s(texturePath,path); }
	LPCWCH GetTexturePath() const { return texturePath; }
	MapData* GetMapData(); 
};