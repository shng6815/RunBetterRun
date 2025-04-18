#include "Monster.h"
#include "TextureManager.h"

HRESULT Monster::Init(FPOINT startPos, float speed)
{
	sprite.pos = startPos;
	sprite.distance = 0;
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/boss.bmp"));
	sprite.aniInfo = { 0, 0, {423, 437}, {1, 1}, {0, 0} };
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
