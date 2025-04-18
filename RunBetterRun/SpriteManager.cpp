#include "SpriteManager.h"
#include "TextureManager.h"
#include "Player.h"
#include <fstream>
#include <map>

HRESULT SpriteManager::Init()
{
    static bool isInitialized = false;
    if (isInitialized)
        return S_OK;
    sprites.clear();
    playerPos = { 0, 0 };
    isInitialized = true;
    return S_OK;
}

void SpriteManager::Release()
{
    sprites.clear();
}

void SpriteManager::PutSprite(LPCWCH path, FPOINT pos)
{
    Texture* texture = TextureManager::GetInstance()->GetTexture(path);
    if (texture)
        sprites.push_back(Sprite{ pos, 0, texture });
}

void SpriteManager::ClearMonsterSprites(LPCWCH path)
{
    Texture* texture = TextureManager::GetInstance()->GetTexture(path);
    if (!texture) return;

    // 해당 텍스처를 가진 모든 스프라이트 제거
    auto it = sprites.begin();
    while (it != sprites.end()) {
        if (it->texture == texture) {
            it = sprites.erase(it);
        }
        else {
            ++it;
        }
    }
}

void SpriteManager::SortSpritesByDistance()
{
    playerPos = Player::GetInstance()->GetCameraPos();
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

void SpriteManager::UpdateMonsterPosition(LPCWCH path, FPOINT newPos)
{

    auto it = spritesTextureData.find(path);
    if (it == spritesTextureData.end()) {

        PutSprite(path, newPos);
        return;
    }


    Texture* targetTexture = &it->second;


    bool found = false;
    for (auto& sprite : sprites) {
        if (sprite.texture == targetTexture) {

            sprite.pos = newPos;
            found = true;
            
            break;
        }
    }


    if (!found) {
        sprites.push_back(Sprite{ newPos, 0, targetTexture });
    }
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

    // ��Ʈ�� ��� �б�
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    file.read(reinterpret_cast<LPCH>(&fileHeader), sizeof(fileHeader));
    file.read(reinterpret_cast<LPCH>(&infoHeader), sizeof(infoHeader));

    // ���� ���� ����
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

    // ��Ʈ���� COLORREF �������� ��ȯ
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