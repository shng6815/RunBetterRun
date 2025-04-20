#include "LossLifeScene.h"
#include "ImageManager.h"
#include "SceneManager.h"
#include "TimerManager.h"
#include "Image.h";
#include "Player.h"
#include <mfapi.h>

// 비디오 재생용 전역 변수
HWND videoWindow = NULL;

// 비디오 윈도우 프로시저
LRESULT CALLBACK VideoWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	return DefWindowProc(hwnd,msg,wParam,lParam);
}


HRESULT LossLifeScene::Init()
{
	bg = ImageManager::GetInstance()->AddImage("LossLifeImage",
	   L"horrorloading/horror_loading.bmp",WINSIZE_X,WINSIZE_Y);

	delayTime = 0.0f;
	videoTimer = 0.0f;

	// 비디오 윈도우 클래스 등록
	WNDCLASS wc = {0};
	wc.lpfnWndProc = VideoWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"VideoWindow";
	RegisterClass(&wc);

	RECT mainRect;
	GetWindowRect(g_hWnd,&mainRect);

	// 비디오 재생용 새 윈도우 생성
	videoWindow = CreateWindow(
		L"VideoWindow",L"Video",
		WS_POPUP | WS_VISIBLE,
		mainRect.left,mainRect.top,
		mainRect.right - mainRect.left,
		mainRect.bottom - mainRect.top,
		g_hWnd,NULL,GetModuleHandle(NULL),NULL);

	if(!videoWindow) {
		return E_FAIL;
	}

	int playerLife = Player::GetInstance()->GetPlayerLife();
	switch(playerLife)
	{
	case 2:
	// 새 윈도우에 비디오 플레이어 생성
		MFPCreateMediaPlayer(
			L"SceneAnimation/3Life2.mp4",
			FALSE,
			MFP_OPTION_NONE,
			NULL,
			videoWindow,
			&pPlayer
		);
		videoLength = 8.0f;
		break;

	case 1:
		MFPCreateMediaPlayer(
			L"SceneAnimation/2life1.mp4",
			FALSE,
			MFP_OPTION_NONE,
			NULL,
			videoWindow,
			&pPlayer
		);
		videoLength = 7.0f;
		break;
	}

	if(pPlayer) {
		pPlayer->Play();
	}

	return S_OK;
}

void LossLifeScene::Release()
{
	// 비디오 플레이어 정리
	if(pPlayer) {
		pPlayer->Stop();         // 재생 중지
		pPlayer->Shutdown();     // 플레이어 종료
		pPlayer->Release();      // 객체 해제
		pPlayer = nullptr;       // 포인터 초기화
	}

	MFShutdown();
}

void LossLifeScene::Update()
{

	if(pPlayer) {
		videoTimer += TimerManager::GetInstance()->GetDeltaTime();

		if(videoTimer >= videoLength) {
			
			// 비디오 윈도우 닫기
			if(videoWindow && IsWindow(videoWindow)) {
				DestroyWindow(videoWindow);
				videoWindow = NULL;
			}

			SceneManager::GetInstance()->ChangeScene("MainGameScene");
		}
	} else {
		// 비디오 재생 실패한 경우 바로 전환
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}
}

void LossLifeScene::Render(HDC hdc)
{
	
}
