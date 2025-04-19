#pragma once

#include "GameObject.h"
class DeadScene : public GameObject
{

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

};

