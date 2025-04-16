#pragma once
#include "Singleton.h"
#include "CommonTypes.h"
#include <string>
#include <vector>
#include <map>

typedef struct tagMapData
{
    string name;
    int width;
    int height;
    vector<int> tiles;
    map<LPCWCH, Texture> textureMap;
} MapData;

class MapManager : public Singleton<MapManager>
{
private:
    map<string, MapData> mapDataList;
    string currMapName;
public:
    HRESULT Init();
    void Release();

    bool LoadMap(const string& mapName, const wchar_t* filePath);
    bool SaveMap(const string& mapName, const wchar_t* filePath);
    bool CreateEmptyMap(const string& mapName, int width, int height);
    bool SetCurrentMap(const string& mapName);

    MapData* GetMapData(const string& mapName);
    MapData* GetCurrMapData();
    const int* GetMapTiles(const string& mapName) const;
    string GetCurrMapName() const { return currMapName; }

    // 타일 조작
    bool SetTile(int x, int y, int tileValue);
    bool SetTile(const string& mapName, int x, int y, int tileValue);
    bool FillTiles(int tileValue);
    bool FillTiles(const string& mapName, int tileValue);
    bool SetMapTiles(const vector<int>& newTiles);
    bool SetMapTiles(const string& mapName, const vector<int>& newTiles);

    int GetMapWidth(const string& mapName) const;
    int GetMapHeight(const string& mapName) const;
};