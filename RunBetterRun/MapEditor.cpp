#include "MapEditor.h"
#include "MapEditorUI.h"
#include "MapEditorRender.h"
#include "TextureManager.h"
#include "KeyManager.h"
#include "DataManager.h"
#include "SceneManager.h"
#include "CommonFunction.h"

MapEditor::MapEditor()
	: ui(nullptr),render(nullptr),mode(EditorMode::TILE),
	selectedRoomType(RoomType::WALL),selectedObstacleDir(Direction::EAST),
	sampleTileImage(nullptr),mapWidth(VISIBLE_MAP_WIDTH),mapHeight(VISIBLE_MAP_HEIGHT),
	zoomLevel(1.0f), viewportOffset({ 0.0f, 0.0f }), isDragging(false)
{}

MapEditor::~MapEditor()
{
	Release();
}

HRESULT MapEditor::Init()
{
	// 에디터신 들어갈때 초기화 호출
	mode = EditorMode::TILE;
	selectedRoomType = RoomType::WALL;
	selectedObstacleDir = Direction::EAST;
	startPosition = {VISIBLE_MAP_WIDTH / 2.0f + 0.5f,VISIBLE_MAP_HEIGHT / 2.0f + 0.5f};

	// 샘플 타일 이미지 로드

	sampleTileImage = ImageManager::GetInstance()->AddImage(
		"EditorSampleTile",L"Image/tiles32x32.bmp",
		224,128,SAMPLE_TILE_X,SAMPLE_TILE_Y,
		true,RGB(255,0,255));

	if(!sampleTileImage)
	{
		MessageBox(g_hWnd,TEXT("Failed to load tile image"),TEXT("Error"),MB_OK);
		return E_FAIL;
	}

	// UI 레이아웃 계산
	int rightPanelWidth = 300;  // 오른쪽 패널 너비
	int uiPadding = 20;         // UI 요소 간 여백
	int sampleTileSize = 32;    // 샘플 타일 크기 

	// 오른쪽 패널 영역 설정
	int rightPanelLeft = TILEMAPTOOL_X - rightPanelWidth - uiPadding;

	// 맵 영역 설정 - 왼쪽 영역을 최대한 활용
	int mapAreaWidth = rightPanelLeft - (uiPadding * 2);
	int mapAreaHeight = TILEMAPTOOL_Y - (uiPadding * 4);
	int mapTileSize = 32;

	// 타일 크기를 맵 영역과 맵 크기에 맞게 조정
	int tileWidth = mapAreaWidth / mapWidth;
	int tileHeight = mapAreaHeight / mapHeight;

	// 일관성을 위해 타일을 정사각형으로 유지
	int tileSize = min(tileWidth,tileHeight);

	// 샘플 타일 영역 설정
	sampleArea.left = rightPanelLeft;
	sampleArea.top = uiPadding;
	sampleArea.right = sampleArea.left + (SAMPLE_TILE_X * sampleTileSize);
	sampleArea.bottom = sampleArea.top + (SAMPLE_TILE_Y * sampleTileSize);

	mapArea.left = uiPadding;
	mapArea.top = uiPadding;
	mapArea.right = mapArea.left + mapAreaWidth;
	mapArea.bottom = mapArea.top + mapAreaHeight;

	// UI 및 렌더러 초기화
	ui = new MapEditorUI();
	render = new MapEditorRender();

	ui->Init(sampleArea,mapArea);
	render->Init(sampleTileImage,mapTileSize);

	// 기존 맵 로드 시도
	if(DataManager::GetInstance()->LoadMapFile(L"Map/EditorMap.dat")) 
	{
		LoadFromDataManager();
	} 
	else 
	{
		InitTiles();
	}

	// 커서 표시
	while(ShowCursor(TRUE) < 0);

	return S_OK;
}

void MapEditor::Release()
{
	if(ui)
	{
		delete ui;
		ui = nullptr;
	}

	if(render)
	{
		delete render;
		render = nullptr;
	}

	// 벡터 비우기
	tiles.clear();
	editorSprites.clear();
	editorObstacles.clear();
}

