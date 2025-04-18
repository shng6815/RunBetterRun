#pragma once
#include "structs.h"
#include "Singleton.h"
#include <list>
#include <map>


class SpriteManager : public Singleton<SpriteManager>
{
private:
    list<Sprite> sprites;
    FPOINT playerPos;
    FPOINT monsterPos;

public:
    HRESULT Init();
    void Release();

    void PutSprite(LPCWCH path, FPOINT pos);
    void SortSpritesByDistance();

    void UpdatePlayerPosition(FPOINT pos) { playerPos = pos; }
    void UpdateMonsterPosition(LPCWCH path, FPOINT newPos);

    HRESULT LoadTexture(LPCWCH path, Texture& outTexture);
    HRESULT LoadMapTileTexture(LPCWCH path);

    const list<Sprite>& GetSprites() const { return sprites; }
};