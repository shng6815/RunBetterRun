#pragma once
#include "UIUnit.h"

class MinimapUI: public UIUnit
{
public:
	HRESULT Init(UIType type,FPOINT pos,FPOINT size,INT layer) override;
	void Release() override;
	void Update() override;
	void Render(HDC hdc) override;

	void DrawMiniMapToDC(HDC hdc,int drawSize);
	void DrawMiniMapWithRotation(HDC hdc,int size,float angle);
};

