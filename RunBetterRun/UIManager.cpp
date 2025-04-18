#include "UIManager.h"
#include "UIUnit.h"

HRESULT UIManager::Init()
{
	uiType = UIType::NONE;
    return S_OK;
}

void UIManager::Update()
{
	for (auto& uiUnit : uiUnits)
	{
		if (uiUnit.second->GetUIType() == uiType)
		{
			uiUnit.second->Update();
		}
	}
}

void UIManager::Render(HDC hdc)
{
	for (auto& uiUnit : uiUnits)
	{
		if (uiUnit.second->GetUIType() == uiType)
		{
			uiUnit.second->Render(hdc);
		}
	}
}

void UIManager::Release()
{
	for (auto& uiUnit : uiUnits)
	{
		if (uiUnit.second)
		{
			uiUnit.second->Release();
			delete uiUnit.second;
		}
	}
	uiUnits.clear();
}

void UIManager::AddUIUnit(string str, UIUnit* uiUnit)
{
	if (uiUnit)
	{
		uiUnits.insert(make_pair(str, uiUnit));
	}
	else
	{
		cout << "UIUnit is nullptr" << endl;
	}
}

void UIManager::RemoveUIUnit(string str, UIUnit* uiUnit)
{
	if (uiUnit)
	{
		auto it = uiUnits.find(str);
		if (it != uiUnits.end())
		{
			it->second->Release();
			delete it->second;
			uiUnits.erase(it);
		}
	}
	else
	{
		cout << "UIUnit is nullptr" << endl;
	}
}
