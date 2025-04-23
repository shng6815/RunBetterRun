#include "MainGameScene.h"
#include "RayCast.h"
#include "Player.h"
#include "MonsterManager.h"
#include "SpriteManager.h"
#include "ItemManager.h"
#include "UIManager.h"
#include "Key.h"
#include "Tentacle.h"
#include "ObstacleManager.h"
#include "Pile.h"
#include "DataManager.h"
#include "SoundManager.h"
#include "Display.h"
#include "Elevator.h"
#include "Phone.h"
#include "Stun.h"
#include "Insight.h"

HRESULT MainGameScene::Init()
{
	rayCasting = new RayCast();
	if (FAILED(rayCasting->Init()))
	{
		MessageBox(g_hWnd, TEXT("error"), TEXT("error"), MB_OK);
		return E_FAIL;
	}

	MapManager::GetInstance()->Init(L"Map/EditorMap.dat");
	SpriteManager::GetInstance()->Init();
	Player::GetInstance()->Init([&](float shakePower, float time, bool isStepShake) { ShakeScreen(shakePower, time, isStepShake); });
	MonsterManager::GetInstance()->Init();
	ItemManager::GetInstance()->Init();
	ObstacleManager::GetInstance()->Init();

	const auto& items = DataManager::GetInstance()->GetItems();
	for(const auto& itemData : items)
	{
		Key* key = new Key(itemData.pos);
		key->SetAnimInfo(itemData.aniInfo);
		ItemManager::GetInstance()->PutItem(key);
	}
	const auto& monsters = DataManager::GetInstance()->GetMonsters();
	for(const auto& monsterData : monsters)
	{
		Tentacle* tentacle = new Tentacle(monsterData.pos);
		tentacle->SetAnimInfo(monsterData.aniInfo);
		MonsterManager::GetInstance()->PutMonster(tentacle);
	}
	const auto& obstacles = DataManager::GetInstance()->GetObstacles();
	for(const auto& obstacleData : obstacles)
	{
		Pile* pile = new Pile(obstacleData.pos,obstacleData.dir);
		ObstacleManager::GetInstance()->PutObstacle(pile);
	}

	UIManager::GetInstance()->Init();
	UIManager::GetInstance()->ChangeUIType(UIType::PLAYING);

	status = SceneStatus::IN_GAME;
	while(ShowCursor(FALSE) >= 0);

	RECT rc;
	GetClientRect(g_hWnd, &rc);

	HDC screenDC = GetDC(g_hWnd);
	backBufferDC = CreateCompatibleDC(screenDC);
	backBufferBitmap = CreateCompatibleBitmap(screenDC, rc.right, rc.bottom);
	oldBitmap = (HBITMAP)SelectObject(backBufferDC, backBufferBitmap);
	ReleaseDC(g_hWnd, screenDC);

	SoundManager::GetInstance()->LoadMusic("GameSceneBGM","Sounds/BGM_InGame.wav");

	SoundManager::GetInstance()->PlayMusic("GameSceneBGM",true,0.5f);

	/*ItemManager::GetInstance()->PutItem(new Key({ 21.5, 10.5 }));
	MonsterManager::GetInstance()->PutMonster(new Tentacle({ 21.5, 8.5 }));
	MonsterManager::GetInstance()->PutMonster(new Tentacle({21.5,7.5}));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,21},Direction::WEST));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,19},Direction::EAST));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,20},Direction::NORTH));
	ObstacleManager::GetInstance()->PutObstacle(new Pile({21,18},Direction::SOUTH));*/

	ObstacleManager::GetInstance()->PutObstacle(new Pile({19,23},Direction::EAST));
	ObstacleManager::GetInstance()->PutObstacle(new Elevator({20, 23}, Direction::EAST));
	ItemManager::GetInstance()->PutItem(new Display({21.5f,23.5f},TEXT("Image/drumtong.bmp")));
	ItemManager::GetInstance()->PutItem(new Display({22.5f,23.5f},TEXT("Image/pipe.bmp")));
	ItemManager::GetInstance()->PutItem(new Display({23.5f,23.5f},TEXT("Image/trash.bmp")));
	ItemManager::GetInstance()->PutItem(new Display({24.5f,23.5f},TEXT("Image/poo.bmp")));
	ItemManager::GetInstance()->PutItem(new Display({25.5f,23.5f},TEXT("Image/sohwa.bmp")));
	ItemManager::GetInstance()->PutItem(new Stun({26.5f,23.5f}));
	ItemManager::GetInstance()->PutItem(new Insight({27.5f,23.5f}));
	ItemManager::GetInstance()->PutItem(new Phone({28.5f,23.5f}));

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
		if (KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)) 
		{
			while(ShowCursor(TRUE) < 0);
			status = SceneStatus::PAUSE;
		}

		if (KeyManager::GetInstance()->IsOnceKeyDown('M'))
		{
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
		if (KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)
			) {
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