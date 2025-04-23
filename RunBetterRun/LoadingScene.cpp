#include "LoadingScene.h"
#include "TimerManager.h"
#include "SceneManager.h"
#include "Image.h"
#include <cmath>

HRESULT LoadingScene::Init()
{
	loadingTime = 0.0f;
	minLoadingTime = 3.0f;
	isLoadingComplete = false;

	loadingImage = ImageManager::GetInstance()->AddImage("LoadingAnim",
		L"horrorloading/Loading.bmp",4500,169,15,1,true,RGB(0,0,0));

	// 프레임 애니메이션 관련 변수 초기화
	frameCount = 15;     // 프레임 수
	frameTime = 0.0f;    // 현재 프레임 타이머
	frameDelay = 0.1f;   // 프레임 간 지연 시간 (0.1초)

	while(ShowCursor(FALSE)>=0);

	return S_OK;
}

void LoadingScene::Release()
{
	while(ShowCursor(TRUE) < 0);
}

void LoadingScene::Update()
{
	float deltaTime = TimerManager::GetInstance()->GetDeltaTime();

	// 로딩 경과 시간 증가
	loadingTime += deltaTime;

	frameTime += deltaTime;
	if(loadingImage)
	{
		frameTime += deltaTime;
		if(frameTime >= frameDelay)
		{
			frameTime -= frameDelay;
			currentFrame++;
			if(currentFrame >= loadingImage->GetMaxFrameX())
			{
				currentFrame = 0; 
			}
		}
	}
	
	
	// 로딩 완료 및 최소 시간 경과 확인
	if(isLoadingComplete && loadingTime >= minLoadingTime)
	{
		SceneManager::GetInstance()->LoadingComplete();
	}
}

void LoadingScene::Render(HDC hdc)
{
	RECT rc;
	GetClientRect(g_hWnd,&rc);

	// 배경 그리기
	HBRUSH bgBrush = CreateSolidBrush(RGB(0,0,0));
	FillRect(hdc,&rc,bgBrush);
	DeleteObject(bgBrush);

	// 로딩 애니메이션 그리기
	if(loadingImage)
	{
		int centerX = rc.right / 2;
		int centerY = rc.bottom / 2;

		// 프레임 렌더링 - 중앙에 위치
		loadingImage->FrameRender(hdc,centerX,centerY,currentFrame,0,false,true);
	}
}

