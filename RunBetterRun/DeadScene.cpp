#include "DeadScene.h"
#include "ImageManager.h"
#include "SceneManager.h"
#include "TimerManager.h"
#include "Image.h";
#include "Player.h"
#include "VideoManager.h"
#include <mfapi.h>


HRESULT DeadScene::Init()
{
	bg = ImageManager::GetInstance()->AddImage("DeadImage",
	L"horrorloading/horror_loading2.bmp",WINSIZE_X,WINSIZE_Y);

	// 비디오 재생 시작
	if(!VideoManager::PlayVideo(L"SceneAnimation/Dead.mp4",10.0f)) {
		// 실패 시 바로 다음 씬으로
		Player::GetInstance()->InitPlayerLife();
		SceneManager::GetInstance()->ChangeScene("GameStartScene");
	}

	return S_OK;
}

void DeadScene::Release()
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

void DeadScene::Update()
{
	VideoManager::Update();

	// 비디오 종료 확인
	if(VideoManager::IsFinished()) {
		Player::GetInstance()->InitPlayerLife();
		SceneManager::GetInstance()->ChangeScene("GameStartScene");
	}
}


void DeadScene::Render(HDC hdc)
{

}
