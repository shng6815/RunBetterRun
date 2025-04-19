#include "MapEditor.h"
#include "Image.h"
#include "Button.h"
#include "MapManager.h"
#include "TextureManager.h"
#include "SceneManager.h"

HRESULT MapEditor::Init()
{
	// 에디터 모드 초기화
	mode = EditorMode::TILE;
	selectedTile = {0,0};
	selectedRoomType = RoomType::FLOOR;

	// 샘플 타일 이미지 로드
	sampleTileImage = ImageManager::GetInstance()->AddImage(
		"EditorSampleTile",L"Image/horrorMapTiles.bmp",
		 1408,1408,
		SAMPLE_TILE_X,SAMPLE_TILE_Y,  
		true,RGB(255,0,255));

	if(!sampleTileImage) {
		MessageBox(g_hWnd,TEXT("타일 이미지 로드 실패"),TEXT("오류"),MB_OK);
		return E_FAIL;
	}

	// 샘플 타일 영역 설정
	rcSampleArea.left = WINSIZE_X - (SAMPLE_TILE_X * TILE_SIZE) - 20;
	rcSampleArea.top = 50;
	rcSampleArea.right = rcSampleArea.left + (SAMPLE_TILE_X * TILE_SIZE);
	rcSampleArea.bottom = rcSampleArea.top + (SAMPLE_TILE_Y * TILE_SIZE);

	// 맵 영역 설정
	rcMapArea.left = 20;
	rcMapArea.top = 50;
	rcMapArea.right = rcMapArea.left + (MAP_EDITOR_WIDTH * TILE_SIZE);
	rcMapArea.bottom = rcMapArea.top + (MAP_EDITOR_HEIGHT * TILE_SIZE);

	// 맵 타일 초기화
	InitTiles();

	// 버튼 초기화
	InitButtons();

	// 마우스 커서 표시
	while(ShowCursor(TRUE) < 0);

	return S_OK;
}

void MapEditor::InitTiles()
{
	// 기존 맵 데이터 확인
	MapData* existingMap = MapManager::GetInstance()->GetMapData();

	for(int y = 0; y < MAP_EDITOR_HEIGHT; y++) {
		for(int x = 0; x < MAP_EDITOR_WIDTH; x++) {
			// 타일 위치 설정
			mapTiles[y][x].rc.left = rcMapArea.left + (x * TILE_SIZE);
			mapTiles[y][x].rc.top = rcMapArea.top + (y * TILE_SIZE);
			mapTiles[y][x].rc.right = mapTiles[y][x].rc.left + TILE_SIZE;
			mapTiles[y][x].rc.bottom = mapTiles[y][x].rc.top + TILE_SIZE;

			// 기존 맵 데이터가 있으면 복사
			if(existingMap && x < existingMap->width && y < existingMap->height) {
				int index = y * existingMap->width + x;
				mapTiles[y][x].tileIndex = existingMap->tiles[index].tilePos;
				mapTiles[y][x].type = existingMap->tiles[index].roomType;
			} else {
				// 기본 타일 설정
				if(x == 0 || y == 0 || x == MAP_EDITOR_WIDTH - 1 || y == MAP_EDITOR_HEIGHT - 1) {
					// 테두리는 벽으로
					mapTiles[y][x].tileIndex = 4; // 벽 타일 인덱스
					mapTiles[y][x].type = RoomType::WALL;
				} else {
					// 내부는 바닥으로
					mapTiles[y][x].tileIndex = 10; // 바닥 타일 인덱스
					mapTiles[y][x].type = RoomType::FLOOR;
				}
			}
		}
	}

	// 시작 위치 설정
	int centerX = MAP_EDITOR_WIDTH / 2;
	int centerY = MAP_EDITOR_HEIGHT / 2;
	mapTiles[centerY][centerX].type = RoomType::START;
}

void MapEditor::Release()
{
	// 버튼 해제
	for(auto& button : buttons) {
		button->Release();
		delete button;
	}
	buttons.clear();

	// 스프라이트 해제
	editorSprites.clear();
}

