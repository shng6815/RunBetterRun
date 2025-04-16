#pragma once
#include "Singleton.h"
#include "CommonTypes.h"
#include <string>
#include <vector>
#include <map>

// 맵 정보를 저장하는 구조체
typedef struct tagMapData
{
    string name;           
    int width;                 
    int height;                
    vector<int> tiles;     
    map<LPCWCH, Texture> textureMap; // 맵 텍스처 맵
} MapData;

class MapManager : public Singleton<MapManager>
{
private:
    map<string, MapData> mapDataList;  // 다양한 맵을 저장
    string currentMapName;                 // 현재 활성화된 맵 이름

public:
    HRESULT Init();
    void Release();

    // 맵 로드/저장 함수
    bool LoadMapFromFile(const std::string& mapName, const wchar_t* filePath);
    bool SaveMapToFile(const std::string& mapName, const wchar_t* filePath);

    // 맵 생성 함수
    bool CreateEmptyMap(const std::string& mapName, int width, int height);
    bool SetCurrentMap(const std::string& mapName);

    // 맵 접근 함수
    const MapData* GetMapData(const std::string& mapName) const;
    const MapData* GetCurrentMapData() const;
    int* GetMapTiles(const std::string& mapName); // RayCasting과의 호환성 위한 함수
    std::string GetCurrentMapName() const { return currentMapName; }

    // 맵 속성 접근 함수
    int GetMapWidth(const std::string& mapName) const;
    int GetMapHeight(const std::string& mapName) const;
};