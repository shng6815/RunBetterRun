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
	const int squareSize = static_cast<int>(min(size.x, size.y));

	// 미니맵을 그릴 메모리 DC
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP memBmp = CreateCompatibleBitmap(hdc,squareSize,squareSize);
	HBITMAP oldMemBmp = (HBITMAP)SelectObject(memDC,memBmp);

	// 배경 투명색으로 채우기
	HBRUSH bgBrush = CreateSolidBrush(RGB(255,0,255));
	RECT bgRect = {0,0,squareSize,squareSize};
	FillRect(memDC,&bgRect,bgBrush);
	DeleteObject(bgBrush);

	// 회전 각도 계산
	auto dir = Player::GetInstance()->GetCameraVerDir();
	float angle = atan2f(dir.y,dir.x);
	float rotatedAngle = DEG_TO_RAD(-90) + angle;

	// 회전 적용해서 미니맵 그리기
	DrawMiniMapWithRotation(memDC,squareSize,rotatedAngle);

	// 원본 DC에 정사각형 클리핑 렌더링
	int drawX = static_cast<int>(pos.x + (size.x - squareSize) / 2);
	int drawY = static_cast<int>(pos.y + (size.y - squareSize) / 2);
	TransparentBlt(hdc,drawX,drawY,squareSize,squareSize,memDC,0,0,squareSize,squareSize,RGB(255,0,255));

	// 정리
	SelectObject(memDC,oldMemBmp);
	DeleteObject(memBmp);
	DeleteDC(memDC);
}

void PhoneUI::DrawMiniMapWithRotation(HDC hdc,int size,float angle)
{
	// 회전 중심
	float centerX = size / 2.0f;
	float centerY = size / 2.0f;

	// 회전용 임시 DC
	HDC rotatedDC = CreateCompatibleDC(hdc);
	HBITMAP rotatedBmp = CreateCompatibleBitmap(hdc,size,size);
	HBITMAP oldBmp = (HBITMAP)SelectObject(rotatedDC,rotatedBmp);

	// 배경 채우기
	HBRUSH bgBrush = CreateSolidBrush(RGB(255,0,255));
	RECT rect = {0,0,size,size};
	FillRect(rotatedDC,&rect,bgBrush);
	DeleteObject(bgBrush);

	// 미니맵 내용 그리기 (이전 DrawMiniMapToDC 사용)
	this->size = {(float)size,(float)size};
	DrawMiniMapToDC(rotatedDC);

	// 회전 적용해서 원래 hdc에 그리기
	XFORM xform;
	float cosA = cosf(angle);
	float sinA = sinf(angle);
	xform.eM11 = cosA;
	xform.eM12 = sinA;
	xform.eM21 = -sinA;
	xform.eM22 = cosA;
	xform.eDx = centerX * (1 - cosA) + centerY * sinA;
	xform.eDy = centerY * (1 - cosA) - centerX * sinA;

	SetGraphicsMode(hdc,GM_ADVANCED);
	XFORM oldXform;
	GetWorldTransform(hdc,&oldXform);
	SetWorldTransform(hdc,&xform);

	TransparentBlt(hdc,0,0,size,size,rotatedDC,0,0,size,size,RGB(255,0,255));

	SetWorldTransform(hdc,&oldXform);

	// 정리
	SelectObject(rotatedDC,oldBmp);
	DeleteObject(rotatedBmp);
	DeleteDC(rotatedDC);
}

void PhoneUI::DrawMiniMapToDC(HDC hdc)
{
    if (!isActive)
        return;

    int tileSize = 20;
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