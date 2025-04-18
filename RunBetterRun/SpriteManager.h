#pragma once
#include "structs.h"
#include "Singleton.h"
#include <list>
#include <map>


class SpriteManager : public Singleton<SpriteManager>
{
private:
    map<LPCWCH, Texture> spritesTextureData;
    list<Sprite> sprites;
    Texture mapTileTexture;
    FPOINT playerPos;
    FPOINT monsterPos;
    map<LPCWCH, Texture> spritesTextureData;
    Texture mapTileTexture;

public:
    HRESULT Init();
    void Release();

    void PutSprite(LPCWCH path, FPOINT pos);
    void ClearMonsterSprites(LPCWCH path);
    void SortSpritesByDistance();
    void UpdatePlayerPosition(FPOINT pos) { playerPos = pos; }
    void UpdateMonsterPosition(LPCWCH path, FPOINT newPos);

    HRESULT LoadTexture(LPCWCH path, Texture& outTexture);
    HRESULT LoadMapTileTexture(LPCWCH path);

    // 접근자 함수
    const list<Sprite>& GetSprites() const { return sprites; }
    Texture* GetTexture(LPCWCH path);
    Texture* GetMapTileTexture() { return &mapTileTexture; }
};