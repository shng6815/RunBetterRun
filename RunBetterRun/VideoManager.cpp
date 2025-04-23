#include "VideoManager.h"
#include "TimerManager.h"
#include <mfapi.h>

HWND VideoManager::videoWindow = NULL;
IMFPMediaPlayer* VideoManager::pPlayer = nullptr;
bool VideoManager::isPlaying = false;
float VideoManager::videoTimer = 0.0f;
float VideoManager::videoLength = 0.0f;

// 비디오 윈도우 프로시저
LRESULT CALLBACK VideoWndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	return DefWindowProc(hwnd,msg,wParam,lParam);
}

void VideoManager::Init()
{
	// 비디오 윈도우 클래스 등록
	WNDCLASS wc = {0};
	wc.lpfnWndProc = VideoWndProc;
	wc.hInstance = GetModuleHandle(NULL);
	wc.lpszClassName = L"VideoWindowClass";
	RegisterClass(&wc);

	// Media Foundation 초기화
	CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
	MFStartup(MF_VERSION);
}

void VideoManager::Release()
{
	CloseVideoWindow();

	MFShutdown();
	CoUninitialize();
}

bool VideoManager::PlayVideo(const wchar_t* filePath,float length)
{
	if(isPlaying) {
		CloseVideoWindow();
	}

	// 메인 윈도우 크기 및 위치 가져오기
	RECT mainRect;
	GetWindowRect(g_hWnd,&mainRect);

	// 비디오 재생용 새 윈도우 생성
	videoWindow = CreateWindow(
		L"VideoWindowClass",L"VideoPlayer",
		WS_POPUP | WS_VISIBLE,
		mainRect.left,mainRect.top,
		mainRect.right - mainRect.left,
		mainRect.bottom - mainRect.top,
		g_hWnd,NULL,GetModuleHandle(NULL),NULL);

	if(!videoWindow) {
		return false;
	}

	// Media Foundation 플레이어 생성
	MFPCreateMediaPlayer(
		filePath,
		FALSE,
		MFP_OPTION_NONE,
		NULL,
		videoWindow,
		&pPlayer
	);

	if(!pPlayer) {
		DestroyWindow(videoWindow);
		videoWindow = NULL;
		return false;
	}

	// 재생 시작
	pPlayer->Play();

	// 상태 업데이트
	isPlaying = true;
	videoTimer = 0.0f;
	videoLength = length;

	return true;
}

void VideoManager::Update()
{
	if(!isPlaying) return;

	videoTimer += TimerManager::GetInstance()->GetDeltaTime();

	if(videoTimer >= videoLength) {
		CloseVideoWindow();
	}
}

bool VideoManager::IsFinished()
{
	return !isPlaying;
}

void VideoManager::CloseVideoWindow()
{
	if(pPlayer) {
		pPlayer->Stop();
		pPlayer->Shutdown();
		pPlayer->Release();
		pPlayer = nullptr;
	}

	if(videoWindow && IsWindow(videoWindow)) {
		DestroyWindow(videoWindow);
		videoWindow = NULL;
	}

	isPlaying = false;
}