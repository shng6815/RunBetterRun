#pragma once

#pragma once
#include <Windows.h>
#include <mfplay.h>

class VideoManager
{
private:
	static HWND videoWindow;
	static IMFPMediaPlayer* pPlayer;
	static bool isPlaying;
	static float videoTimer;
	static float videoLength;

public:
	static void Init();
	static void Release();
	static bool PlayVideo(const wchar_t* filePath,float length);
	static void Update();
	static bool IsFinished();
	static void CloseVideoWindow();
};




