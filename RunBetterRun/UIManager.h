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
	void SetActiveUIUnit(string str, bool isActive);
	void ToggleActiveUIUnit(string str);

	UIUnit* GetUIUnit(string str)
	{
		auto it = uiUnits.find(str);
		if(it != uiUnits.end())
			return it->second;
		return nullptr;
	}

private:
	map<string, UIUnit*> uiUnits;
	UIType uiType;
};

