#include "Phone.h"
#include "TextureManager.h"
#include "SpriteManager.h"
#include "MainGameScene.h"

BOOL Phone::Action(void)
{
	// 메인 게임 씬의 상태를 가이드 상태로 변경
	MainGameScene* mainScene = dynamic_cast<MainGameScene*>(SceneManager::GetInstance()->currentScene);
	if(mainScene) {
		mainScene->ShowPhoneGuide(); // 이 메소드를 추가해야 함
		ObstacleManager::GetInstance()->LockElevator();
		return TRUE;
	}
	return FALSE;
}

HRESULT Phone::Init(FPOINT pos)
{
	sprite.id = 1;
	sprite.texture = TextureManager::GetInstance()->GetTexture(TEXT("Image/phone.bmp"));
	if(!sprite.texture)
		return E_FAIL;
	sprite.type = SpriteType::ITEM;
	sprite.pos = pos;
	sprite.distance = 0;
	sprite.aniInfo = {0, 0,{250,250},{1,1},{0, 0}};
	SpriteManager::GetInstance()->AddSprite(sprite);
	return S_OK;
}