void MapEditor::Update()
{
	// 마우스 위치 업데이트
	POINT mousePos;
	GetCursorPos(&mousePos);
	ScreenToClient(g_hWnd,&mousePos);
	ui->UpdateMousePosition(mousePos);

	// UI 입력 처리
	TileSelect();
	MapEdit();
	Shortcut();

	// 마우스 중간 버튼으로 드래그 처리
	if (KeyManager::GetInstance()->IsStayKeyDown(VK_MBUTTON))
	{
		if (!isDragging) 
		{
			StartDrag(mousePos);
		}
		else 
		{
			UpdateDrag(mousePos);
		}
	}
	else if (isDragging)
	{
		EndDrag();
	}
}

void MapEditor::Render(HDC hdc)
{
	// 배경 채우기
	PatBlt(hdc,0,0,WINSIZE_X,WINSIZE_Y,WHITENESS);

	// 맵 요소 렌더링 (확대/축소 및 스크롤 적용)
	render->RenderTiles(hdc,tiles,mapWidth,mapHeight,mapArea,
					   ui->GetMousePos(),ui->IsMouseInMapArea(),
					   zoomLevel,viewportOffset);

	render->RenderSprites(hdc,editorSprites,mapArea,zoomLevel,viewportOffset);
	render->RenderObstacles(hdc,editorObstacles,mapArea,zoomLevel,viewportOffset);
	render->RenderStartPosition(hdc,startPosition,tiles,mapWidth,mapArea,zoomLevel,viewportOffset);

	// UI 요소 렌더링
	render->RenderSampleTiles(hdc,sampleArea,ui->GetSelectedTile());
	render->RenderModeInfo(hdc,mode,selectedRoomType,zoomLevel);
	render->RenderControlGuide(hdc);
	render->RenderSelectedTilePreview(hdc,ui->GetSelectedTile(),sampleArea);

	// 디버그 정보 표시
	TCHAR szDebug[100];
	wsprintf(szDebug,L"Zoom: %.2f, Offset: (%.1f, %.1f), 아이템 수: %d",
			 zoomLevel,viewportOffset.x,viewportOffset.y,editorSprites.size());
	TextOut(hdc,20,TILEMAPTOOL_Y - 100,szDebug,lstrlen(szDebug));
}

void MapEditor::TileSelect()
{
	if(ui->HandleTileSelection(ui->GetMousePos(),sampleArea)) 
	{ //나중에 타일 인덱스 접근을위해 남겨놓음
	}
}

void MapEditor::MapEdit()
{
	if(ui->IsMouseInMapArea())
	{
		// 마우스 위치를 맵 타일 위치로 변환
		POINT mousePos = ui->GetMousePos();
		POINT mapPos = ui->ScreenToMap(mousePos,mapArea,mapWidth,mapHeight,zoomLevel,viewportOffset);
		int tileX = mapPos.x;
		int tileY = mapPos.y;

		// 디버그 출력
		TCHAR szDebug[100];
		wsprintf(szDebug,L"마우스 위치: 화면(%d, %d) -> 맵(%d, %d)\n",
				 mousePos.x,mousePos.y,tileX,tileY);
		OutputDebugString(szDebug);

		if(KeyManager::GetInstance()->IsStayKeyDown(VK_LBUTTON))
		{
			if(tileX >= 0 && tileX < mapWidth && tileY >= 0 && tileY < mapHeight)
			{
				switch(mode)
				{
				case EditorMode::TILE:
				PlaceTile(tileX,tileY);
				break;
				case EditorMode::START:
				PlaceStartPoint(tileX,tileY);
				break;
				case EditorMode::ITEM:
				PlaceItem(tileX,tileY);
				break;
				case EditorMode::MONSTER:
				PlaceMonster(tileX,tileY);
				break;
				case EditorMode::OBSTACLE:
				PlaceObstacle(tileX,tileY);
				break;
				}
			}
		}

		if(KeyManager::GetInstance()->IsStayKeyDown(VK_RBUTTON))
		{
			if(tileX >= 0 && tileX < mapWidth && tileY >= 0 && tileY < mapHeight)
			{
				if(mode == EditorMode::ITEM || mode == EditorMode::MONSTER)
				{
					RemoveSprite(tileX,tileY);
				} else if(mode == EditorMode::OBSTACLE)
				{
					RemoveObstacle(tileX,tileY);
				} else if(mode == EditorMode::TILE)
				{
					int index = tileY * mapWidth + tileX;
					if(index < tiles.size()) {
						tiles[index].roomType = RoomType::FLOOR;
						tiles[index].tilePos = 10;
					}
				}
			}
		}
	}
}