void MapEditor::Update()
{
	// 마우스 위치 업데이트
	GetCursorPos(&mousePos);
	ScreenToClient(g_hWnd,&mousePos);

	// 버튼 업데이트
	for(auto& button : buttons) {
		button->Update();
	}

	// 샘플 타일에서 선택
	if(PtInRect(&rcSampleArea,mousePos)) {
		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON)) {
			int relX = mousePos.x - rcSampleArea.left;
			int relY = mousePos.y - rcSampleArea.top;

			selectedTile.x = relX / TILE_SIZE;
			selectedTile.y = relY / TILE_SIZE;

			// 타일 유형 설정
			int tileIndex = selectedTile.y * SAMPLE_TILE_X + selectedTile.x;
			if(tileIndex < 5) {
				selectedRoomType = RoomType::WALL;
			} else {
				selectedRoomType = RoomType::FLOOR;
			}
		}
	}

	// 맵 영역에 요소 배치
	if(PtInRect(&rcMapArea,mousePos)) {
		if(KeyManager::GetInstance()->IsStayKeyDown(VK_LBUTTON)) {
			int relX = mousePos.x - rcMapArea.left;
			int relY = mousePos.y - rcMapArea.top;

			int tileX = relX / TILE_SIZE;
			int tileY = relY / TILE_SIZE;

			if(tileX >= 0 && tileX < MAP_EDITOR_WIDTH && tileY >= 0 && tileY < MAP_EDITOR_HEIGHT) {
				switch(mode) {
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
				}
			}
		}

		// 우클릭으로 스프라이트 삭제
		if(KeyManager::GetInstance()->IsOnceKeyDown(VK_RBUTTON)) {
			int relX = mousePos.x - rcMapArea.left;
			int relY = mousePos.y - rcMapArea.top;

			int tileX = relX / TILE_SIZE;
			int tileY = relY / TILE_SIZE;

			if(mode == EditorMode::ITEM || mode == EditorMode::MONSTER) {
				RemoveSprite(tileX,tileY);
			}
		}
	}

	// 단축키 처리
	if(KeyManager::GetInstance()->IsOnceKeyDown('S') &&
		KeyManager::GetInstance()->IsStayKeyDown(VK_CONTROL)) {
		SaveMap();
	} else if(KeyManager::GetInstance()->IsOnceKeyDown('L') &&
			  KeyManager::GetInstance()->IsStayKeyDown(VK_CONTROL)) {
		LoadMap();
	}

	// ESC 키로 게임으로 돌아가기
	if(KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)) {
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}
}

void MapEditor::Render(HDC hdc)
{
	// 배경 그리기
	PatBlt(hdc,0,0,WINSIZE_X,WINSIZE_Y,WHITENESS);

	// 맵 영역 테두리
	Rectangle(hdc,rcMapArea.left - 1,rcMapArea.top - 1,
			 rcMapArea.right + 1,rcMapArea.bottom + 1);

	// 맵 타일 그리기
	RenderTiles(hdc);

	// 스프라이트 그리기
	RenderSprites(hdc);

	// 샘플 타일 영역 테두리
	Rectangle(hdc,rcSampleArea.left - 1,rcSampleArea.top - 1,
			 rcSampleArea.right + 1,rcSampleArea.bottom + 1);

	// 샘플 타일 그리기
	RenderSampleTiles(hdc);

	// 현재 선택된 타일 표시
	RECT rcSelected = {20,10,60,40};
	Rectangle(hdc,rcSelected.left,rcSelected.top,rcSelected.right,rcSelected.bottom);

	if(sampleTileImage) {
		sampleTileImage->FrameRender(hdc,
								   (rcSelected.left + rcSelected.right) / 2,
								   (rcSelected.top + rcSelected.bottom) / 2,
								   selectedTile.x,selectedTile.y);
	}

	/// 모드 정보 표시
	TCHAR szText[128];
	LPCWSTR modeName = TEXT("Tile");  

	// 현재 모드에 따라 텍스트 결정
	if(mode == EditorMode::TILE)
		modeName = TEXT("Tile");
	else if(mode == EditorMode::START)
		modeName = TEXT("Start");
	else if(mode == EditorMode::ITEM)
		modeName = TEXT("Item");
	else if(mode == EditorMode::MONSTER)
		modeName = TEXT("Monster");

	// 문자열 포맷팅
	wsprintf(szText,TEXT("CurrentMode: %s"),modeName);
	TextOut(hdc,80,20,szText,lstrlen(szText));

	// 도움말 표시
	TextOut(hdc,20,rcMapArea.bottom + 20,TEXT("LeftClick: add, RightClick: delete"),
		   lstrlen(TEXT("LeftClick: add, RightClick: delete")));
	TextOut(hdc,20,rcMapArea.bottom + 40,TEXT("Ctrl+S: save, Ctrl+L: load"),
		   lstrlen(TEXT("Ctrl+S: save, Ctrl+L: load")));
	TextOut(hdc,20,rcMapArea.bottom + 60,TEXT("ESC: return to game"),
		   lstrlen(TEXT("ESC: return to game")));

	// 버튼 렌더링
	for(auto& button : buttons) {
		button->Render(hdc);
	}
}

