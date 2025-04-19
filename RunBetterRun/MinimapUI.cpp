#pragma comment(lib, "Msimg32.lib")

#include "MinimapUI.h"
#include "MapManager.h"
#include "Player.h"

void MinimapUI::DrawMiniMapWithRotation(HDC hdc,int drawSize,float angle)
{
	const int fullSize = static_cast<int>(ceilf(drawSize * 2));
	const float center = fullSize / 2.0f;

	HDC baseDC = CreateCompatibleDC(hdc);
	HBITMAP baseBmp = CreateCompatibleBitmap(hdc,fullSize,fullSize);
	HBITMAP oldBaseBmp = (HBITMAP)SelectObject(baseDC,baseBmp);

	static const COLORREF kMaskColor = RGB(255,0,255);
	HBRUSH bgBrush = CreateSolidBrush(kMaskColor);
	RECT fullRect = {0,0,fullSize,fullSize};
	FillRect(baseDC,&fullRect,bgBrush);

	// 내부 미니맵 그리기
	int offset = (fullSize - drawSize) / 2;

	HDC miniDC = CreateCompatibleDC(hdc);
	HBITMAP miniBmp = CreateCompatibleBitmap(hdc,drawSize,drawSize);
	HBITMAP oldMiniBmp = (HBITMAP)SelectObject(miniDC,miniBmp);

	RECT miniRect = {0,0,drawSize,drawSize};
	FillRect(miniDC,&miniRect,bgBrush); // 재사용
	DrawMiniMapToDC(miniDC,drawSize);

	BitBlt(baseDC,offset,offset,drawSize,drawSize,miniDC,0,0,SRCCOPY);

	// 회전
	HDC rotDC = CreateCompatibleDC(hdc);
	HBITMAP rotBmp = CreateCompatibleBitmap(hdc,fullSize,fullSize);
	HBITMAP oldRotBmp = (HBITMAP)SelectObject(rotDC,rotBmp);

	FillRect(rotDC,&fullRect,bgBrush); // 회전용 배경

	SetGraphicsMode(rotDC,GM_ADVANCED);

	XFORM xform = {
		cosf(angle),sinf(angle),
		-sinf(angle),cosf(angle),
		center * (1 - cosf(angle)) + center * sinf(angle),
		center * (1 - cosf(angle)) - center * sinf(angle)
	};

	XFORM oldXform;
	GetWorldTransform(rotDC,&oldXform);
	SetWorldTransform(rotDC,&xform);

	BitBlt(rotDC,0,0,fullSize,fullSize,baseDC,0,0,SRCCOPY);
	SetWorldTransform(rotDC,&oldXform);

	// 출력
	int cutOffset = (fullSize - drawSize) / 2;
	TransparentBlt(hdc,0,0,drawSize,drawSize,
				   rotDC,cutOffset,cutOffset,drawSize,drawSize,
				   kMaskColor);

	// 정리
	DeleteObject(bgBrush);
	SelectObject(baseDC,oldBaseBmp);
	DeleteObject(baseBmp);
	DeleteDC(baseDC);

	SelectObject(miniDC,oldMiniBmp);
	DeleteObject(miniBmp);
	DeleteDC(miniDC);

	SelectObject(rotDC,oldRotBmp);
	DeleteObject(rotBmp);
	DeleteDC(rotDC);
}

HRESULT MinimapUI::Init(UIType type,FPOINT pos,FPOINT size,INT layer)
{
	isActive = true;

	this->uiType = type;
	this->pos = pos;
	this->size = size;
	this->layer = layer;

	return S_OK;
}

void MinimapUI::Release()
{}

void MinimapUI::Update()
{}

