#pragma once

#include "GameObject.h"
#include <mfPlay.h>

class OpeningScene : public GameObject
{
private:
	IMFPMediaPlayer*  pPlayer     = nullptr;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;
};

