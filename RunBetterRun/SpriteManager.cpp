#include "SpriteManager.h"
#include <fstream>

HRESULT SpriteManager::Init()
{
    static bool isInitialized = false;
    if (isInitialized)
        return S_OK;

    spritesTextureData.clear();
    sprites.clear();
    playerPos = { 0, 0 };

    isInitialized = true;

    return S_OK;
}

void SpriteManager::Release()
{
    for (auto& pair : spritesTextureData)
    {
        pair.second.bmp.clear();
    }
    spritesTextureData.clear();
    sprites.clear();

    if (!mapTileTexture.bmp.empty())
        mapTileTexture.bmp.clear();
}

void SpriteManager::PutSprite(LPCWCH path, FPOINT pos)
{
    auto it = spritesTextureData.find(path);
    if (it != spritesTextureData.end())
        sprites.push_back(Sprite{ pos, 0, &it->second });
    else
    {
        spritesTextureData.insert(make_pair(path, Texture()));
        Texture& sprite = spritesTextureData[path];
        if (FAILED(LoadTexture(path, sprite)))
            spritesTextureData.erase(path);
        else
            sprites.push_back(Sprite{ pos, 0, &sprite });
    }
}

void SpriteManager::ClearSprites()
{
    sprites.clear();
}

void SpriteManager::SortSpritesByDistance()
{
    for (auto& sprite : sprites)
    {
        sprite.distance = sqrtf(
            powf(playerPos.x - sprite.pos.x, 2) +
            powf(playerPos.y - sprite.pos.y, 2)
        );
    }

    sprites.sort([](const Sprite& a, const Sprite& b) -> BOOL {
        return a.distance > b.distance;
        });
}


HRESULT SpriteManager::LoadTexture(LPCWCH path, Texture& texture)
{
    if (!texture.bmp.empty())
        texture.bmp.clear();

    std::ifstream file(path, std::ios::binary);

    if (!file.is_open())
    {
        wstring error = TEXT("Error opening file: ");
        error += path;
        MessageBox(g_hWnd,
            error.c_str(), TEXT("Warning"), MB_OK);
        return E_FAIL;
    }

    // 비트맵 헤더 읽기
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    file.read(reinterpret_cast<LPCH>(&fileHeader), sizeof(fileHeader));
    file.read(reinterpret_cast<LPCH>(&infoHeader), sizeof(infoHeader));

    // 파일 형식 검증
    if (fileHeader.bfType != 0x4D42)
    {
        wstring error = TEXT("Not a valid BMP file: ");
        error += path;
        MessageBox(g_hWnd,
            error.c_str(), TEXT("Warning"), MB_OK);
        return E_FAIL;
    }

    texture.bmpWidth = infoHeader.biWidth;
    texture.bmpHeight = infoHeader.biHeight;

    if (infoHeader.biBitCount != 24)
    {
        wstring error = TEXT("Only 24-bit BMP files are supported: ");
        error += path;
        MessageBox(g_hWnd,
            error.c_str(), TEXT("Warning"), MB_OK);
        return E_FAIL;
    }

    DWORD bmpRowSize = (texture.bmpWidth * 3 + 3) & ~3; 
    vector<BYTE> bmpData(bmpRowSize * texture.bmpHeight);

    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    file.read(reinterpret_cast<LPCH>(bmpData.data()), bmpData.size());
    file.close();

    // 비트맵을 COLORREF 형식으로 변환
    texture.bmp.resize(texture.bmpWidth * texture.bmpHeight);
    for (DWORD i = 0; i < texture.bmpWidth * texture.bmpHeight; ++i)
    {
        DWORD index = (texture.bmpHeight - (i / texture.bmpWidth) - 1) * bmpRowSize + (i % texture.bmpWidth) * 3;
        texture.bmp[i] = RGB(bmpData[index], bmpData[index + 1], bmpData[index + 2]);
    }
    return S_OK;
}

HRESULT SpriteManager::LoadMapTileTexture(LPCWCH path)
{
    return LoadTexture(path, mapTileTexture);
}

Texture* SpriteManager::GetTexture(LPCWCH path)
{
    auto it = spritesTextureData.find(path);
    if (it != spritesTextureData.end())
        return &it->second;
    return nullptr;
}