void MapEditor::Shortcut()
{
	KeyManager* km = KeyManager::GetInstance();

	if(km->IsOnceKeyDown(VK_ESCAPE))
	{
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}

	if(km->IsOnceKeyDown('1'))
	{
		ChangeMode(EditorMode::TILE);
	} 
	else if(km->IsOnceKeyDown('2'))
	{
		ChangeMode(EditorMode::START);
	} 
	else if(km->IsOnceKeyDown('3'))
	{
		ChangeMode(EditorMode::ITEM);
	}	
	else if(km->IsOnceKeyDown('4'))
	{
		ChangeMode(EditorMode::MONSTER);
	} 
	else if(km->IsOnceKeyDown('5'))
	{
		ChangeMode(EditorMode::OBSTACLE);
	}

	if(km->IsOnceKeyDown('S'))
	{
		SaveMap();
	} 
	else if(km->IsOnceKeyDown('L'))
	{
		LoadMap();
	} 
	else if(km->IsOnceKeyDown('C'))
	{
		ClearMap();
	}

	if(km->IsOnceKeyDown('F'))
	{
		selectedRoomType = RoomType::FLOOR;
	} 
	else if(km->IsOnceKeyDown('W'))
	{
		selectedRoomType = RoomType::WALL;
	}
	else if(km->IsOnceKeyDown('G'))
	{
		selectedRoomType = RoomType::START;
	}

	if (km->IsOnceKeyDown(VK_OEM_PLUS) || km->IsOnceKeyDown(VK_ADD))
	{
		Zoom(0.1f); // 확대
	}
	else if (km->IsOnceKeyDown(VK_OEM_MINUS) || km->IsOnceKeyDown(VK_SUBTRACT))
	{
		Zoom(-0.1f); // 축소
	}

	if(mode == EditorMode::OBSTACLE)
	{
		if(km->IsOnceKeyDown(VK_UP))
		{
			ChangeObstacleDirection(Direction::NORTH);
		}
		else if(km->IsOnceKeyDown(VK_DOWN))
		{
			ChangeObstacleDirection(Direction::SOUTH);
		}
		else if(km->IsOnceKeyDown(VK_LEFT))
		{
			ChangeObstacleDirection(Direction::WEST);
		} 
		else if(km->IsOnceKeyDown(VK_RIGHT))
		{
			ChangeObstacleDirection(Direction::EAST);
		}
	}
}

void MapEditor::PlaceTile(int x,int y)
{
	int tileIndex = ui->GetSelectedTile().y * SAMPLE_TILE_X + ui->GetSelectedTile().x;
	int index = y * mapWidth + x;


	if(index < tiles.size()) {
		tiles[index].tilePos = tileIndex;
		tiles[index].roomType = selectedRoomType;
	}
}

void MapEditor::PlaceStartPoint(int x,int y)
{
	// 이미 있는 시작 위치 제거
	for(size_t i = 0; i < tiles.size(); i++) {
		if(tiles[i].roomType == RoomType::START) {
			tiles[i].roomType = RoomType::FLOOR;
			tiles[i].tilePos = 10;
		}
	}

	// 새 시작 위치 설정
	int index = y * mapWidth + x;
	if(index < tiles.size()) {
		tiles[index].roomType = RoomType::START;
		startPosition = {x + 0.5f,y + 0.5f}; // 타일 중앙에 위치
	}
}

void MapEditor::PlaceItem(int x,int y)
{
	// 해당 위치에 아이템이 이미 있는지 확인
	if(FindSprite(x,y) >= 0)
		return;

	Texture* keyTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	if(keyTexture)
	{
		// 타일 중앙에 정확히 배치 - 0.5f 오프셋 추가
		FPOINT centerPos = {static_cast<float>(x) + 0.5f,static_cast<float>(y) + 0.5f};
		AddSprite(centerPos,keyTexture,SpriteType::KEY);
	}
}


void MapEditor::PlaceMonster(int x,int y)
{
	// 해당 위치에 몬스터가 이미 있는지 확인
	if(FindSprite(x,y) >= 0)
		return;

	Texture* monsterTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));
	if(monsterTexture)
	{
		// 타일 중앙에 정확히 배치 - 0.5f 오프셋 추가
		FPOINT centerPos = {static_cast<float>(x) + 0.5f,static_cast<float>(y) + 0.5f};
		AddSprite(centerPos,monsterTexture,SpriteType::MONSTER);
	}
}

