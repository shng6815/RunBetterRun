#include "SoundManager.h"
#include <SDL2/SDL.h>
#include <SDL2_mixer/SDL_mixer.h>

// 싱글톤 인스턴스 초기화
template<> SoundManager* Singleton<SoundManager>::instance = nullptr;

void SoundManager::ChannelFinished(int channel)
{
	// 싱글톤 인스턴스 접근
	SoundManager* instance = SoundManager::GetInstance();

	// 채널이 매핑되어 있는지 확인
	auto channelIt = instance->channelToSoundMap.find(channel);
	if(channelIt != instance->channelToSoundMap.end()) {
		std::string soundID = channelIt->second;

		// channelToSoundMap에서 제거
		instance->channelToSoundMap.erase(channelIt);

		// activeChannels에서 해당 채널 제거
		auto& channels = instance->activeChannels[soundID];
		channels.erase(std::remove(channels.begin(),channels.end(),channel),channels.end());

		// 해당 사운드의 모든 채널이 종료되었다면 엔트리 제거
		if(channels.empty()) {
			instance->activeChannels.erase(soundID);
		}
	}
}

HRESULT SoundManager::Init()
{
	if(SDL_Init(SDL_INIT_AUDIO) < 0)
		return E_FAIL;

	// SDL_mixer 초기화 (44100Hz, 기본 형식, 2채널(스테레오), 2048 샘플 버퍼)
	if(Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,2048) < 0) {
		return E_FAIL;
	}

	// 기본값 설정
	masterVolume = 1.0f;
	isMuted = false;
	currentMusicID = -1;

	// 카테고리별 볼륨 초기화
	categoryVolume[SoundType::BGM] = 1.0f;
	categoryVolume[SoundType::SFX] = 1.0f;

	Mix_ChannelFinished(ChannelFinished);  // 채널 종료 시 호출될 콜백 함수 설정

	return S_OK;
}

void SoundManager::Release()
{
	// 모든 음악 자원 해제
	StopAllSounds();

	// 배경음악 라이브러리 해제
	for(auto& music : musicLibrary) {
		if(music.second) {
			Mix_FreeMusic(music.second);
			music.second = nullptr;
		}
	}
	musicLibrary.clear();

	// 효과음 라이브러리 해제
	for(auto& sound : soundLibrary) {
		if(sound.second) {
			Mix_FreeChunk(sound.second);
			sound.second = nullptr;
		}
	}
	soundLibrary.clear();

	// SDL_mixer 종료
	Mix_CloseAudio();
	Mix_Quit();
	SDL_Quit();

	activeChannels.clear();
	channelToSoundMap.clear();
}

HRESULT SoundManager::LoadMusic(const std::string& musicID,const std::string& filePath)
{
	// 이미 동일한 ID가 있는지 확인
	if(musicLibrary.find(musicID) != musicLibrary.end()) {
		return E_FAIL;
	}

	// 음악 파일 로드
	Mix_Music* music = Mix_LoadMUS(filePath.c_str());
	if(!music) {
		return E_FAIL;
	}

	// 라이브러리에 추가
	musicLibrary[musicID] = music;

	return S_OK;
}

HRESULT SoundManager::LoadSound(const std::string& soundID,const std::string& filePath)
{
	// 이미 동일한 ID가 있는지 확인
	if(soundLibrary.find(soundID) != soundLibrary.end()) {
		return E_FAIL;
	}

	// 효과음 파일 로드
	Mix_Chunk* chunk = Mix_LoadWAV(filePath.c_str());
	if(!chunk) {
		return E_FAIL;
	}

	// 라이브러리에 추가
	soundLibrary[soundID] = chunk;

	return S_OK;
}

HRESULT SoundManager::PlayMusic(const std::string& musicID,bool loop,float volume)
{
	// 이미 재생 중인 음악이 있다면 중지
	StopMusic();

	// 음악이 존재하는지 확인
	auto it = musicLibrary.find(musicID);
	if(it == musicLibrary.end()) {
		return E_FAIL;
	}

	// 볼륨 계산 (마스터 볼륨 × 카테고리 볼륨 × 개별 볼륨)
	float calculatedVolume = masterVolume * categoryVolume[SoundType::BGM] * volume;
	if(isMuted) calculatedVolume = 0.0f;

	// MIX_MAX_VOLUME은 128이므로 0.0~1.0 범위를 0~128로 변환
	int sdlVolume = static_cast<int>(calculatedVolume * MIX_MAX_VOLUME);
	Mix_VolumeMusic(sdlVolume);

	// 음악 재생
	int loopCount = loop ? -1 : 0;  // -1은 무한 반복, 0은 1회 재생
	if(Mix_PlayMusic(it->second,loopCount) == -1) {
		return E_FAIL;
	}

	currentMusicID = 1;  // 음악이 재생 중임을 표시

	return S_OK;
}

