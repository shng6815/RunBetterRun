#include "UIUnit.h"
#include "MapManager.h"

// 구현 예시
HRESULT UIUnit::Init()
{
	isActive = true;
	parentUI = nullptr;
	useRelativePosition = false;
	useRelativeSize = false;
	return S_OK;
}

HRESULT UIUnit::Init(UIType type,FPOINT pos,FPOINT size,INT layer)
{
	this->uiType = type;
	this->pos = pos;
	this->size = size;
	this->layer = layer;
	isActive = true;
	parentUI = nullptr;
	useRelativePosition = false;
	useRelativeSize = false;
	return S_OK;
}

HRESULT UIUnit::Init(UIType type,FPOINT relPos,FPOINT relSize,UIUnit* parent,INT layer)
{
	this->uiType = type;
	this->relativePos = relPos;
	this->relativeSize = relSize;
	this->parentUI = parent;
	this->layer = layer;
	isActive = true;
	useRelativePosition = true;
	useRelativeSize = true;

	// 초기 절대 위치 및 크기 계산
	if(parent) {
		FPOINT parentPos = parent->GetPosition();
		FPOINT parentSize = parent->GetSize();

		pos.x = parentPos.x + parentSize.x * relPos.x;
		pos.y = parentPos.y + parentSize.y * relPos.y;
		size.x = parentSize.x * relSize.x;
		size.y = parentSize.y * relSize.y;

		// 부모의 자식으로 등록
		parent->AddChild(this);
	}

	return S_OK;
}

void UIUnit::Update()
{
	if(!isActive)
		return;

	// 만약 부모 UI가 있고 상대적 위치/크기를 사용 중이라면 업데이트
	if(parentUI) {
		if(useRelativePosition)
			UpdateAbsolutePosition();
		if(useRelativeSize)
			UpdateAbsoluteSize();
	}

	// 자식 UI 유닛들 업데이트
	for(auto& uiUnit : uiUnits) {
		if(uiUnit) {
			uiUnit->Update();
		}
	}
}

void UIUnit::Release()
{
	for(auto& uiUnit : uiUnits) {
		if(uiUnit) {
			uiUnit->Release();
			delete uiUnit;
			uiUnit = nullptr;
		}
	}
	uiUnits.clear();
}

void UIUnit::Render(HDC hdc)
{
	if(!isActive)
		return;

	// 현재 UI 렌더링 코드 (필요에 따라 구현)

	// 자식 UI 유닛들 렌더링
	for(auto& uiUnit : uiUnits) {
		if(uiUnit) {
			uiUnit->Render(hdc);
		}
	}
}