void MapEditor::RenderTiles(HDC hdc)
{
	for(int y = 0; y < MAP_EDITOR_HEIGHT; y++) {
		for(int x = 0; x < MAP_EDITOR_WIDTH; x++) {
			// 타일 인덱스
			int tileIndex = mapTiles[y][x].tileIndex;
			int frameX = tileIndex % SAMPLE_TILE_X;
			int frameY = tileIndex / SAMPLE_TILE_X;

			// 타일 렌더링
			if(sampleTileImage) {
				sampleTileImage->FrameRender(hdc,
										  mapTiles[y][x].rc.left + TILE_SIZE / 2,
										  mapTiles[y][x].rc.top + TILE_SIZE / 2,
										  frameX,frameY);
			}

			// 시작 위치 표시
			if(mapTiles[y][x].type == RoomType::START) {
				HBRUSH hBrush = CreateSolidBrush(RGB(0,255,0));
				HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,hBrush);

				Ellipse(hdc,
					   mapTiles[y][x].rc.left + TILE_SIZE/4,
					   mapTiles[y][x].rc.top + TILE_SIZE/4,
					   mapTiles[y][x].rc.right - TILE_SIZE/4,
					   mapTiles[y][x].rc.bottom - TILE_SIZE/4);

				SelectObject(hdc,oldBrush);
				DeleteObject(hBrush);
			}
		}
	}
}

void MapEditor::RenderSampleTiles(HDC hdc)
{
	if(sampleTileImage) {
		for(int y = 0; y < SAMPLE_TILE_Y; y++) {
			for(int x = 0; x < SAMPLE_TILE_X; x++) {
				sampleTileImage->FrameRender(hdc,
										  rcSampleArea.left + x * TILE_SIZE + TILE_SIZE / 2,
										  rcSampleArea.top + y * TILE_SIZE + TILE_SIZE / 2,
										  x,y);
			}
		}
	}
}

void MapEditor::RenderSprites(HDC hdc)
{
	for(const auto& sprite : editorSprites) {
		int tileX = static_cast<int>(sprite.pos.x - 0.5f);
		int tileY = static_cast<int>(sprite.pos.y - 0.5f);

		int posX = rcMapArea.left + tileX * TILE_SIZE;
		int posY = rcMapArea.top + tileY * TILE_SIZE;

		// 스프라이트 타입에 따라 다른 색상으로 표시
		COLORREF color;
		if(sprite.type == SpriteType::KEY) { // 아이템
			color = RGB(0,0,255);
		} else if(sprite.type == SpriteType::MONSTER) { // 몬스터
			color = RGB(255,0,0);
		} else {
			color = RGB(128,128,128); // 기타
		}

		HBRUSH hBrush = CreateSolidBrush(color);
		HBRUSH oldBrush = (HBRUSH)SelectObject(hdc,hBrush);

		Ellipse(hdc,
			   posX + TILE_SIZE/4,
			   posY + TILE_SIZE/4,
			   posX + TILE_SIZE - TILE_SIZE/4,
			   posY + TILE_SIZE - TILE_SIZE/4);

		SelectObject(hdc,oldBrush);
		DeleteObject(hBrush);
	}
}

