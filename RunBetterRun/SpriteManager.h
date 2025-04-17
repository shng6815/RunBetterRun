#pragma once
#include "structs.h"
#include "Singleton.h"
#include <list>
#include <map>
#include "structs.h"

class SpriteManager : public Singleton<SpriteManager>
{
private:
    map<LPCWCH, Texture> spritesTextureData;
    list<Sprite> sprites;
    Texture mapTileTexture;
    FPOINT playerPos;

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
    const list<Sprite>& GetSprites() const { return sprites; }
    Texture* GetMapTileTexture() { return &mapTileTexture; }
    Texture* GetTexture(LPCWCH path);
};