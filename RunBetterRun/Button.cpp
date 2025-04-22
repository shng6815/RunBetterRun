#include "Button.h"
#include "Image.h"

HRESULT Button::Init(int posX, int posY)
{
	state = ButtonState::None;
	image = ImageManager::GetInstance()->
		AddImage("버튼",L"Image/LoadButton.bmp",122,62,1,2);
	if(image == nullptr)
	{
		return E_FAIL;
	}

	pos.x = posX;
	pos.y = posY;

	rc.left = pos.x - image->GetFrameWidth() / 2;
	rc.top = pos.y - image->GetFrameHeight() / 2;
	rc.right = rc.left + image->GetFrameWidth();
	rc.bottom = rc.top + image->GetFrameHeight();

	buttonFunc = nullptr;
	useTextOnly = false;
	text = nullptr;

	return S_OK;
}

HRESULT Button::InitTextButton(int posX, int posY, int width, int height, LPCWSTR buttonText)
{
	state = ButtonState::None;
	image = nullptr;
	useTextOnly = true;
	text = buttonText;

	pos.x = posX + width / 2;
	pos.y = posY + height / 2;

	rc.left = posX;
	rc.top = posY;
	rc.right = rc.left + width;
	rc.bottom = rc.top + height;

	buttonFunc = nullptr;

	return S_OK;
}

void Button::Release()
{
}

void Button::Update()
{
	if (PtInRect(&rc, g_ptMouse))
	{
		if (KeyManager::GetInstance()->IsOnceKeyDown(VK_LBUTTON))
		{
			state = ButtonState::Down;
		}
		else if (state == ButtonState::Down &&
			KeyManager::GetInstance()->IsOnceKeyUp(VK_LBUTTON))
		{
			state = ButtonState::Up;

			if (buttonFunc) buttonFunc();
		}
	}
	else
	{
		state = ButtonState::None;
	}
}

void Button::Render(HDC hdc)
{
	if (useTextOnly)
    {
		//텍스트 버튼
        COLORREF bgColor;
        switch (state)
        {
        case ButtonState::None:
            bgColor = RGB(50, 50, 50);
            break;
        case ButtonState::Down:
            bgColor = RGB(120, 120, 120);
            break;
        case ButtonState::Up:
            bgColor = RGB(80, 80, 80);
            break;
        }
        
        HBRUSH hBrush = CreateSolidBrush(bgColor);
        HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 10, 10);
        
        SetTextColor(hdc, RGB(255, 255, 255));
        SetBkMode(hdc, TRANSPARENT);
        
        HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"));
        HFONT oldFont = (HFONT)SelectObject(hdc, hFont);
        
        DrawText(hdc, text, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        
        SelectObject(hdc, oldFont);
        DeleteObject(hFont);
        SelectObject(hdc, oldBrush);
        DeleteObject(hBrush);
    }
    else if (image)
    {	//이미지 버튼
        switch (state)
        {
        case ButtonState::None:
        case ButtonState::Up:
            image->FrameRender(hdc, pos.x, pos.y, 0, 0);
            break;
        case ButtonState::Down:
            image->FrameRender(hdc, pos.x, pos.y, 0, 1);
            break;
        }
    }
}