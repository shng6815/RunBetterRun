#include "MainGame.h"
#include "CommonFunction.h"
#include "Image.h"
#include "Timer.h"
#include "LoadingScene.h"
#include "RayCast.h"
#include "SpriteManager.h"
#include "MapManager.h"
#include "OpeningScene.h"
#include "MainGameScene.h"
#include "GameStartScene.h"
#include "JumpscareScene.h"
#include "LossLifeScene.h"
#include "DeadScene.h"
#include "VideoManager.h"
#include "DataManager.h"
#include "MapEditor.h"
#include "SoundManager.h"

HRESULT MainGame::Init()
{
	ImageManager::GetInstance()->Init();
	KeyManager::GetInstance()->Init();
	SceneManager::GetInstance()->Init();
	MapManager::GetInstance()->Init(L"Map/SavedMap.dat");

	SoundManager::GetInstance()->Init();

	if(FAILED(SoundManager::GetInstance()->LoadMusic("BGM","Sounds/bgm_main.wav")))
	{
		MessageBox(g_hWnd,TEXT("BGM Load Failed"),TEXT("Error"),MB_OK);
		return E_FAIL;
	}

	if(FAILED(SoundManager::GetInstance()->LoadSound("Step","Sounds/SFX_Step.wav")))
	{
		MessageBox(g_hWnd,TEXT("SFX Load Failed"),TEXT("Error"),MB_OK);
		return E_FAIL;
	}

	SoundManager::GetInstance()->PlayMusic("BGM",true,0.8f);

	VideoManager::Init();
	DataManager::GetInstance()->Init();	
	MapManager::GetInstance()->Init(L"Map/EditorMap.dat");
	SceneManager::GetInstance()->AddScene("MapEditorScene",new MapEditor());
	SceneManager::GetInstance()->AddScene("OpeningScene",new OpeningScene());
	SceneManager::GetInstance()->AddScene("MainGameScene",new MainGameScene());
	SceneManager::GetInstance()->AddScene("GameStartScene",new GameStartScene());
	SceneManager::GetInstance()->AddScene("LossLifeScene",new LossLifeScene());
	SceneManager::GetInstance()->AddScene("JumpscareScene",new JumpscareScene());
	SceneManager::GetInstance()->AddScene("DeadScene",new DeadScene());
	SceneManager::GetInstance()->AddLoadingScene("LoadingScene",new LoadingScene());
	SceneManager::GetInstance()->ChangeScene("GameStartScene");

	hdc = GetDC(g_hWnd);

	backBuffer = new Image();
	if (FAILED(backBuffer->Init(TILEMAPTOOL_X, TILEMAPTOOL_Y)))
	{
		MessageBox(g_hWnd,
			TEXT("백버퍼 생성 실패"), TEXT("경고"), MB_OK);
		return E_FAIL;
	}

	return S_OK;
}

void MainGame::Release()
{
	if (backBuffer)
	{
		backBuffer->Release();
		delete backBuffer;
		backBuffer = nullptr;
	}

	ReleaseDC(g_hWnd, hdc);
	MapManager::GetInstance()->Release();
	SpriteManager::GetInstance()->Release();
	DataManager::GetInstance()->Release();
	SceneManager::GetInstance()->Release();
	KeyManager::GetInstance()->Release();
	VideoManager::Release();
	ImageManager::GetInstance()->Release();
	MapManager::GetInstance()->Release();
	SoundManager::GetInstance()->Release();
}

void MainGame::Update()
{
	SceneManager::GetInstance()->Update();
	InvalidateRect(g_hWnd,NULL,false);
}

void MainGame::Render()
{
	// 백버퍼에 먼저 복사
	HDC hBackBufferDC = backBuffer->GetMemDC();

	SceneManager::GetInstance()->Render(hBackBufferDC);

	TimerManager::GetInstance()->Render(hBackBufferDC);
	/*wsprintf(szText, TEXT("Mouse X : %d, Y : %d"), g_ptMouse.x, g_ptMouse.y);
	TextOut(hBackBufferDC, 20, 60, szText, wcslen(szText));*/

	// 백버퍼에 있는 내용을 메인 hdc에 복사
	backBuffer->Render(hdc);
}

LRESULT MainGame::MainProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_KEYDOWN:
		break;
	case WM_LBUTTONDOWN:
		g_ptMouse.x = LOWORD(lParam);
		g_ptMouse.y = HIWORD(lParam);
		break;
	case WM_LBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		g_ptMouse.x = LOWORD(lParam);
		g_ptMouse.y = HIWORD(lParam);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	}

	return DefWindowProc(hWnd, iMessage, wParam, lParam);
}

MainGame::MainGame()
{
}

MainGame::~MainGame()
{
}
