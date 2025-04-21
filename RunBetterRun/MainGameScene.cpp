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

	MapManager::GetInstance()->Init();
	SpriteManager::GetInstance()->Init();
	Player::GetInstance()->Init([&](float shakePower, float time, bool isStepShake) { ShakeScreen(shakePower, time, isStepShake); });
	MonsterManager::GetInstance()->Init();
	ItemManager::GetInstance()->Init();
	ObstacleManager::GetInstance()->Init();

	UIManager::GetInstance()->Init();
	UIManager::GetInstance()->ChangeUIType(UIType::PLAYING);
	PhoneUI* uiUnit = new PhoneUI();
	uiUnit->Init(UIType::PLAYING, FPOINT{ 200, WINSIZE_Y - 200 }, FPOINT{ 150, 150 }, 0);
	UIManager::GetInstance()->AddUIUnit("PhoneUI", uiUnit);

	status = SceneStatus::IN_GAME;
	ShowCursor(FALSE);

	RECT rc;
	GetClientRect(g_hWnd, &rc);

	HDC screenDC = GetDC(g_hWnd);
	backBufferDC = CreateCompatibleDC(screenDC);
	backBufferBitmap = CreateCompatibleBitmap(screenDC, rc.right, rc.bottom);
	oldBitmap = (HBITMAP)SelectObject(backBufferDC, backBufferBitmap);
	ReleaseDC(g_hWnd, screenDC);

	ItemManager::GetInstance()->PutItem(new Key({ 21.5, 10.5 }));
	MonsterManager::GetInstance()->PutMonster(new Tentacle({ 21.5, 8.5 }));
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
			ShowCursor(TRUE);
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
		//UIManager::GetInstance()->Update();
		ObstacleManager::GetInstance()->Update();
		break;
	case MainGameScene::SceneStatus::PAUSE:
		if (KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)) {
			ShowCursor(FALSE);
			status = SceneStatus::IN_GAME;
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