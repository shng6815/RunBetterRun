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

public:
    HRESULT Init();
    void Release();

    void PutSprite(LPCWCH path, FPOINT pos);
    void SortSpritesByDistance();
    const list<Sprite>& GetSprites() const { return sprites; }
};