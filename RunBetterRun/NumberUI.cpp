#include "NumberUI.h"
#include "Image.h"
#include "ItemManager.h"

HRESULT NumberUI::Init(UIType type,FPOINT relPos,FPOINT relSize,UIUnit * parent,INT layer)
{
	UIUnit::Init(type,relPos,relSize,parent,layer);

	numberImage = ImageManager::GetInstance()->AddImage("NumberUI",
		L"Image/NumberUI.bmp",size.x,size.y,10,1,true,RGB(255,0,255));

	return S_OK;
}

void NumberUI::Release()
{}

void NumberUI::Update()
{
	UIUnit::Update();

	auto num = ItemManager::GetInstance()->Size();

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
	}
}

void NumberUI::Render(HDC hdc)
{
	if(!isActive)
		return;
	// 숫자 UI를 그리는 코드 추가
	if(numberImage)
	{
		for(size_t i = 0; i < numbers.size(); ++i)
		{
			int number = numbers[numbers.size() - 1 - i];

			numberImage->RenderResized(hdc,
				pos.x + i * (size.x / 10),
				pos.y,
				size.x / 10,
				size.y,
				number);
		}
	}
}
