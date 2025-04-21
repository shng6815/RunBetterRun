#pragma once
#include "GameObject.h"

class LoadingScene: public GameObject
{
private:
	float loadingTime;
	float minLoadingTime;
	bool isLoadingComplete;

	// 로딩 애니메이션 관련 변수
	float rotationAngle;  // 회전 각도
	float pulseScale;     // 맥동 효과 크기
	float pulseDirection; // 맥동 방향

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void SetLoadingComplete() {
		isLoadingComplete = true;
	}
	bool IsMinTimeElapsed() const {
		return loadingTime >= minLoadingTime;
	}

	// 유틸리티 함수
	void DrawLoadingCircle(HDC hdc,int centerX,int centerY,int radius);
	void DrawBloodEffect(HDC hdc);
	void DrawLoadingVisual(HDC hdc,int centerX,int centerY,int yOffset);

	virtual ~LoadingScene() {};
};