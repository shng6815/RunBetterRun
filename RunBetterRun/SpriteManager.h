#pragma once
#include "CommonTypes.h"
#include "Singleton.h"
#include <list>
#include <map>

class SpriteManager : public Singleton<SpriteManager>
{
private:
    std::map<LPCWCH, Texture> spritesTextureData;
    std::list<Sprite> sprites;
    Texture mapTileTexture;
    FPOINT playerPos;
    Texture	mapTile;

public:
    HRESULT Init();
    void Release();

    void PutSprite(LPCWCH path, FPOINT pos);
    void ClearSprites();
    void SortSpritesByDistance();
    void UpdatePlayerPosition(FPOINT pos) { playerPos = pos; }

    HRESULT LoadTexture(LPCWCH path, Texture& outTexture);
    HRESULT LoadMapTileTexture(LPCWCH path);

    // 접근자 함수
    const std::list<Sprite>& GetSprites() const { return sprites; }
    Texture* GetMapTileTexture() { return &mapTileTexture; }
    Texture* GetTexture(LPCWCH path);
};