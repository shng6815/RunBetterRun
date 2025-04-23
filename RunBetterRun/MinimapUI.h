#pragma once
#include "UIUnit.h"

class MinimapUI: public UIUnit
{
public:
	void Release() override;
	void Render(HDC hdc) override;

	void DrawMiniMapToDC(HDC hdc,int drawSize);
	void DrawMiniMapWithRotation(HDC hdc,int size,float angle);
};

