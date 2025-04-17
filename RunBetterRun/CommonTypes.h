#pragma once
#include "config.h"

typedef struct tagTexture
{
    vector<COLORREF> bmp;
    DWORD bmpWidth;
    DWORD bmpHeight;
} Texture;

typedef struct tagSprite
{
    FPOINT pos;
    double distance;
    Texture* texture;
} Sprite;