void MinimapUI::Render(HDC hdc)
{
	if(!isActive)
		return;

	int clippedSize = static_cast<int>(ceilf(max(size.x,size.y)));
	int squareSize = static_cast<int>(ceilf(clippedSize * 1.4142f));

	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP memBmp = CreateCompatibleBitmap(hdc,squareSize,squareSize);
	HBITMAP oldBmp = (HBITMAP)SelectObject(memDC,memBmp);

	// 배경 초기화
	static const COLORREF kMaskColor = RGB(255,0,255);
	HBRUSH bgBrush = CreateSolidBrush(kMaskColor);
	RECT fillRect = {0,0,squareSize,squareSize};
	FillRect(memDC,&fillRect,bgBrush);
	DeleteObject(bgBrush);

	// 회전 각도 계산
	auto dir = Player::GetInstance()->GetCameraVerDir();
	float angle = atan2f(dir.y,dir.x) + DEG_TO_RAD(-90);

	DrawMiniMapWithRotation(memDC,squareSize,angle);

	// 위치 계산 후 출력
	int drawX = static_cast<int>(pos.x + (size.x - clippedSize) / 2);
	int drawY = static_cast<int>(pos.y + (size.y - clippedSize) / 2);
	int srcOffset = (squareSize - clippedSize) / 2;

	TransparentBlt(hdc,drawX,drawY,clippedSize,clippedSize,
				   memDC,srcOffset,srcOffset,clippedSize,clippedSize,
				   kMaskColor);

	SelectObject(memDC,oldBmp);
	DeleteObject(memBmp);
	DeleteDC(memDC);
}

void MinimapUI::DrawMiniMapToDC(HDC hdc,int drawSize)
{
	if(!isActive)
		return;

	const int tileSize = 20;
	const int halfTile = tileSize / 2;

	FPOINT pPos = Player::GetInstance()->GetCameraPos();
	auto mapData = MapManager::GetInstance()->GetMapData();

	const float tilesVisibleX = drawSize / static_cast<float>(tileSize);
	const float tilesVisibleY = drawSize / static_cast<float>(tileSize);
	const int radiusX = static_cast<int>(ceilf(tilesVisibleX * 0.75f));
	const int radiusY = static_cast<int>(ceilf(tilesVisibleY * 0.75f));

	HBRUSH floorBrush = CreateSolidBrush(RGB(100,100,100)); // 더 밝은 색상
	HPEN oldPen = (HPEN)SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,floorBrush);

	for(int dy = -radiusY; dy <= radiusY; ++dy)
	{
		for(int dx = -radiusX; dx <= radiusX; ++dx)
		{
			int mapX = static_cast<int>(pPos.x) + dx;
			int mapY = static_cast<int>(pPos.y) + dy;

			if(mapX < 0 || mapX >= mapData->width || mapY < 0 || mapY >= mapData->height)
				continue;

			const auto& tile = mapData->tiles[mapY * mapData->width + mapX];

			if(tile.roomType == RoomType::FLOOR)
			{
				float drawX = (dx - fmodf(pPos.x,1.0f)) * tileSize + drawSize / 2.0f;
				float drawY = (-dy + fmodf(pPos.y,1.0f) - 1) * tileSize + drawSize / 2.0f;

				RECT rect = {
					static_cast<int>(drawX),
					static_cast<int>(drawY),
					static_cast<int>(drawX + tileSize),
					static_cast<int>(drawY + tileSize)
				};

				FillRect(hdc,&rect,floorBrush);
			}
		}
	}

	DeleteObject(floorBrush);
	SelectObject(hdc,oldPen);

	// 플레이어 아이콘
	HBRUSH playerBrush = CreateSolidBrush(RGB(255,50,50)); // 눈에 띄는 색상
	SelectObject(hdc,playerBrush);

	Ellipse(hdc,
		static_cast<int>(drawSize / 2.0f - halfTile/1.5f),
		static_cast<int>(drawSize / 2.0f - halfTile/1.5f),
		static_cast<int>(drawSize / 2.0f + halfTile/1.5f),
		static_cast<int>(drawSize / 2.0f + halfTile/1.5f));

	DeleteObject(playerBrush);
}