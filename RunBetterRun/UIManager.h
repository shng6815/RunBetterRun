#pragma once
#include "Singleton.h"
#include "config.h"
#include "structs.h"

class UIUnit;
class UIManager : public Singleton<UIManager>
{
public:
	HRESULT Init();
	void Update();
	void Render(HDC hdc);
	void Release();

	void AddUIUnit(string str, UIUnit* uiUnit);
	void RemoveUIUnit(string str, UIUnit* uiUnit);
	void ChangeUIType(UIType type) { uiType = type; }

private:
	map<string, UIUnit*> uiUnits;
	UIType uiType;
};

