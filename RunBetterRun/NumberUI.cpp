#include "NumberUI.h"
#include "Image.h"
#include "ItemManager.h"

HRESULT NumberUI::Init(UIType type,FPOINT relPos,FPOINT relSize,UIUnit * parent,INT layer)
{
	UIUnit::Init(type,relPos,relSize,parent,layer);

	numberImage = ImageManager::GetInstance()->AddImage("NumberUI",
		L"Image/number80x16.bmp",size.x,size.y,10,1,true,RGB(255,0,255));

	return S_OK;
}

void NumberUI::Release()
{}

void NumberUI::Update()
{
	UIUnit::Update();

	auto num = ItemManager::GetInstance()->Key();

	if(num > 0)
	{
		numbers.clear();
		while(num > 0)
		{
			numbers.push_back(num % 10);
			num /= 10;
		}
	} else
	{
		numbers.clear();
		numbers.push_back(0);
		numbers.push_back(0);
		numbers.push_back(0);
	}
}

void NumberUI::Render(HDC hdc)
{
	if(!isActive)
		return;

	if(numberImage)
	{
		// 숫자 하나의 폭
		float digitWidth = size.x / 10;

		// 숫자들의 총 너비
		float totalWidth = numbers.size() * digitWidth;

		// UI 요소의 중앙 좌표
		float centerX = pos.x + 5;

		// 숫자들의 시작 x좌표 (중앙에서 총 너비의 절반을 왼쪽으로)
		float startX = centerX - totalWidth / 2;

		for(size_t i = 0; i < numbers.size(); ++i)
		{
			int number = numbers[numbers.size() - 1 - i];
			numberImage->RenderResized(hdc,
				startX + i * digitWidth,
				pos.y,
				digitWidth,
				size.y,
				number);
		}
	}
}
