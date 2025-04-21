#include "LossLifeScene.h"
#include "ImageManager.h"
#include "SceneManager.h"
#include "TimerManager.h"
#include "VideoManager.h"
#include "Image.h";
#include "Player.h"
#include <mfapi.h>

HRESULT LossLifeScene::Init()
{
	bg = ImageManager::GetInstance()->AddImage("LossLifeImage",
	   L"horrorloading/horror_loading.bmp",WINSIZE_X,WINSIZE_Y);

	delayTime = 0.0f;
	videoTimer = 0.0f;

	//// 비디오 윈도우 클래스 등록
	//WNDCLASS wc = {0};
	//wc.lpfnWndProc = LossLifeWndProc;
	//wc.hInstance = GetModuleHandle(NULL);
	//wc.lpszClassName = L"LossLifeWindow";
	//RegisterClass(&wc);

	//RECT mainRect;
	//GetWindowRect(g_hWnd,&mainRect);

	//// 비디오 재생용 새 윈도우 생성
	//LossLifeWindow = CreateWindow(
	//	L"LossLifeWindow",L"LossLifeVideo",
	//	WS_POPUP | WS_VISIBLE,
	//	mainRect.left,mainRect.top,
	//	mainRect.right - mainRect.left,
	//	mainRect.bottom - mainRect.top,
	//	g_hWnd,NULL,GetModuleHandle(NULL),NULL);

	//if(!LossLifeWindow) {
	//	return E_FAIL;
	//}

	//int playerLife = Player::GetInstance()->GetPlayerLife();
	//switch(playerLife)
	//{
	//case 2:
	//// 새 윈도우에 비디오 플레이어 생성
	//	MFPCreateMediaPlayer(
	//		L"SceneAnimation/3Life2.mp4",
	//		FALSE,
	//		MFP_OPTION_NONE,
	//		NULL,
	//		LossLifeWindow,
	//		&pPlayer
	//	);
	//	videoLength = 8.0f;
	//	break;

	//case 1:
	//	MFPCreateMediaPlayer(
	//		L"SceneAnimation/2life1.mp4",
	//		FALSE,
	//		MFP_OPTION_NONE,
	//		NULL,
	//		LossLifeWindow,
	//		&pPlayer
	//	);
	//	videoLength = 7.0f;
	//	break;
	//}

	//if(pPlayer) {
	//	pPlayer->Play();
	//}

	//return S_OK;

	int playerLife = Player::GetInstance()->GetPlayerLife();
	const wchar_t* videoPath = nullptr;
	float videoLength = 0.0f;

	switch(playerLife)
	{
	case 2:
		videoPath = L"SceneAnimation/3Life2.mp4";
		videoLength = 8.0f;
		break;
	case 1:
		videoPath = L"SceneAnimation/2life1.mp4";
		videoLength = 7.0f;
		break;
	default:
		// 기본 비디오 설정
		videoPath = L"SceneAnimation/loss_life.mp4";
		videoLength = 7.0f;
	}

	// 비디오 재생 시작
	if(!VideoManager::PlayVideo(videoPath,videoLength)) {
		// 실패 시 바로 다음 씬으로
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}

	return S_OK;
}

void LossLifeScene::Release()
{
	//// 비디오 플레이어 정리
	//if(pPlayer) {
	//	pPlayer->Stop();         // 재생 중지
	//	pPlayer->Shutdown();     // 플레이어 종료
	//	pPlayer->Release();      // 객체 해제
	//	pPlayer = nullptr;       // 포인터 초기화
	//}

	//MFShutdown();
}

void LossLifeScene::Update()
{

	//if(pPlayer) {
	//	videoTimer += TimerManager::GetInstance()->GetDeltaTime();

	//	if(videoTimer >= videoLength) {
	//		
	//		// 비디오 윈도우 닫기
	//		if(LossLifeWindow && IsWindow(LossLifeWindow)) {
	//			DestroyWindow(LossLifeWindow);
	//			LossLifeWindow = NULL;
	//		}

	//		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	//	}
	//} else {
	//	// 비디오 재생 실패한 경우 바로 전환
	//	SceneManager::GetInstance()->ChangeScene("MainGameScene");
	//}

	// 비디오 상태 업데이트
	VideoManager::Update();

	// 비디오 종료 확인
	if(VideoManager::IsFinished()) {
		SceneManager::GetInstance()->ChangeScene("MainGameScene");
	}
}

void LossLifeScene::Render(HDC hdc)
{
	
}
