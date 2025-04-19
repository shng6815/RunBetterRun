#include "PhoneUI.h"
#include "MinimapUI.h"


HRESULT PhoneUI::Init(UIType type, FPOINT pos, FPOINT size, INT layer)
{
	isActive = true;

	this->uiType = type;
	this->pos = pos;
	this->size = size;
	this->layer = layer;

	// Initialize the UI unit
	MinimapUI* minimapUI = new MinimapUI();
	minimapUI->Init(type,pos,{size.x - 10,size.y - 10},1);
	uiUnits.push_back(minimapUI);

    return S_OK;
}

void PhoneUI::Release()
{
	for(auto& uiUnit : uiUnits)
	{
		uiUnit->Release();
		delete uiUnit;
		uiUnit = nullptr;
	}
	uiUnits.clear();
}

void PhoneUI::Update()
{
}

void PhoneUI::Render(HDC hdc)
{
	//휴대폰 UI 배경 그리기
	HBRUSH bgBrush = CreateSolidBrush(RGB(0,0,0));
	RECT fillRect = {pos.x - 10,pos.y - 10,pos.x + size.x,pos.y + size.y};
	FillRect(hdc,&fillRect,bgBrush);

	DeleteObject(bgBrush);

	for(auto& uiUnit : uiUnits)
	{
		uiUnit->Render(hdc);
	}
}
