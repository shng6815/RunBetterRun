#pragma comment(lib, "Msimg32.lib")

#include "MinimapUI.h"
#include "MapManager.h"
#include "Player.h"
#include "ItemManager.h"
#include "Key.h"
#include "TimerManager.h"
#include "MonsterManager.h"
#include "ObstacleManager.h"

HRESULT MinimapUI::Init(UIType type,FPOINT relPos,FPOINT relSize,UIUnit* parent,INT layer)
{
	UIUnit::Init(type,relPos,relSize,parent,layer);

	frameSkip = 8;
	frameCounter = 0;
	lastAngle = 0.0f;
	cachedDC = nullptr;
	cachedBitmap = nullptr;
	oldCachedBitmap = nullptr;
	needsUpdate = true;

	showMonsters = false;

	return S_OK;
}

void MinimapUI::Update()
{
	UIUnit::Update();

	frameCounter++;

	// 플레이어의 현재 방향 각도 계산
	auto dir = Player::GetInstance()->GetCameraVerDir();
	float currentAngle = atan2f(dir.y,dir.x) + DEG_TO_RAD(-90);

	float angleDiff = fabsf(currentAngle - lastAngle);
	if(angleDiff > DEG_TO_RAD(10.0f)) { 
		needsUpdate = true;
	}

	if(frameCounter >= frameSkip || needsUpdate) {
		frameCounter = 0;
		lastAngle = currentAngle;
		needsUpdate = false;

		if(cachedDC) {
			SelectObject(cachedDC,oldCachedBitmap);
			DeleteObject(cachedBitmap);
			DeleteDC(cachedDC);
			cachedDC = nullptr;
		}
	}
}

void MinimapUI::Release()
{
	if(cachedDC) {
		SelectObject(cachedDC,oldCachedBitmap);
		DeleteObject(cachedBitmap);
		DeleteDC(cachedDC);
		cachedDC = nullptr;
	}
}

