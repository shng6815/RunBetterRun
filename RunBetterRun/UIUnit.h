#pragma once
#include "GameObject.h"
#include "structs.h"

class UIUnit : public GameObject
{
public:
	virtual HRESULT Init() override;
	virtual HRESULT Init(UIType type, FPOINT pos, FPOINT size, INT layer);
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	UIType GetUIType() { return uiType; }
	void SetActive(bool isActive) { this->isActive = isActive; }
	virtual void ToggleActive() { isActive = !isActive; }
	void SetPosition(FPOINT newPos) {
		pos = newPos;
	}

	void SetSize(FPOINT newSize) {
		size = newSize;
	}

protected:
	UIType uiType;
	vector<UIUnit*> uiUnits;
	FPOINT pos;
	FPOINT size;
	INT layer;

	bool isActive;
};