#pragma once
#include "UIUnit.h"

class NumberUI: public UIUnit
{
public:
	HRESULT Init(UIType type,FPOINT relPos,FPOINT relSize,UIUnit* parent,INT layer) override;
	void Release() override;
	void Update() override;
	void Render(HDC hdc) override;
private:
	Image* numberImage;

	vector<int> numbers; // 숫자 배열
};