#include "MainGameScene.h"
#include "RayCasting.h"
#include "Player.h"

HRESULT MainGameScene::Init()
{
	rayCasting = new RayCasting();
	if (FAILED(rayCasting->Init()))
	{
		MessageBox(g_hWnd, TEXT("RayCasting 초기화 실패"), TEXT("경고"), MB_OK);
		return E_FAIL;
	}

	//MapManager::GetInstance()->Init();
	Player::GetInstance()->Init();
	//ItemManager::GetInstance()->Init();
	//MonsterManager::GetInstance()->Init();

	status = SceneStatus::IN_GAME;
	ShowCursor(FALSE);

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
	if (rayCasting)
		rayCasting->Render(hdc);
	
	//UIManager::GetInstance()->Render(hdc);
}
