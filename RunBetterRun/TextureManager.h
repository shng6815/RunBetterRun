#pragma once
#include "Singleton.h"
#include "structs.h"
#include <map>

class TextureManager: public Singleton<TextureManager>
{
	map<LPCWCH, Texture> source;

	HRESULT		LoadTexture(LPCWCH path, Texture& texture);

public:
	void		Release(void);
	Texture*	GetTexture(LPCWCH path);
	BOOL		Empty(void) { return source.empty(); }
};

