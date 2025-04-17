#include "Monster.h"

HRESULT Monster::Init(FPOINT startPos)
{
	position = startPos;
	speed = 1.0f;
	isActive = true;
	//sprite = nullptr;

	return S_OK;
}

void Monster::Release()
{
}

void Monster::Update()
{
}

void Monster::Render(HDC hdc)
{
}
