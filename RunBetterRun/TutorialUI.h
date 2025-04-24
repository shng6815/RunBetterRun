#pragma once
#include "UIUnit.h"

class TutorialUI: public UIUnit
{
public:
	HRESULT Init(UIType type,FPOINT pos,FPOINT size,INT layer) override;
	void Release() override;
	void Update() override;
	void Render(HDC hdc) override;
	virtual void ToggleActive() override;

private:
	Image* tutorialImage;
	INT frameIDX;
	bool isUp;
	bool isDone;
};

