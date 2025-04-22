#pragma once
#include "GameObject.h"
#include "structs.h"

class UIUnit: public GameObject
{
public:
	virtual HRESULT Init() override;
	virtual HRESULT Init(UIType type,FPOINT pos,FPOINT size,INT layer);
	virtual HRESULT Init(UIType type,FPOINT relPos,FPOINT relSize,UIUnit* parent,INT layer);
	virtual void Release() override;
	virtual void Update() override;
	virtual void Render(HDC hdc) override;

	UIType GetUIType() {
		return uiType;
	}
	void SetActive(bool isActive) {
		this->isActive = isActive;
	}
	virtual void ToggleActive() {
		isActive = !isActive;
	}

	// 절대 위치와 크기 설정
	void SetPosition(FPOINT newPos) {
		pos = newPos;
		UpdateChildrenPositions();
	}
	void SetSize(FPOINT newSize) {
		size = newSize;
		UpdateChildrenPositions();
	}

	// 부모 UI에 대한 상대적 위치와 크기 설정
	void SetRelativePosition(FPOINT relPos) {
		relativePos = relPos;
		UpdateAbsolutePosition();
	}
	void SetRelativeSize(FPOINT relSize) {
		relativeSize = relSize;
		UpdateAbsoluteSize();
	}

	// 부모 UI 설정
	void SetParentUI(UIUnit* parent) {
		parentUI = parent;
		UpdateAbsolutePosition();
		UpdateAbsoluteSize();
	}

	FPOINT GetPosition() {
		return pos;
	}
	FPOINT GetSize() {
		return size;
	}

	void AddChild(UIUnit* child) {
		if(child) {
			child->SetParentUI(this);
			uiUnits.push_back(child);
		}
	}

protected:
	UIType uiType;
	vector<UIUnit*> uiUnits;  // 자식 UI 유닛들
	FPOINT pos;               // 절대 위치
	FPOINT size;              // 절대 크기
	FPOINT relativePos;       // 부모 UI에 대한 상대적 위치 (0~1)
	FPOINT relativeSize;      // 부모 UI에 대한 상대적 크기 (0~1)
	UIUnit* parentUI;         // 부모 UI 참조
	INT layer;
	bool isActive;
	bool useRelativePosition; // 상대적 위치 사용 여부
	bool useRelativeSize;     // 상대적 크기 사용 여부

	// 절대 위치 업데이트
	void UpdateAbsolutePosition() {
		if(parentUI && useRelativePosition) {
			FPOINT parentPos = parentUI->GetPosition();
			FPOINT parentSize = parentUI->GetSize();

			pos.x = parentPos.x + parentSize.x * relativePos.x;
			pos.y = parentPos.y + parentSize.y * relativePos.y;
		}
		UpdateChildrenPositions();
	}

	// 절대 크기 업데이트
	void UpdateAbsoluteSize() {
		if(parentUI && useRelativeSize) {
			FPOINT parentSize = parentUI->GetSize();

			size.x = parentSize.x * relativeSize.x;
			size.y = parentSize.y * relativeSize.y;
		}
		UpdateChildrenPositions();
	}

	// 자식 UI들의 위치와 크기 업데이트
	void UpdateChildrenPositions() {
		for(auto& unit : uiUnits) {
			if(unit) {
				unit->UpdateAbsolutePosition();
				unit->UpdateAbsoluteSize();
			}
		}
	}
};