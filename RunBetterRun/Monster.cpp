#include "Monster.h"

HRESULT Monster::Init(FPOINT startPos, float speed)
{
	position = startPos;
	targetPosition = startPos;
	this->speed = speed;
	isActive = true;
	isMoving = false;
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