void MapEditor::InitButtons()
{
	// 버튼 위치 계산
	int buttonWidth = 80;
	int buttonHeight = 30;
	int buttonSpacing = 10;
	int startX = rcMapArea.left;
	int startY = rcMapArea.bottom + 80;

	// 모드 버튼들
	Button* tileButton = new Button();
	tileButton->Init(startX,startY);
	tileButton->SetFunction([this]() { this->ChangeMode(EditorMode::TILE); });
	buttons.push_back(tileButton);

	Button* startButton = new Button();
	startButton->Init(startX + buttonWidth + buttonSpacing,startY);
	startButton->SetFunction([this]() { this->ChangeMode(EditorMode::START); });
	buttons.push_back(startButton);

	Button* itemButton = new Button();
	itemButton->Init(startX + (buttonWidth + buttonSpacing) * 2,startY);
	itemButton->SetFunction([this]() { this->ChangeMode(EditorMode::ITEM); });
	buttons.push_back(itemButton);

	Button* monsterButton = new Button();
	monsterButton->Init(startX + (buttonWidth + buttonSpacing) * 3,startY);
	monsterButton->SetFunction([this]() { this->ChangeMode(EditorMode::MONSTER); });
	buttons.push_back(monsterButton);

	// 저장/로드 버튼
	Button* saveButton = new Button();
	saveButton->Init(startX,startY + buttonHeight + buttonSpacing);
	saveButton->SetFunction([this]() { this->SaveMap(); });
	buttons.push_back(saveButton);

	Button* loadButton = new Button();
	loadButton->Init(startX + buttonWidth + buttonSpacing,startY + buttonHeight + buttonSpacing);
	loadButton->SetFunction([this]() { this->LoadMap(); });
	buttons.push_back(loadButton);

	// 게임으로 돌아가기 버튼
	Button* returnButton = new Button();
	returnButton->Init(startX + (buttonWidth + buttonSpacing) * 2,startY + buttonHeight + buttonSpacing);
	returnButton->SetFunction([]() { SceneManager::GetInstance()->ChangeScene("MainGameScene"); });
	buttons.push_back(returnButton);
}

void MapEditor::ChangeMode(EditorMode newMode)
{
	mode = newMode;
}

void MapEditor::PlaceTile(int x,int y)
{
	int tileIndex = selectedTile.y * SAMPLE_TILE_X + selectedTile.x;
	mapTiles[y][x].tileIndex = tileIndex;
	mapTiles[y][x].type = selectedRoomType;
}

void MapEditor::PlaceStartPoint(int x,int y)
{
	// 기존 시작 위치를 찾아 바닥으로 변경
	for(int j = 0; j < MAP_EDITOR_HEIGHT; j++) {
		for(int i = 0; i < MAP_EDITOR_WIDTH; i++) {
			if(mapTiles[j][i].type == RoomType::START) {
				mapTiles[j][i].type = RoomType::FLOOR;
			}
		}
	}

	// 새 시작 위치 설정
	mapTiles[y][x].type = RoomType::START;
}

void MapEditor::PlaceItem(int x,int y)
{
	// 해당 위치에 이미 스프라이트가 있는지 확인
	if(FindSpriteAt(x,y) >= 0) {
		return; // 이미 있으면 배치하지 않음
	}

	// 아이템용 텍스처 가져오기
	Texture* itemTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	if(itemTexture) {
		// 타일 중앙 위치로 변환
		FPOINT centerPos = {x + 0.5f,y + 0.5f};
		AddSprite(centerPos,itemTexture,SpriteType::KEY);
	}
}

void MapEditor::PlaceMonster(int x,int y)
{
	// 해당 위치에 이미 스프라이트가 있는지 확인
	if(FindSpriteAt(x,y) >= 0) {
		return; // 이미 있으면 배치하지 않음
	}

	// 몬스터용 텍스처 가져오기
	Texture* monsterTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));
	if(monsterTexture) {
		// 타일 중앙 위치로 변환
		FPOINT centerPos = {x + 0.5f,y + 0.5f};
		AddSprite(centerPos,monsterTexture,SpriteType::MONSTER);
	}
}

void MapEditor::AddSprite(FPOINT position,Texture* texture,SpriteType type)
{
	Sprite newSprite;
	newSprite.pos = position;
	newSprite.texture = texture;
	newSprite.type = type;

	// 애니메이션 정보 설정
	if(type == SpriteType::KEY) {
		newSprite.aniInfo = {0.1f,0.1f,{456,488},{10,1},{0,0}};
		newSprite.distance = 0.0f;
	} else {
		newSprite.aniInfo = {0.0f,0.0f,{423,437},{1,1},{0,0}};
		newSprite.distance = 1.0f;
	}

	editorSprites.push_back(newSprite);
}

