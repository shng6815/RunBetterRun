#pragma once
#include "Singleton.h"
#include <map>
#include <string>
#include <Windows.h>
#include <vector>

// SDL_mixer를 위한 전방 선언
struct Mix_Music;  // 올바른 전방 선언
struct Mix_Chunk;  // 올바른 전방 선언

// 사운드 타입 정의
enum class SoundType {
	BGM,      // 배경음악
	SFX,      // 효과음
	VOICE,    // 음성
	AMBIENT   // 환경음
};

class SoundManager: public Singleton<SoundManager>
{
private:
	std::map<std::string,Mix_Music*> musicLibrary;    // 배경음악 라이브러리
	std::map<std::string,Mix_Chunk*> soundLibrary;    // 효과음 라이브러리
	std::map<std::string,std::vector<int>> activeChannels;
	std::map<int,std::string> channelToSoundMap;

	float masterVolume;                                // 마스터 볼륨
	std::map<SoundType,float> categoryVolume;         // 카테고리별 볼륨
	bool isMuted;                                      // 음소거 상태
	int currentMusicID;                                // 현재 재생 중인 음악 ID

	static void ChannelFinished(int channel);
public:
	// 초기화 및 해제
	HRESULT Init();
	void Release();
	// 사운드 파일 관리
	HRESULT LoadMusic(const std::string& musicID,const std::string& filePath);
	HRESULT LoadSound(const std::string& soundID,const std::string& filePath);
	// 재생 제어
	HRESULT PlayMusic(const std::string& musicID,bool loop = true,float volume = 1.0f);
	HRESULT PlaySound(const std::string& soundID,bool loop = false,float volume = 1.0f);
	void StopMusic();
	void StopSound(int channel = -1);  // -1은 모든 채널
	void StopSound(std::string soundID);
	void PauseMusic();
	void ResumeMusic();
	// 볼륨 제어
	void SetMasterVolume(float volume);
	void SetCategoryVolume(SoundType type,float volume);
	// 특수 제어
	void SetMute(bool mute);
	void ToggleMute();
	// 상태 확인
	bool IsMusicPlaying();
	bool IsSoundPlaying(std::string soundID);
	// 정리
	void StopAllSounds();
	void Update();  // 필요하다면 매 프레임 업데이트
};