HRESULT SoundManager::PlaySound(const std::string& soundID,bool loop,float volume)
{
	// 효과음이 존재하는지 확인
	auto it = soundLibrary.find(soundID);
	if(it == soundLibrary.end()) {
		return E_FAIL;
	}

	// 볼륨 계산 (마스터 볼륨 × 카테고리 볼륨 × 개별 볼륨)
	float calculatedVolume = masterVolume * categoryVolume[SoundType::SFX] * volume;
	if(isMuted) calculatedVolume = 0.0f;

	// MIX_MAX_VOLUME은 128이므로 0.0~1.0 범위를 0~128로 변환
	int sdlVolume = static_cast<int>(calculatedVolume * MIX_MAX_VOLUME);
	Mix_VolumeChunk(it->second,sdlVolume);

	// 효과음 재생
	int loopCount = loop ? -1 : 0;  // -1은 무한 반복, 0은 1회 재생
	int channel = Mix_PlayChannel(-1,it->second,loopCount);  // -1은 첫 번째 사용 가능한 채널 사용

	if(channel == -1) {
		return E_FAIL;
	}

	// 채널 추적에 추가
	activeChannels[soundID].push_back(channel);
	channelToSoundMap[channel] = soundID;

	return S_OK;
}

void SoundManager::StopMusic()
{
	if(Mix_PlayingMusic()) {
		Mix_HaltMusic();
		currentMusicID = -1;
	}
}

void SoundManager::StopSound(int channel)
{
	Mix_HaltChannel(channel);

	// 채널이 매핑되어 있는지 확인
	auto it = channelToSoundMap.find(channel);
	if(it != channelToSoundMap.end()) {
		std::string soundID = it->second;
		channelToSoundMap.erase(it);

		// activeChannels에서 해당 채널 제거
		auto& channels = activeChannels[soundID];
		channels.erase(std::remove(channels.begin(),channels.end(),channel),channels.end());

		// 해당 사운드의 모든 채널이 종료되었다면 엔트리 제거
		if(channels.empty()) {
			activeChannels.erase(soundID);
		}
	}
}

void SoundManager::StopSound(std::string soundID)
{
	auto it = activeChannels.find(soundID);
	if(it != activeChannels.end()) {
		// 임시 벡터에 채널 번호 복사 (iterator 무효화 방지)
		std::vector<int> channelsToStop = it->second;

		// 모든 채널 중지
		for(int channel : channelsToStop) {
			Mix_HaltChannel(channel);
			// 채널-사운드 매핑에서 제거
			channelToSoundMap.erase(channel);
		}

		// 사운드 엔트리 제거
		activeChannels.erase(soundID);
	}
}

void SoundManager::PauseMusic()
{
	if(Mix_PlayingMusic() && !Mix_PausedMusic()) {
		Mix_PauseMusic();
	}
}

void SoundManager::ResumeMusic()
{
	if(Mix_PausedMusic()) {
		Mix_ResumeMusic();
	}
}

void SoundManager::SetMasterVolume(float volume)
{
	// 볼륨 범위 제한 (0.0 ~ 1.0)
	masterVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;

	// 현재 재생 중인 음악의 볼륨 업데이트
	if(Mix_PlayingMusic()) {
		float musicVolume = masterVolume * categoryVolume[SoundType::BGM];
		if(isMuted) musicVolume = 0.0f;
		Mix_VolumeMusic(static_cast<int>(musicVolume * MIX_MAX_VOLUME));
	}
}

void SoundManager::SetCategoryVolume(SoundType type,float volume)
{
	// 볼륨 범위 제한 (0.0 ~ 1.0)
	categoryVolume[type] = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;

	// 해당 카테고리가 BGM이고 현재 음악이 재생 중이면 볼륨 업데이트
	if(type == SoundType::BGM && Mix_PlayingMusic()) {
		float musicVolume = masterVolume * categoryVolume[SoundType::BGM];
		if(isMuted) musicVolume = 0.0f;
		Mix_VolumeMusic(static_cast<int>(musicVolume * MIX_MAX_VOLUME));
	}
}

void SoundManager::SetMute(bool mute)
{
	isMuted = mute;

	// 현재 재생 중인 모든 소리의 볼륨 설정
	if(Mix_PlayingMusic()) {
		float musicVolume = isMuted ? 0.0f : masterVolume * categoryVolume[SoundType::BGM];
		Mix_VolumeMusic(static_cast<int>(musicVolume * MIX_MAX_VOLUME));
	}
}

void SoundManager::ToggleMute()
{
	SetMute(!isMuted);
}

bool SoundManager::IsMusicPlaying()
{
	return Mix_PlayingMusic() && !Mix_PausedMusic();
}

bool SoundManager::IsSoundPlaying(std::string soundID)
{
	auto it = activeChannels.find(soundID);
	return (it != activeChannels.end() && !it->second.empty());
}

void SoundManager::StopAllSounds()
{
	// 모든 효과음 채널 중지
	Mix_HaltChannel(-1);

	// 추적 맵 초기화
	activeChannels.clear();
	channelToSoundMap.clear();

	// 음악 중지
	StopMusic();
}

void SoundManager::Update()
{
	// 필요한 경우 매 프레임 업데이트 로직 구현
	// 예: 페이드 인/아웃, 동적 볼륨 조절 등
}