void MapEditor::RemoveSprite(int x,int y)
{
	int index = FindSpriteAt(x,y);
	if(index >= 0) {
		editorSprites.erase(editorSprites.begin() + index);
	}
}

int MapEditor::FindSpriteAt(int x,int y)
{
	for(size_t i = 0; i < editorSprites.size(); i++) {
		float spriteX = editorSprites[i].pos.x - 0.5f;
		float spriteY = editorSprites[i].pos.y - 0.5f;

		if(static_cast<int>(spriteX) == x && static_cast<int>(spriteY) == y) {
			return i;
		}
	}
	return -1; // 찾지 못함
}

void MapEditor::SaveMap()
{
	// 맵 저장 파일 경로
	const LPCWCH mapFilePath = L"Map/EditorMap.dat";

	// Map 디렉토리 생성
	CreateDirectory(L"Map",NULL);

	// 파일 생성
	HANDLE hFile = CreateFile(
		mapFilePath,GENERIC_WRITE,0,NULL,
		CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);

	if(hFile == INVALID_HANDLE_VALUE) {
		MessageBox(g_hWnd,TEXT("맵 파일 생성 실패"),TEXT("오류"),MB_OK);
		return;
	}

	DWORD bytesWritten = 0;

	// MapData 구조체로 변환
	MapData mapData;
	mapData.width = MAP_EDITOR_WIDTH;
	mapData.height = MAP_EDITOR_HEIGHT;
	mapData.tiles.resize(MAP_EDITOR_WIDTH * MAP_EDITOR_HEIGHT);

	// 중요: texture 포인터 설정
	mapData.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/horrorMapTiles.bmp"));
	mapData.textureTileSize = TILE_SIZE;
	mapData.textureTileRowSize = SAMPLE_TILE_X;
	mapData.textureTileColumnSize = SAMPLE_TILE_Y;

	for(int y = 0; y < MAP_EDITOR_HEIGHT; y++) {
		for(int x = 0; x < MAP_EDITOR_WIDTH; x++) {
			int index = y * MAP_EDITOR_WIDTH + x;
			mapData.tiles[index].roomType = mapTiles[y][x].type;
			mapData.tiles[index].tilePos = mapTiles[y][x].tileIndex;
		}
	}

	// 맵 크기 정보 저장
	WriteFile(hFile,&mapData.width,sizeof(int),&bytesWritten,NULL);
	WriteFile(hFile,&mapData.height,sizeof(int),&bytesWritten,NULL);

	// 타일 데이터 개수
	int tileCount = mapData.tiles.size();
	WriteFile(hFile,&tileCount,sizeof(int),&bytesWritten,NULL);

	// 타일 데이터 저장
	WriteFile(hFile,mapData.tiles.data(),sizeof(Room) * tileCount,&bytesWritten,NULL);

	// 아이템 개수 저장
	int itemCount = 0;
	for(const auto& sprite : editorSprites) {
		if(sprite.type == SpriteType::KEY) {
			itemCount++;
		}
	}
	WriteFile(hFile,&itemCount,sizeof(int),&bytesWritten,NULL);

	// 아이템 위치 정보 저장
	for(const auto& sprite : editorSprites) {
		if(sprite.type == SpriteType::KEY) {
			WriteFile(hFile,&sprite.pos,sizeof(FPOINT),&bytesWritten,NULL);
		}
	}

	// 몬스터 개수 저장
	int monsterCount = 0;
	for(const auto& sprite : editorSprites) {
		if(sprite.type == SpriteType::MONSTER) {
			monsterCount++;
		}
	}
	WriteFile(hFile,&monsterCount,sizeof(int),&bytesWritten,NULL);

	// 몬스터 위치 정보 저장
	for(const auto& sprite : editorSprites) {
		if(sprite.type == SpriteType::MONSTER) {
			WriteFile(hFile,&sprite.pos,sizeof(FPOINT),&bytesWritten,NULL);
		}
	}

	CloseHandle(hFile);

	// MapManager에도 현재 맵 데이터 복사
	MapData* managerMapData = MapManager::GetInstance()->GetMapData();
	managerMapData->width = mapData.width;
	managerMapData->height = mapData.height;
	managerMapData->tiles.resize(mapData.width * mapData.height);

	// 중요: texture 포인터도 복사
	managerMapData->texture = mapData.texture;
	managerMapData->textureTileSize = mapData.textureTileSize;
	managerMapData->textureTileRowSize = mapData.textureTileRowSize;
	managerMapData->textureTileColumnSize = mapData.textureTileColumnSize;

	for(int i = 0; i < mapData.width * mapData.height; i++) {
		managerMapData->tiles[i] = mapData.tiles[i];
	}

	MessageBox(g_hWnd,TEXT("맵 저장 완료"),TEXT("알림"),MB_OK);
}