void MinimapUI::Render(HDC hdc)
{
	if(!isActive)
		return;

	int maxSize = static_cast<int>(ceilf(max(size.x,size.y)));
	int squareSize = static_cast<int>(ceilf(maxSize * 1.4142f)); // 회전을 고려한 대각선 길이

	// 방향 각도 계산
	auto dir = Player::GetInstance()->GetCameraVerDir();
	float angle = atan2f(dir.y,dir.x) + DEG_TO_RAD(-90);

	// 캐싱 시스템 활용
	HDC memDC = CreateCompatibleDC(hdc);
	HBITMAP memBmp = CreateCompatibleBitmap(hdc,squareSize,squareSize);
	HBITMAP oldBmp = (HBITMAP)SelectObject(memDC,memBmp);

	// 배경 초기화
	static const COLORREF kMaskColor = RGB(255,0,255);
	HBRUSH bgBrush = CreateSolidBrush(kMaskColor);
	RECT fillRect = {0,0,squareSize,squareSize};
	FillRect(memDC,&fillRect,bgBrush);
	DeleteObject(bgBrush);

	// frameCounter가 0이거나 캐시가 없으면 새로 계산, 아니면 캐시된 결과 사용
	if(frameCounter == 0 || !cachedDC) {
		// 캐시된 DC 생성 및 초기화
		cachedDC = CreateCompatibleDC(hdc);
		cachedBitmap = CreateCompatibleBitmap(hdc,squareSize,squareSize);
		oldCachedBitmap = (HBITMAP)SelectObject(cachedDC,cachedBitmap);

		// 배경 초기화
		RECT cacheRect = {0,0,squareSize,squareSize};
		FillRect(cachedDC,&cacheRect,bgBrush);

		// 회전된 미니맵 계산하여 캐시에 저장
		DrawMiniMapWithRotation(cachedDC,squareSize,angle);
	}

	// 캐시된 결과를 memDC에 복사
	BitBlt(memDC,0,0,squareSize,squareSize,cachedDC,0,0,SRCCOPY);

	// 위치 계산 후 원하는 사이즈로 출력 (직사각형으로 클리핑)
	int drawX = static_cast<int>(pos.x);
	int drawY = static_cast<int>(pos.y);

	// 중앙 정렬을 위한 오프셋 계산
	int srcX = (squareSize - static_cast<int>(size.x)) / 2;
	int srcY = (squareSize - static_cast<int>(size.y)) / 2;

	// 원하는 직사각형 크기로 클리핑하여 출력
	TransparentBlt(hdc,drawX,drawY,
				  static_cast<int>(size.x),static_cast<int>(size.y),
				  memDC,srcX,srcY,
				  static_cast<int>(size.x),static_cast<int>(size.y),
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

	HBRUSH floorBrush = CreateSolidBrush(RGB(30,30,30)); // 더 밝은 색상
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

			if(tile.roomType != RoomType::WALL)
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

	SelectObject(hdc,oldPen);
	DeleteObject(floorBrush);

	// 아이템(키) 그리기
	HBRUSH keyBrush = CreateSolidBrush(RGB(238,130,238));
	HPEN oldKeyPen = (HPEN)SelectObject(hdc,GetStockObject(NULL_PEN));
	SelectObject(hdc,keyBrush);

	for(const auto& item : ItemManager::GetInstance()->GetItems())
	{
		
		Key* key = dynamic_cast<Key*>(item);
		if(key)
		{
			FPOINT keyPos = key->GetPos();

			// 상대적 위치 계산
			float relX = (keyPos.x - pPos.x) * tileSize + drawSize / 2.0f;
			float relY = (-keyPos.y + pPos.y) * tileSize + drawSize / 2.0f;

			// 미니맵 범위 체크
			if(relX >= 0 && relX < drawSize && relY >= 0 && relY < drawSize)
			{
				// 키 그리기
				int keySize = 6;
				Rectangle(hdc,
					static_cast<int>(relX - keySize),
					static_cast<int>(relY - keySize),
					static_cast<int>(relX + keySize),
					static_cast<int>(relY + keySize));
			}
		}
	}

	SelectObject(hdc,oldKeyPen);
	DeleteObject(keyBrush);

	// 플레이어 아이콘
	HBRUSH playerBrush = CreateSolidBrush(RGB(255,50,50));
	HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
	oldPen = (HPEN)SelectObject(hdc,nullPen);
	SelectObject(hdc,playerBrush);

	Ellipse(hdc,
		static_cast<int>(drawSize / 2.0f - halfTile/1.5f),
		static_cast<int>(drawSize / 2.0f - halfTile/1.5f),
		static_cast<int>(drawSize / 2.0f + halfTile/1.5f),
		static_cast<int>(drawSize / 2.0f + halfTile/1.5f));

	SelectObject(hdc,oldPen);
	DeleteObject(playerBrush);

	// 인사이트가 활성화되어 있으면 몬스터 표시
	if(showMonsters) {
		FPOINT monsterPos = MonsterManager::GetInstance()->GetClosestMonsterPos();

		if(monsterPos.x != -1 && monsterPos.y != -1) {
			float dx = monsterPos.x - pPos.x;
			float dy = monsterPos.y - pPos.y;
			float angle = atan2f(dy,dx);

			// 빨간색 방향 표시기
			DrawDirectionTriangle(hdc,angle,drawSize,halfTile,RGB(255,0,0));
		}
	}

	if(ItemManager::GetInstance()->Key() == 0) {
		auto exitPos = ObstacleManager::GetInstance()->GetElevatorPosition();

		if(exitPos.x != -1 && exitPos.y != -1) {
			float dx = exitPos.x - pPos.x;
			float dy = exitPos.y - pPos.y;
			float angle = atan2f(dy,dx);

			// 노란색 방향 표시기
			DrawDirectionTriangle(hdc,angle,drawSize,halfTile,RGB(255,255,0));
		}
	}
}

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
	FillRect(miniDC,&miniRect,bgBrush);
	DrawMiniMapToDC(miniDC,drawSize);

	BitBlt(baseDC,offset,offset,drawSize,drawSize,miniDC,0,0,SRCCOPY);

	// 회전
	HDC rotDC = CreateCompatibleDC(hdc);
	HBITMAP rotBmp = CreateCompatibleBitmap(hdc,fullSize,fullSize);
	HBITMAP oldRotBmp = (HBITMAP)SelectObject(rotDC,rotBmp);

	FillRect(rotDC,&fullRect,bgBrush);

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
	BitBlt(hdc,0,0,drawSize,drawSize,rotDC,cutOffset,cutOffset,SRCCOPY);

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

void MinimapUI::DrawDirectionTriangle(HDC hdc,float angle,int drawSize,int halfTile,COLORREF color)
{
	float playerCenterX = drawSize / 2.0f;
	float playerCenterY = drawSize / 2.0f;

	HBRUSH directionBrush = CreateSolidBrush(color);
	HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
	HPEN oldPen = (HPEN)SelectObject(hdc,nullPen);
	SelectObject(hdc,directionBrush);

	int distance = halfTile + 5; // 플레이어 아이콘보다 약간 바깥에 표시
	int dirSize = 8; // 방향 표시기 크기

	POINT dirPoints[3];
	// 방향을 가리키는 삼각형의 꼭짓점 계산
	dirPoints[0].x = static_cast<int>(playerCenterX + distance * cosf(angle));
	dirPoints[0].y = static_cast<int>(playerCenterY - distance * sinf(angle));

	dirPoints[1].x = static_cast<int>(playerCenterX + (distance - dirSize/2) * cosf(angle + 0.5f));
	dirPoints[1].y = static_cast<int>(playerCenterY - (distance - dirSize/2) * sinf(angle + 0.5f));

	dirPoints[2].x = static_cast<int>(playerCenterX + (distance - dirSize/2) * cosf(angle - 0.5f));
	dirPoints[2].y = static_cast<int>(playerCenterY - (distance - dirSize/2) * sinf(angle - 0.5f));

	// 삼각형 그리기
	Polygon(hdc,dirPoints,3);

	SelectObject(hdc,oldPen);
	DeleteObject(directionBrush);
}