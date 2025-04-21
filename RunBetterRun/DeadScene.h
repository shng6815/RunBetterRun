#pragma once

#include "GameObject.h"
#include <mfplay.h>
class Image;
class DeadScene : public GameObject
{
	Image* bg;
	float delayTime;

	IMFPMediaPlayer*  pPlayer     = nullptr;
	float videoTimer;  // 비디오 재생 시간 체크용
	float videoLength;
public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

};

