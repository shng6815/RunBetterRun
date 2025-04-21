#pragma once
#include "Singleton.h"
#include <xaudio2.h>
#include <map>
#include <string>
#include <vector>
#include <wrl/client.h>

#pragma comment(lib, "xaudio2.lib")

using Microsoft::WRL::ComPtr;

// WAVE 파일 구조체
struct WAVEDATA {
	WAVEFORMATEX waveFormat;        // 오디오 포맷
	XAUDIO2_BUFFER buffer;          // 오디오 버퍼
	std::vector<BYTE> audioData;    // 오디오 데이터
	bool isLoaded;                  // 로드 여부
};

// 사운드 타입 정의
enum class SoundType {
	BGM,      // 배경음악(길게 재생되는 음악)
	SFX,      // 효과음(짧은 소리)
	VOICE,    // 음성(대사 등)
	AMBIENT   // 환경음(배경 소음 등)
};

// 사운드 인스턴스 (동일한 사운드를 여러 곳에서 재생할 때 사용)
struct SoundInstance {
	IXAudio2SourceVoice* sourceVoice;
	float volume;
	float pitch;
	bool isPlaying;
	bool isLooping;
};

class SoundManager: public Singleton<SoundManager>
{
private:
	ComPtr<IXAudio2> xAudio;                         // XAudio2 엔진
	IXAudio2MasteringVoice* masteringVoice;          // 마스터 보이스

	std::map<std::string,WAVEDATA> soundLibrary;    // 사운드 라이브러리
	std::map<std::string,std::vector<SoundInstance>> activeVoices;  // 현재 재생 중인 소리

	float masterVolume;             // 마스터 볼륨
	std::map<SoundType,float> categoryVolume;  // 카테고리별 볼륨

	bool isMuted;                   // 음소거 상태

	// 파일에서 WAV 데이터 로드
	HRESULT LoadWaveFile(const std::wstring& filePath,WAVEDATA& waveData);

	// 오디오 엔진 초기화
	HRESULT InitXAudio2();

public:
	// 초기화 및 해제
	HRESULT Init();
	void Release();

	// 사운드 파일 관리
	HRESULT LoadSound(const std::string& soundID,const std::wstring& filePath);
	void UnloadSound(const std::string& soundID);

	// 재생 제어
	HRESULT PlaySound(const std::string& soundID,SoundType type,bool loop = false,float volume = 1.0f);
	void StopSound(const std::string& soundID,int instanceID = -1); // -1은 모든 인스턴스
	void PauseSound(const std::string& soundID,int instanceID = -1);
	void ResumeSound(const std::string& soundID,int instanceID = -1);

	// 볼륨 제어
	void SetMasterVolume(float volume);
	void SetCategoryVolume(SoundType type,float volume);
	void SetSoundVolume(const std::string& soundID,int instanceID,float volume);

	// 특수 제어
	void SetSoundPitch(const std::string& soundID,int instanceID,float pitch);
	void SetMute(bool mute);
	void ToggleMute();

	// 상태 확인
	bool IsPlaying(const std::string& soundID,int instanceID = -1);

	// 정리
	void StopAllSounds();
	void Update();  // 비활성화된 소리들 정리
};