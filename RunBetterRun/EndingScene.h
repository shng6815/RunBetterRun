#pragma once

#include "GameObject.h"
#include <mfplay.h>

class EndingScene : public GameObject
{
private:
	IMFPMediaPlayer*  pPlayer     = nullptr;

	float displayDeadTime;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;
};

