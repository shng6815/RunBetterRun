#pragma once
#include "GameObject.h"

class Image;
class LoadingScene : public GameObject
{
private:
	Image* bg;
	float loadingTime;
	float minLoadingTime;
	bool isLoadingComplete;

public:
	virtual HRESULT Init() override;
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	void SetLoadingComplete() { isLoadingComplete = true; }
	bool IsMinTimeElapsed() const { return loadingTime >= minLoadingTime;}

	virtual ~LoadingScene() {};

};