void MapEditor::PlaceObstacle(int x,int y)
{
	if(FindObstacle(x,y) >= 0)
		return;

	Texture* obstacleTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	if(obstacleTexture)
	{
		POINT pos = {x,y};
		AddObstacle(pos,obstacleTexture,selectedObstacleDir);

		// 디버그 출력
		TCHAR szDebug[100];
		wsprintf(szDebug,L"장애물 배치: (%d, %d), 방향: %d\n",x,y,(int)selectedObstacleDir);
		OutputDebugString(szDebug);
	}
}

void MapEditor::ChangeMode(EditorMode newMode)
{
	mode = newMode;
}

void MapEditor::ChangeObstacleDirection(Direction dir)
{
	selectedObstacleDir = dir;
}

void MapEditor::Zoom(float delta)
{
	// 확대/축소 비율 계산 (최소 0.5, 최대 3.0으로 제한)
	float newZoomLevel = zoomLevel + delta;
	newZoomLevel = max(0.5f, min(newZoomLevel, 3.0f));

	// 마우스 위치 기준 확대/축소를 위한 계산
	POINT mousePos = ui->GetMousePos();

	// 현재 마우스 위치의 맵 좌표 계산
	int tileWidth = (mapArea.right - mapArea.left) / (VISIBLE_MAP_WIDTH / zoomLevel);
	int tileHeight = (mapArea.bottom - mapArea.top) / (VISIBLE_MAP_HEIGHT / zoomLevel);

	float mouseMapX = viewportOffset.x + (mousePos.x - mapArea.left) / (float)tileWidth;
	float mouseMapY = viewportOffset.y + (mousePos.y - mapArea.top) / (float)tileHeight;

	// 확대/축소 비율 변경
	zoomLevel = newZoomLevel;

	// 새 타일 크기
	tileWidth = (mapArea.right - mapArea.left) / (VISIBLE_MAP_WIDTH / zoomLevel);
	tileHeight = (mapArea.bottom - mapArea.top) / (VISIBLE_MAP_HEIGHT / zoomLevel);

	// 새 뷰포트 오프셋 계산 (마우스 위치가 동일한 맵 좌표를 가리키도록)
	viewportOffset.x = mouseMapX - (mousePos.x - mapArea.left) / (float)tileWidth;
	viewportOffset.y = mouseMapY - (mousePos.y - mapArea.top) / (float)tileHeight;

	// 뷰포트 오프셋 범위 제한
	viewportOffset.x = max(0.0f, min(viewportOffset.x, mapWidth - VISIBLE_MAP_WIDTH / zoomLevel));
	viewportOffset.y = max(0.0f, min(viewportOffset.y, mapHeight - VISIBLE_MAP_HEIGHT / zoomLevel));
}

int MapEditor::FindSprite(int x,int y)
{
	for(size_t i = 0; i < editorSprites.size(); i++)
	{
		float spriteX = editorSprites[i].pos.x - 0.5f;
		float spriteY = editorSprites[i].pos.y - 0.5f;

		if(static_cast<int>(spriteX) == x && static_cast<int>(spriteY) == y)
		{
			return i;
		}
	}
	return -1;
}

void MapEditor::StartDrag(POINT mousePos)
{
	isDragging = true;
	lastMousePos = mousePos;
}

void MapEditor::EndDrag()
{
	isDragging = false;
}

void MapEditor::UpdateDrag(POINT mousePos)
{
	if (isDragging)
	{
		// 마우스 이동 거리 계산
		int deltaX = mousePos.x - lastMousePos.x;
		int deltaY = mousePos.y - lastMousePos.y;

		// 타일 크기 계산
		int tileWidth = (mapArea.right - mapArea.left) / (VISIBLE_MAP_WIDTH / zoomLevel);
		int tileHeight = (mapArea.bottom - mapArea.top) / (VISIBLE_MAP_HEIGHT / zoomLevel);

		// 스크롤 오프셋 업데이트
		viewportOffset.x -= deltaX / (float)tileWidth;
		viewportOffset.y -= deltaY / (float)tileHeight;

		// 오프셋 범위 제한
		viewportOffset.x = max(0.0f, min(viewportOffset.x, mapWidth - VISIBLE_MAP_WIDTH / zoomLevel));
		viewportOffset.y = max(0.0f, min(viewportOffset.y, mapHeight - VISIBLE_MAP_HEIGHT / zoomLevel));

		lastMousePos = mousePos;
	}
}

