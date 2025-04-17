#pragma once
#include "GameObject.h"

class RayCasting;
class MainGameScene : public GameObject
{
public:
	virtual HRESULT Init() override;
	virtual HRESULT Init(LPCWCH path);
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;
private:
	enum class SceneStatus { IN_GAME, PAUSE, QUIT };

	SceneStatus status;
	RayCasting* rayCasting;
	void LoadFile(LPCWCH path);
};

