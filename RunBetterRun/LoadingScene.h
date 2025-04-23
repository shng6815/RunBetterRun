#pragma once
#include "GameObject.h"

class Image;
class LoadingScene: public GameObject
{
private:
	float loadingTime;
	float minLoadingTime;
	bool isLoadingComplete;

	Image* loadingImage;    // 로딩 애니메이션 이미지
	int frameCount;         // 로딩 애니메이션 프레임 수
	float frameTime;        // 프레임 변경 타이머
	float frameDelay;       // 프레임 간 지연 시간
	int currentFrame;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void SetLoadingComplete() { isLoadingComplete = true; }


	virtual ~LoadingScene() {};
};