#include "TextureManager.h"
#include <fstream>

HRESULT TextureManager::LoadTexture(LPCWCH path, Texture& texture)
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

    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;

    file.read(reinterpret_cast<LPCH>(&fileHeader), sizeof(fileHeader));
    file.read(reinterpret_cast<LPCH>(&infoHeader), sizeof(infoHeader));

    if (fileHeader.bfType != 0x4D42)
    {
        wstring error = TEXT("Not a valid BMP file: ");
        error += path;
        MessageBox(g_hWnd,
            error.c_str(), TEXT("Warning"), MB_OK);
        file.close();
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
        file.close();
        return E_FAIL;
    }

    DWORD bmpRowSize = (texture.bmpWidth * 3 + 3) & ~3;
    vector<BYTE> bmpData(bmpRowSize * texture.bmpHeight);

    file.seekg(fileHeader.bfOffBits, std::ios::beg);
    file.read(reinterpret_cast<LPCH>(bmpData.data()), bmpData.size());
    file.close();

    texture.bmp.resize(texture.bmpWidth * texture.bmpHeight);
    for (DWORD i = 0; i < texture.bmpWidth * texture.bmpHeight; ++i)
    {
        DWORD index = (texture.bmpHeight - (i / texture.bmpWidth) - 1) * bmpRowSize + (i % texture.bmpWidth) * 3;
        texture.bmp[i] = RGB(bmpData[index], bmpData[index + 1], bmpData[index + 2]);
    }
    return S_OK;
}


void TextureManager::Release(void)
{
    if (!source.empty())
        source.clear();
}

Texture* TextureManager::GetTexture(LPCWCH path)
{
    auto iter = source.find(path);
    if (iter == source.end())
    {
        source.insert(make_pair(path, Texture()));
        iter = source.find(path);
        if (FAILED(LoadTexture(path, iter->second)))
            return nullptr;
    }
    return &iter->second;
}
