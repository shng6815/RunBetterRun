#pragma once
#include "config.h"
#include "UIUnit.h"

class PhoneUI : public UIUnit
{
public:
	HRESULT Init(UIType type, FPOINT pos, FPOINT size, INT layer) override;
	void Release() override;
	void Update() override;
	void Render(HDC hdc) override;
};