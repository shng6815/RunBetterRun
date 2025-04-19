#pragma once
#include "Singleton.h"
#include "structs.h"
#include "Item.h"
#include <list>


class ItemManager: public Singleton<ItemManager>
{
	list<Item>	items;
	DWORD		keyCount;

	HRESULT LoadFile(LPCWCH path);

public:
	HRESULT Init(void);
	HRESULT Init(LPCWCH path);
	void	Release(void);
	void	Update(void);

	SIZE_T	Size(void) { return items.size(); }
	void	PutItem(FPOINT pos);

	void	PushKey(void);
	void	PopKey(void);
};

