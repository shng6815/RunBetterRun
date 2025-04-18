#include "UIUnit.h"

HRESULT UIUnit::Init()
{
    return E_NOTIMPL;
}

HRESULT UIUnit::Init(UIType type, FPOINT pos, FPOINT size, INT layer)
{
	this->uiType = type;
	this->pos = pos;
	this->size = size;
	this->layer = layer;

    return S_OK;
}

void UIUnit::Release()
{
}

void UIUnit::Update()
{
}

void UIUnit::Render(HDC hdc)
{
	Rectangle(hdc, pos.x, pos.y, pos.x + size.x, pos.y + size.y);
}
