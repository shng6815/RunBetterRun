#pragma comment(lib, "Msimg32.lib")
#include "PhoneUI.h"
#include "MapManager.h"
#include "Player.h"

HRESULT PhoneUI::Init(UIType type, FPOINT pos, FPOINT size, INT layer)
{
	isActive = true;

	this->uiType = type;
	this->pos = pos;
	this->size = size;
	this->layer = layer;
    return E_NOTIMPL;
}

void PhoneUI::Release()
{
}

void PhoneUI::Update()
{
}

void PhoneUI::Render(HDC hdc)
{
    // 1. 미니맵을 그릴 메모리 DC와 비트맵 생성
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP hBmp = CreateCompatibleBitmap(hdc, size.x, size.y);
    HBITMAP oldBmp = (HBITMAP)SelectObject(memDC, hBmp);


    HBRUSH bgBrush = CreateSolidBrush(RGB(255, 0, 255));
    RECT rect = { 0, 0, size.x, size.y };
    FillRect(memDC, &rect, bgBrush);
    DeleteObject(bgBrush);

    // 2. 미니맵을 그리기
    DrawMiniMapToDC(memDC);

	// 3. 메모리 DC를 원본 DC에 복사
    auto dir = Player::GetInstance()->GetCameraVerDir();
    float angle = atan2f(dir.y, dir.x); // 라디안 단위
    GDITransformRotate(hdc, memDC, DEG_TO_RAD(-90) + angle);

    // 4. 정리
    SelectObject(memDC, oldBmp);
    DeleteObject(hBmp);
    DeleteDC(memDC);
}

void PhoneUI::DrawMiniMapToDC(HDC hdc)
{
    if (!isActive)
        return;

    int tileSize = 15;
    int halfTile = tileSize / 2;

    FPOINT pPos = Player::GetInstance()->GetCameraPos();
    auto mapData = MapManager::GetInstance()->GetMapData();

    // 화면 크기 기준으로 그릴 radius 계산
    float tilesVisibleX = size.x / (float)tileSize;
    float tilesVisibleY = size.y / (float)tileSize;

    int radiusX = static_cast<int>(ceil(tilesVisibleX / 2.0f));
    int radiusY = static_cast<int>(ceil(tilesVisibleY / 2.0f));

    HBRUSH brush = CreateSolidBrush(RGB(0, 0, 0));

    for (int dy = -radiusY; dy <= radiusY; ++dy)
    {
        for (int dx = -radiusX; dx <= radiusX; ++dx)
        {
            int mapX = static_cast<int>(pPos.x) + dx;
            int mapY = static_cast<int>(pPos.y) + dy;

            if (mapX < 0 || mapX >= mapData->width || mapY < 0 || mapY >= mapData->height)
                continue;

            const auto& tile = mapData->tiles[mapY * mapData->width + mapX];

            if (tile.roomType == RoomType::FLOOR)
            {
                float drawX = (dx - (pPos.x - static_cast<int>(pPos.x))) * tileSize + size.x / 2.0f;
                float drawY = (-dy + (pPos.y - static_cast<int>(pPos.y))-1) * tileSize + size.y / 2.0f;

                RECT rect = {
                    static_cast<int>(drawX),
                    static_cast<int>(drawY),
                    static_cast<int>(drawX + tileSize),
                    static_cast<int>(drawY + tileSize)
                };

                FillRect(hdc, &rect, brush);
            }
        }
    }

    DeleteObject(brush);

    // 플레이어 중심 그리기 (항상 중앙)
    float playerDrawX = size.x / 2.0f;
    float playerDrawY = size.y / 2.0f;

    Ellipse(hdc,
        static_cast<int>(playerDrawX - halfTile),
        static_cast<int>(playerDrawY - halfTile),
        static_cast<int>(playerDrawX + halfTile),
        static_cast<int>(playerDrawY + halfTile));
}

void PhoneUI::GDITransformRotate(HDC hdc, HDC memDC, float angle)
{
    XFORM oldXform;
    GetWorldTransform(hdc, &oldXform);

    SetGraphicsMode(hdc, GM_ADVANCED);

    // 회전 중심
    float centerX = pos.x + size.x / 2.0f;
    float centerY = pos.y + size.y / 2.0f;

    // 회전 변환 적용: 이동(-중심) → 회전 → 이동(+중심)
    XFORM xform;

    float cosA = cos(angle);
    float sinA = sin(angle);

    xform.eM11 = cosA;
    xform.eM12 = sinA;
    xform.eM21 = -sinA;
    xform.eM22 = cosA;

    // 회전 중심 보정
    xform.eDx = centerX * (1 - cosA) + centerY * sinA;
    xform.eDy = centerY * (1 - cosA) - centerX * sinA;

    SetWorldTransform(hdc, &xform);

    // 중심 기준으로 BitBlt 위치 조정
    int drawX = static_cast<int>(centerX - size.x / 2);
    int drawY = static_cast<int>(centerY - size.y / 2);

    TransparentBlt(hdc, drawX, drawY, size.x, size.y, memDC, 0, 0, size.x, size.y, RGB(255, 0, 255));

    SetWorldTransform(hdc, &oldXform);
}