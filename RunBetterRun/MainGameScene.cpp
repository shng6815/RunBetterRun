#include "MainGameScene.h"
#include "RayCast.h"
#include "Player.h"
#include "SpriteManager.h"

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

	status = SceneStatus::IN_GAME;
	ShowCursor(FALSE);

	RECT rc;
	GetClientRect(g_hWnd, &rc);

	HDC screenDC = GetDC(g_hWnd);
	backBufferDC = CreateCompatibleDC(screenDC);
	backBufferBitmap = CreateCompatibleBitmap(screenDC, rc.right, rc.bottom);
	oldBitmap = (HBITMAP)SelectObject(backBufferDC, backBufferBitmap);
	ReleaseDC(g_hWnd, screenDC);

	SpriteManager::GetInstance()->PutSprite(TEXT("Image/rocket.bmp"), { 19, 12 });
	SpriteManager::GetInstance()->PutSprite(TEXT("Image/rocket.bmp"), { 16, 12 });

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
	SpriteManager::GetInstance()->Release();
	//MapManager::GetInstance()->Release();

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
		//ItemManager::GetInstance()->Update();
		//MonsterManager::GetInstance()->Update();
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
	// 1. 모든 렌더링을 백 버퍼에
	rayCasting->Render(backBufferDC);

	// 2. 흔들림 반영하여 백 버퍼를 실제 hdc에 출력
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
	// 흔들림 좌표 계산
	int offsetX = static_cast<int>(shakeX);
	int offsetY = static_cast<int>(shakeY);

	// 백 버퍼 내용을 흔들림을 적용해 출력
	BitBlt(hdc, offsetX, offsetY, WINSIZE_X, WINSIZE_Y, backBufferDC, 0, 0, SRCCOPY);

	// 흔들림 지속시간 갱신
	if (shakeTime > 0.0f)
	{
		float dt = TimerManager::GetInstance()->GetDeltaTime();
		elapsedTime += dt;

		// 흔들림 종료
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
				// stepShake면 Y축만 흔들기
				shakeX = 0.0f;
				shakeY = ((rand() % 3) - 1) * damping * maxShakePower;
			}
			else
			{
				// 일반 랜덤 흔들림
				shakeX = ((rand() % 3) - 1) * damping * maxShakePower;
				shakeY = ((rand() % 3) - 1) * damping * maxShakePower;
			}
		}
	}
}