void MapEditor::AddSprite(FPOINT position,Texture* texture,SpriteType type)
{
	Sprite newSprite;
	newSprite.pos = position;
	newSprite.texture = texture;
	newSprite.type = type;
	newSprite.distance = 0.0f;

	if(type == SpriteType::KEY)
	{
		newSprite.aniInfo = {0.1f,0.1f,{456,488},{10,1},{0,0}};
	} 
	else if(type == SpriteType::MONSTER)
	{
		newSprite.aniInfo = {0.1f,0.1f,{423,437},{1,1},{0,0}};
	}
	else
	{
		newSprite.aniInfo = {0.1f,0.1f,{0,0},{1,1},{0,0}};
	}

	editorSprites.push_back(newSprite);
}

void MapEditor::RemoveSprite(int x,int y)
{
	int index = FindSprite(x,y);
	if(index >= 0)
	{
		editorSprites.erase(editorSprites.begin() + index);
	}
}

int MapEditor::FindObstacle(int x,int y)
{
	for(size_t i = 0; i < editorObstacles.size(); i++)
	{
		if(editorObstacles[i].pos.x == x && editorObstacles[i].pos.y == y)
		{
			return i;
		}
	}
	return -1;
}

void MapEditor::AddObstacle(POINT position,Texture* texture,Direction dir)
{
	Obstacle newObstacle;
	newObstacle.pos = position;
	newObstacle.texture = texture;
	newObstacle.dir = dir;

	editorObstacles.push_back(newObstacle);
}

void MapEditor::RemoveObstacle(int x,int y)
{
	int index = FindObstacle(x,y);
	if(index >= 0)
	{
		editorObstacles.erase(editorObstacles.begin() + index);
	}
}

void MapEditor::InitTiles()
{
	// 타일 초기화
	tiles.resize(mapWidth * mapHeight);

	for(int y = 0; y < mapHeight; y++)
	{
		for(int x = 0; x < mapWidth; x++)
		{
			int index = y * mapWidth + x;

			if(x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1)
			{
				tiles[index].tilePos = 4;
				tiles[index].roomType = RoomType::WALL;
			}
			else
			{
				tiles[index].tilePos = 10;
				tiles[index].roomType = RoomType::FLOOR;
			}
		}
	}

	int centerX = mapWidth / 2;
	int centerY = mapHeight / 2;
	tiles[centerY * mapWidth + centerX].roomType = RoomType::START;
	startPosition = {centerX + 0.5f,centerY + 0.5f};
}

void MapEditor::MouseWheel(int delta)
{
	// Ctrl + 휠을 위로 돌리면 확대, 아래로 돌리면 축소
	Zoom(delta > 0 ? 0.1f : -0.1f);
}

void MapEditor::VerticalScroll(int delta)
{
	float scrollAmount = 3.0f;

	float visibleMapHeight = VISIBLE_MAP_HEIGHT / zoomLevel;
	float maxY = max(0.0f,mapHeight - visibleMapHeight);

	if(delta > 0)
	{
		viewportOffset.y = max(0.0f,viewportOffset.y - scrollAmount / zoomLevel);
	} else
	{
		viewportOffset.y = min(maxY,viewportOffset.y + scrollAmount / zoomLevel);
	}
}

void MapEditor::HorizontalScroll(int delta)
{
	float scrollAmount = 3.0f;

	float visibleMapWidth = VISIBLE_MAP_WIDTH / zoomLevel;
	float maxX = max(0.0f,mapWidth - visibleMapWidth);

	if(delta > 0)
	{
		viewportOffset.x = max(0.0f,viewportOffset.x - scrollAmount / zoomLevel);
	} else
	{
		viewportOffset.x = min(maxX,viewportOffset.x + scrollAmount / zoomLevel);
	}
}


void MapEditor::SaveMap()
{
	PrepareDataForSave();

	if(DataManager::GetInstance()->SaveMapFile(L"Map/EditorMap.dat"))
	{
		MessageBox(g_hWnd,TEXT("Map Saved Successfully"),TEXT("Success"),MB_OK);
	} else 
	{
		MessageBox(g_hWnd,TEXT("Failed to Save Map"),TEXT("Error"),MB_OK);
	}
}

void MapEditor::LoadMap()
{
	if(DataManager::GetInstance()->LoadMapFile(L"Map/EditorMap.dat")) 
	{
		LoadFromDataManager();
		MessageBox(g_hWnd,TEXT("Map Loaded Successfully"),TEXT("Success"),MB_OK);
	} else 
	{
		MessageBox(g_hWnd,TEXT("Failed to Load Map"),TEXT("Error"),MB_OK);
	}
}

