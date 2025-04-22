#include "MainGameScene.h"
#include "RayCast.h"
#include "Player.h"
#include "MonsterManager.h"
#include "SpriteManager.h"
#include "ItemManager.h"
#include "UIManager.h"
#include "PhoneUI.h"
#include "Key.h"
#include "Tentacle.h"
#include "ObstacleManager.h"
#include "Pile.h"

HRESULT MainGameScene::Init()
{
	rayCasting = new RayCast();
	if (FAILED(rayCasting->Init()))
	{
		MessageBox(g_hWnd, TEXT("error"), TEXT("error"), MB_OK);
		return E_FAIL;
	}

	MapManager::GetInstance()->Init(L"Map/EditorMap.dat");
	LoadMapItems(L"Map/EditorMap.dat");
	SpriteManager::GetInstance()->Init();
	Player::GetInstance()->Init([&](float shakePower, float time, bool isStepShake) { ShakeScreen(shakePower, time, isStepShake); });
	MonsterManager::GetInstance()->Init();
	ItemManager::GetInstance()->Init();
	ObstacleManager::GetInstance()->Init();

	UIManager::GetInstance()->Init();
	UIManager::GetInstance()->ChangeUIType(UIType::PLAYING);
	PhoneUI* uiUnit = new PhoneUI();
	uiUnit->Init(UIType::PLAYING, FPOINT{ 100, WINSIZE_Y - 500 }, FPOINT{ 300, 400 }, 0);
	UIManager::GetInstance()->AddUIUnit("PhoneUI", uiUnit);

	status = SceneStatus::IN_GAME;
	while(ShowCursor(FALSE) >= 0);

	RECT rc;
	GetClientRect(g_hWnd, &rc);

	HDC screenDC = GetDC(g_hWnd);
	backBufferDC = CreateCompatibleDC(screenDC);
	backBufferBitmap = CreateCompatibleBitmap(screenDC, rc.right, rc.bottom);
	oldBitmap = (HBITMAP)SelectObject(backBufferDC, backBufferBitmap);
	ReleaseDC(g_hWnd, screenDC);

	ItemManager::GetInstance()->PutItem(new Key({ 21.5, 10.5 }));
	MonsterManager::GetInstance()->PutMonster(new Tentacle({ 21.5, 8.5 }));
	MonsterManager::GetInstance()->PutMonster(new Tentacle({21.5,7.5}));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,21},Direction::WEST));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,19},Direction::EAST));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,20},Direction::NORTH));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,18},Direction::SOUTH));

	return S_OK;
}

HRESULT MainGameScene::Init(LPCWCH)
{
	return E_NOTIMPL;
}

void MainGameScene::Release()
{
	if (rayCasting)
	{
		rayCasting->Release();
		delete rayCasting;
		rayCasting = nullptr;
	}
	ItemManager::GetInstance()->Release();
	MonsterManager::GetInstance()->Release();
	Player::GetInstance()->Release();
	SpriteManager::GetInstance()->Release();
	MapManager::GetInstance()->Release();

	UIManager::GetInstance()->Release();

	SelectObject(backBufferDC, oldBitmap);
	DeleteObject(backBufferBitmap);
	DeleteDC(backBufferDC);
}

void MainGameScene::Update()
{
	switch (status)
	{
	case MainGameScene::SceneStatus::IN_GAME:
		if (KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)) {
			while(ShowCursor(TRUE) < 0);
			status = SceneStatus::PAUSE;
		}

		if (KeyManager::GetInstance()->IsOnceKeyDown('M')) {
			UIManager::GetInstance()->ToggleActiveUIUnit("PhoneUI");
		}
		
		Player::GetInstance()->Update();
		if (rayCasting)
			rayCasting->Update();
		SpriteManager::GetInstance()->SortSpritesByDistance();
		MonsterManager::GetInstance()->Update();
		ItemManager::GetInstance()->Update();
		UIManager::GetInstance()->Update();
		ObstacleManager::GetInstance()->Update();
		break;
	case MainGameScene::SceneStatus::PAUSE:
		if (KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)) {
			while(ShowCursor(FALSE) >= 0);
			status = SceneStatus::IN_GAME;
		}

		if(KeyManager::GetInstance()->IsOnceKeyDown('T'))
		{
			SceneManager::GetInstance()->ChangeScene("MapEditorScene");
		}

		break;
	case MainGameScene::SceneStatus::QUIT:
		break;
	default:
		break;
	}

	UIManager::GetInstance()->Update();
}

void MainGameScene::Render(HDC hdc)
{
	rayCasting->Render(backBufferDC);

	ApplyShake(hdc);

	UIManager::GetInstance()->Render(hdc);
}

void MainGameScene::ShakeScreen(float shakePower, float time, bool isStepShake)
{
	screenShake.Start(shakePower, time, isStepShake);
}

void MainGameScene::ApplyShake(HDC hdc)
{
	screenShake.Update(TimerManager::GetInstance()->GetDeltaTime());

	POINT offset = screenShake.GetOffset();

	BitBlt(hdc, offset.x, offset.y, WINSIZE_X, WINSIZE_Y, backBufferDC, 0, 0, SRCCOPY);
}

void MainGameScene::LoadMapItems(LPCWCH filePath)
{
	HANDLE hFile = CreateFile(
		filePath,GENERIC_READ,0,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

	if(hFile == INVALID_HANDLE_VALUE)
	{
		return;
	}

	DWORD bytesRead = 0;

	// �� ������ �ǳʶٱ�
	int mapWidth,mapHeight,tileCount;
	ReadFile(hFile,&mapWidth,sizeof(int),&bytesRead,NULL);
	ReadFile(hFile,&mapHeight,sizeof(int),&bytesRead,NULL);
	ReadFile(hFile,&tileCount,sizeof(int),&bytesRead,NULL);

	// Ÿ�� ������ �ǳʶٱ�
	SetFilePointer(hFile,sizeof(Room) * tileCount,NULL,FILE_CURRENT);

	// ������ ���� �б�
	int itemCount = 0;
	ReadFile(hFile,&itemCount,sizeof(int),&bytesRead,NULL);

	// ������ ��ġ ���� �а� ����
	for(int i = 0; i < itemCount; i++)
	{
		FPOINT pos;
		AnimationInfo aniInfo;

		ReadFile(hFile,&pos,sizeof(FPOINT),&bytesRead,NULL);
		ReadFile(hFile,&aniInfo,sizeof(AnimationInfo),&bytesRead,NULL);

		// Key ������ ����
		Key* key = new Key(pos);
		key->SetAnimInfo(aniInfo); // �ִϸ��̼� ���� ���� (Key Ŭ������ �߰� �ʿ�)
		ItemManager::GetInstance()->PutItem(key);
	}

	// ���� ���� �б�
	int monsterCount = 0;
	ReadFile(hFile,&monsterCount,sizeof(int),&bytesRead,NULL);

	// ���� ��ġ ���� �а� ����
	for(int i = 0; i < monsterCount; i++)
	{
		FPOINT pos;
		AnimationInfo aniInfo;

		ReadFile(hFile,&pos,sizeof(FPOINT),&bytesRead,NULL);
		ReadFile(hFile,&aniInfo,sizeof(AnimationInfo),&bytesRead,NULL);

		// Tentacle ���� ����
		Tentacle* tentacle = new Tentacle(pos);
		tentacle->SetAnimInfo(aniInfo); // �ִϸ��̼� ���� ���� (Tentacle Ŭ������ �߰� �ʿ�)
		MonsterManager::GetInstance()->PutMonster(tentacle);
	}

	CloseHandle(hFile);
}