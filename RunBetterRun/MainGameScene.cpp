#include "MainGameScene.h"
#include "RayCast.h"
#include "Player.h"
#include "MonsterManager.h"
#include "SpriteManager.h"
#include "ItemManager.h"


HRESULT MainGameScene::Init()
{
	rayCasting = new RayCast();
	if (FAILED(rayCasting->Init()))
	{
		MessageBox(g_hWnd, TEXT("error"), TEXT("error"), MB_OK);
		return E_FAIL;
	}

	MapManager::GetInstance()->Init();
	Player::GetInstance()->Init([&](float shakePower, float time, bool isStepShake) { ShakeScreen(shakePower, time, isStepShake); });
	MonsterManager::GetInstance()->Init();
	ItemManager::GetInstance()->Init();

	status = SceneStatus::IN_GAME;
	ShowCursor(FALSE);

	RECT rc;
	GetClientRect(g_hWnd, &rc);

	HDC screenDC = GetDC(g_hWnd);
	backBufferDC = CreateCompatibleDC(screenDC);
	backBufferBitmap = CreateCompatibleBitmap(screenDC, rc.right, rc.bottom);
	oldBitmap = (HBITMAP)SelectObject(backBufferDC, backBufferBitmap);
	ReleaseDC(g_hWnd, screenDC);

	ItemManager::GetInstance()->PutItem({ 21.5, 10.5 });


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
		SpriteManager::GetInstance()->SortSpritesByDistance();
		MonsterManager::GetInstance()->Update();
		ItemManager::GetInstance()->Update();
		//UIManager::GetInstance()->Update();

		break;
	case MainGameScene::SceneStatus::PAUSE:
		if (KeyManager::GetInstance()->IsOnceKeyDown(VK_ESCAPE)) {
			ShowCursor(FALSE);
			status = SceneStatus::IN_GAME;
		}

		//UIManager::GetInstance()->PauseUpdate();

		break;
	case MainGameScene::SceneStatus::QUIT:
		break;
	default:
		break;
	}
}

void MainGameScene::Render(HDC hdc)
{
	rayCasting->Render(backBufferDC);

	AddShake(hdc);
}

void MainGameScene::ShakeScreen(float shakePower, float time, bool isStepShake)
{
	this->isStepShake = isStepShake;
	maxShakePower = shakePower;

	if (isStepShake)
	{
		shakeY = shakePower;
	}
	else
	{
		shakeX = ((rand() % 3) - 1) * shakePower;
		shakeY = ((rand() % 3) - 1) * shakePower;
	}

	shakeTime = time;
	elapsedTime = 0.0f;
}

void MainGameScene::AddShake(HDC hdc)
{
	int offsetX = static_cast<int>(shakeX);
	int offsetY = static_cast<int>(shakeY);

	BitBlt(hdc, offsetX, offsetY, WINSIZE_X, WINSIZE_Y, backBufferDC, 0, 0, SRCCOPY);

	if (shakeTime > 0.0f)
	{
		float dt = TimerManager::GetInstance()->GetDeltaTime();
		elapsedTime += dt;

		if (elapsedTime >= shakeTime)
		{
			shakeX = 0.0f;
			shakeY = 0.0f;
			shakeTime = 0.0f;
			elapsedTime = 0.0f;
		}
		else
		{
			float progress = elapsedTime / shakeTime;
			float damping = 1.0f - progress;

			if (isStepShake)
			{
				shakeX = 0.0f;
				shakeY = ((rand() % 3) - 1) * damping * maxShakePower;
			}
			else
			{
				shakeX = ((rand() % 3) - 1) * damping * maxShakePower;
				shakeY = ((rand() % 3) - 1) * damping * maxShakePower;
			}
		}
	}
}