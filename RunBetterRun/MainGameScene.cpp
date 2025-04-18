#include "MainGameScene.h"
#include "RayCast.h"
#include "Player.h"
#include "UIManager.h"
#include "UIUnit.h"

HRESULT MainGameScene::Init()
{
	rayCasting = new RayCast();
	if (FAILED(rayCasting->Init()))
	{
		MessageBox(g_hWnd, TEXT("RayCasting 초기화 실패"), TEXT("경고"), MB_OK);
		return E_FAIL;
	}

	//MapManager::GetInstance()->Init();
	Player::GetInstance()->Init([&](float shakePower, float time, bool isStepShake) { ShakeScreen(shakePower, time, isStepShake); });
	//ItemManager::GetInstance()->Init();
	//MonsterManager::GetInstance()->Init();

	UIManager::GetInstance()->Init();
	UIManager::GetInstance()->ChangeUIType(UIType::PLAYING);
	UIUnit* uiUnit = new UIUnit();
	uiUnit->Init(UIType::PLAYING, FPOINT{ 0, 0 }, FPOINT{ WINSIZE_X/10, WINSIZE_Y/10 }, 0);
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
	//MonsterManager::GetInstance()->Release();
	//ItemManaher::GetInstance()->Release();
	Player::GetInstance()->Release();
	//MapManager::GetInstance()->Release();

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
		
		Player::GetInstance()->Update();
		if (rayCasting)
			rayCasting->Update();
		//ItemManager::GetInstance()->Update();
		//MonsterManager::GetInstance()->Update();

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
	// 1. 모든 렌더링을 백 버퍼에
	rayCasting->Render(backBufferDC);

	// 2. 흔들림 반영하여 백 버퍼를 실제 hdc에 출력
	ApplyShake(hdc);

	// 3. UI 렌더링
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