#pragma once

#include "GameObject.h"

class Image;
class LossLifeScene : public GameObject
{
private:
	Image* bg;
	float delayTime;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;
};

