#pragma once
#include "Singleton.h"
#include "structs.h"
#include <list>

class ItemManager: public Singleton<ItemManager>
{
	list<Sprite>	items;
	Texture*		texture;
	AnimationInfo	aniInfo;

	HRESULT LoadFile(LPCWCH path);

public:
	HRESULT Init(void);
	HRESULT Init(LPCWCH path);
	void	Release(void);
	void	Update(void);

	SIZE_T	Size(void) { return items.size(); }
	void	PutItem(FPOINT pos);
};