void MapEditor::PrepareDataForSave()
{
	// 가장자리를 벽으로 강제 설정
	for(int y = 0; y < mapHeight; y++)
	{
		for(int x = 0; x < mapWidth; x++)
		{
			if(x == 0 || y == 0 || x == mapWidth - 1 || y == mapHeight - 1)
			{
				int index = y * mapWidth + x;
				tiles[index].roomType = RoomType::WALL;
				tiles[index].tilePos = 4;  // 벽 타일 인덱스
			}
		}
	}

	// DataManager에 저장할 데이터 준비
	DataManager::GetInstance()->ClearAllData();
	// 맵 데이터 설정
	DataManager::GetInstance()->SetMapData(tiles,mapWidth,mapHeight);
	// 텍스처 정보 설정
	DataManager::GetInstance()->SetTextureInfo(L"Image/tiles.bmp",
											 128,SAMPLE_TILE_X,SAMPLE_TILE_Y);
	// 시작 위치 설정
	DataManager::GetInstance()->SetStartPosition(startPosition);
	// 아이템, 몬스터, 장애물 데이터 추가
	for(const auto& sprite : editorSprites)
	{
		if(sprite.type == SpriteType::KEY)
		{
			ItemData item;
			item.pos = sprite.pos;
			item.aniInfo = sprite.aniInfo;
			item.itemType = 0;  // Key
			DataManager::GetInstance()->AddItemData(item);
		} 
		else if(sprite.type == SpriteType::MONSTER)
		{
			MonsterData monster;
			monster.pos = sprite.pos;
			monster.aniInfo = sprite.aniInfo;
			monster.monsterType = 0;  // Tentacle
			DataManager::GetInstance()->AddMonsterData(monster);
		}
	}

	for(const auto& obstacle : editorObstacles)
	{
		ObstacleData obsData;
		obsData.pos = obstacle.pos;
		obsData.dir = obstacle.dir;
		DataManager::GetInstance()->AddObstacleData(obsData);
	}
}

void MapEditor::LoadFromDataManager()
{
	// 맵 데이터 로드
	MapData mapData;
	if(DataManager::GetInstance()->GetMapData(mapData)) {
		// 타일 데이터 복원
		mapWidth = mapData.width;
		mapHeight = mapData.height;
		tiles = mapData.tiles;
	}

	// 시작 위치 복원
	startPosition = DataManager::GetInstance()->GetStartPosition();

	//데이터 초기화
	editorSprites.clear();
	editorObstacles.clear();

	// 아이템 복원
	const auto& items = DataManager::GetInstance()->GetItems();
	for(const auto& item : items) {
		Sprite sprite;
		sprite.pos = item.pos;
		sprite.type = SpriteType::KEY;
		sprite.aniInfo = item.aniInfo;
		sprite.texture = TextureManager::GetInstance()->GetTexture(L"Image/jewel.bmp");
		sprite.distance = 0.0f;
		editorSprites.push_back(sprite);
	}

	// 몬스터 복원
	const auto& monsters = DataManager::GetInstance()->GetMonsters();
	for(const auto& monster : monsters) {
		Sprite sprite;
		sprite.pos = monster.pos;
		sprite.type = SpriteType::MONSTER;
		sprite.aniInfo = monster.aniInfo;
		sprite.texture = TextureManager::GetInstance()->GetTexture(L"Image/boss.bmp");
		sprite.distance = 0.0f;
		editorSprites.push_back(sprite);
	}

	// 장애물 복원 
	const auto& obstacles = DataManager::GetInstance()->GetObstacles();
	for(const auto& obstacle : obstacles) {
		Obstacle editorObstacle;
		editorObstacle.pos = obstacle.pos;
		editorObstacle.dir = obstacle.dir;
		editorObstacle.texture = TextureManager::GetInstance()->GetTexture(L"Image/jewel.bmp");
		editorObstacles.push_back(editorObstacle);
	}
}

void MapEditor::ClearMap()
{
	// 맵 초기화
	InitTiles();

	// 스프라이트와 장애물 초기화
	editorSprites.clear();
	editorObstacles.clear();

	MessageBox(g_hWnd,TEXT("Map Cleared"),TEXT("Success"),MB_OK);
}