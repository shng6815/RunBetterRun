#pragma once

#include "GameObject.h"
#include <mfplay.h>

enum class FADESTATE
{
	FADE_IN,
	DISPLAY,
	FADE_OUT
};

class Image;
class DeadScene : public GameObject
{
	Image* bg;
	IMFPMediaPlayer*  pPlayer     = nullptr;

	float displayDeadTime;
	float minDeadTime;

	float fadeAlpha;
	FADESTATE fadeState;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

};

