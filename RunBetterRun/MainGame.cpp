#include "MainGame.h"
#include "CommonFunction.h"
#include "Image.h"
#include "Timer.h"
#include "TilemapTool.h"
#include "LoadingScene.h"
#include "RayCasting.h"
#include "SpriteManager.h"
#include "MapManager.h"

HRESULT MainGame::Init()
{
	ImageManager::GetInstance()->Init();
	KeyManager::GetInstance()->Init();
	SceneManager::GetInstance()->Init();
	SpriteManager::GetInstance()->Init();
	MapManager::GetInstance()->Init();

	if (!MapManager::GetInstance()->LoadMap("defaultMap", L"Maps/defaultMap.dat"))
	{
		// 기본 맵 생성
		MapManager::GetInstance()->CreateEmptyMap("defaultMap", MAP_COLUME, MAP_ROW);

		// 여기서 기본 맵 데이터를 설정할 수 있습니다
		// 예시: 테스트 맵 구성
		for (int y = 0; y < MAP_ROW; y++) {
			for (int x = 0; x < MAP_COLUME; x++) {
				// 가장자리에 벽 배치
				if (x == 0 || y == 0 || x == MAP_COLUME - 1 || y == MAP_ROW - 1) {
					MapManager::GetInstance()->SetTile("defaultMap", x, y, 1);
				}
			}
		}

		// 몇 가지 장애물 추가
		for (int i = 5; i < 10; i++) {
			MapManager::GetInstance()->SetTile("defaultMap", i, 5, 2);
		}

		// 맵 저장
		MapManager::GetInstance()->SaveMap("defaultMap", L"Maps/defaultMap.dat");
	}

	// 현재 맵으로 설정
	MapManager::GetInstance()->SetCurrentMap("defaultMap");
	SceneManager::GetInstance()->AddScene("타일맵툴", new TilemapTool());
	SceneManager::GetInstance()->AddLoadingScene("로딩_1", new LoadingScene());
	SceneManager::GetInstance()->AddScene("RayCasting", new RayCasting());
	SceneManager::GetInstance()->ChangeScene("RayCasting");

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

	SpriteManager::GetInstance()->Release();
	SceneManager::GetInstance()->Release();
	KeyManager::GetInstance()->Release();
	ImageManager::GetInstance()->Release();
	MapManager::GetInstance()->Release();
}

void MainGame::Update()
{
	SceneManager::GetInstance()->Update();
	InvalidateRect(g_hWnd, NULL, false);
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