void MapEditor::LoadMap()
{
	// 맵 로드 파일 경로
	const LPCWCH mapFilePath = L"Map/EditorMap.dat";

	HANDLE hFile = CreateFile(
		mapFilePath,GENERIC_READ,0,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if(hFile == INVALID_HANDLE_VALUE) {
		MessageBox(g_hWnd,TEXT("맵 파일을 찾을 수 없습니다"),TEXT("오류"),MB_OK);
		return;
	}

	DWORD bytesRead = 0;

	// 맵 크기 정보 읽기
	int mapWidth,mapHeight;
	ReadFile(hFile,&mapWidth,sizeof(int),&bytesRead,NULL);
	ReadFile(hFile,&mapHeight,sizeof(int),&bytesRead,NULL);

	// 타일 데이터 개수
	int tileCount = 0;
	ReadFile(hFile,&tileCount,sizeof(int),&bytesRead,NULL);

	// 임시 맵 데이터 구조체
	MapData mapData;
	mapData.width = mapWidth;
	mapData.height = mapHeight;
	mapData.tiles.resize(tileCount);

	// 타일 데이터 읽기
	ReadFile(hFile,mapData.tiles.data(),sizeof(Room) * tileCount,&bytesRead,NULL);

	// 읽은 데이터를 에디터 타일에 적용
	for(int y = 0; y < min(MAP_EDITOR_HEIGHT,mapHeight); y++) {
		for(int x = 0; x < min(MAP_EDITOR_WIDTH,mapWidth); x++) {
			int index = y * mapWidth + x;
			mapTiles[y][x].tileIndex = mapData.tiles[index].tilePos;
			mapTiles[y][x].type = mapData.tiles[index].roomType;
		}
	}

	// 스프라이트 초기화
	editorSprites.clear();

	// 아이템 개수 읽기
	int itemCount = 0;
	ReadFile(hFile,&itemCount,sizeof(int),&bytesRead,NULL);

	// 아이템 위치 정보 읽기
	Texture* itemTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/jewel.bmp"));
	for(int i = 0; i < itemCount; i++) {
		FPOINT pos;
		ReadFile(hFile,&pos,sizeof(FPOINT),&bytesRead,NULL);

		if(itemTexture) {
			AddSprite(pos,itemTexture,SpriteType::KEY);
		}
	}

	// 몬스터 개수 읽기
	int monsterCount = 0;
	ReadFile(hFile,&monsterCount,sizeof(int),&bytesRead,NULL);

	// 몬스터 위치 정보 읽기
	Texture* monsterTexture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));
	for(int i = 0; i < monsterCount; i++) {
		FPOINT pos;
		ReadFile(hFile,&pos,sizeof(FPOINT),&bytesRead,NULL);

		if(monsterTexture) {
			AddSprite(pos,monsterTexture,SpriteType::MONSTER);
		}
	}

	CloseHandle(hFile);

	// MapManager에 맵 데이터 전달
	MapData* managerMapData = MapManager::GetInstance()->GetMapData();
	managerMapData->width = mapData.width;
	managerMapData->height = mapData.height;
	managerMapData->tiles.resize(mapData.width * mapData.height);

	// 중요: texture 관련 정보 직접 설정 (파일에서 로드되지 않음)
	managerMapData->texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/horrorMapTiles.bmp"));
	managerMapData->textureTileSize = TILE_SIZE;
	managerMapData->textureTileRowSize = SAMPLE_TILE_X;
	managerMapData->textureTileColumnSize = SAMPLE_TILE_Y;

	for(int i = 0; i < mapData.width * mapData.height; i++) {
		managerMapData->tiles[i] = mapData.tiles[i];
	}

	MessageBox(g_hWnd,TEXT("맵 로드 완료"),TEXT("알림"),MB_OK);
}