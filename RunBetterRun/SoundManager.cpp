#include "SoundManager.h"
#include <fstream>
#include <mmreg.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>

// 로컬 코드에서만 사용할 헬퍼 함수
namespace {
// .wav 파일에서 RIFF 헤더 찾기
bool FindChunk(HANDLE hFile,DWORD fourcc,DWORD& chunkSize,DWORD& chunkDataPosition)
{
	DWORD chunkType;
	DWORD chunkDataSize;
	DWORD riffDataSize = 0;
	DWORD fileType;
	DWORD bytesRead = 0;
	DWORD offset = 0;

	SetFilePointer(hFile,0,NULL,FILE_BEGIN);

	while(true)
	{
		DWORD read;
		if(ReadFile(hFile,&chunkType,sizeof(DWORD),&read,NULL) == 0)
			return false;

		if(ReadFile(hFile,&chunkDataSize,sizeof(DWORD),&read,NULL) == 0)
			return false;

		switch(chunkType)
		{
		case 'FFIR': // RIFF 청크
		riffDataSize = chunkDataSize;
		chunkDataSize = 4;
		if(ReadFile(hFile,&fileType,sizeof(DWORD),&read,NULL) == 0)
			return false;
		break;

		default:
		if(SetFilePointer(hFile,chunkDataSize,NULL,FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			return false;
		}

		offset += sizeof(DWORD) * 2;

		if(chunkType == fourcc)
		{
			chunkSize = chunkDataSize;
			chunkDataPosition = offset;
			return true;
		}

		offset += chunkDataSize;

		if(bytesRead >= riffDataSize)
			return false;
	}

	return false;
}

// 데이터 읽기
bool ReadChunkData(HANDLE hFile,void* buffer,DWORD bufferSize,DWORD bufferOffset)
{
	DWORD read;
	SetFilePointer(hFile,bufferOffset,NULL,FILE_BEGIN);
	return (ReadFile(hFile,buffer,bufferSize,&read,NULL) != 0);
}
}

HRESULT SoundManager::Init()
{
	// COM 초기화 - 반드시 XAudio2 생성 전에 호출
	HRESULT hr = CoInitializeEx(nullptr,COINIT_MULTITHREADED);
	if(FAILED(hr))
	{
		MessageBox(NULL,L"COM 초기화 실패",L"오류",MB_OK);
		return hr;
	}

	hr = InitXAudio2();
	if(FAILED(hr))
		return hr;

	masterVolume = 1.0f;
	isMuted = false;

	// 카테고리별 볼륨 초기화
	categoryVolume[SoundType::BGM] = 0.7f;
	categoryVolume[SoundType::SFX] = 1.0f;
	categoryVolume[SoundType::VOICE] = 1.0f;
	categoryVolume[SoundType::AMBIENT] = 0.5f;

	return S_OK;
}

HRESULT SoundManager::InitXAudio2()
{
	HRESULT hr;

	// XAudio2 엔진 초기화 - 지역 변수가 아닌 클래스 멤버 변수에 직접 할당
	hr = XAudio2Create(&this->xAudio,0);  // 여기를 수정! ComPtr<IXAudio2> xAudio 지역 변수를 제거하고 this->xAudio에 직접 할당
	if(FAILED(hr) || this->xAudio == nullptr)
	{
		MessageBox(NULL,L"XAudio2 생성 실패",L"오류",MB_OK);
		return hr;
	}

	#ifdef _DEBUG
	// 디버그 모드에서 디버그 설정 활성화
	XAUDIO2_DEBUG_CONFIGURATION debug = {0};
	debug.TraceMask = XAUDIO2_LOG_ERRORS | XAUDIO2_LOG_WARNINGS;
	debug.BreakMask = XAUDIO2_LOG_ERRORS;
	this->xAudio->SetDebugConfiguration(&debug,0);  // 여기도 this->xAudio로 수정
	#endif

	// 마스터링 보이스 생성
	hr = this->xAudio->CreateMasteringVoice(&masteringVoice);  // 여기도 this->xAudio로 수정
	if(FAILED(hr))
		return hr;

	return S_OK;
}

void SoundManager::Release()
{
	// 모든 사운드 정지
	StopAllSounds();

	// 활성화된 모든 소스 보이스 제거
	for(auto& voicePair : activeVoices)
	{
		for(auto& instance : voicePair.second)
		{
			if(instance.sourceVoice)
			{
				instance.sourceVoice->Stop();
				instance.sourceVoice->DestroyVoice();
				instance.sourceVoice = nullptr;
			}
		}
	}
	activeVoices.clear();

	// 마스터링 보이스 제거
	if(masteringVoice)
	{
		masteringVoice->DestroyVoice();
		masteringVoice = nullptr;
	}

	// 사운드 라이브러리 정리
	soundLibrary.clear();

	// XAudio2 엔진 해제
	xAudio.Reset();

	CoUninitialize();
}

HRESULT SoundManager::LoadWaveFile(const std::wstring& filePath,WAVEDATA& waveData)
{
	HANDLE hFile = CreateFile(
		filePath.c_str(),
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if(hFile == INVALID_HANDLE_VALUE)
		return HRESULT_FROM_WIN32(GetLastError());

	if(SetFilePointer(hFile,0,NULL,FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD chunkSize;
	DWORD chunkPosition;

	// RIFF 청크 찾기
	if(!FindChunk(hFile,'FFIR',chunkSize,chunkPosition))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	DWORD fileType;
	if(!ReadChunkData(hFile,&fileType,sizeof(DWORD),chunkPosition))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// 파일 유형 확인
	if(fileType != 'EVAW')
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// 포맷 청크 찾기
	if(!FindChunk(hFile,' tmf',chunkSize,chunkPosition))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// 포맷 정보 읽기
	if(!ReadChunkData(hFile,&waveData.waveFormat,chunkSize,chunkPosition))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// 데이터 청크 찾기
	if(!FindChunk(hFile,'atad',chunkSize,chunkPosition))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// 오디오 데이터 버퍼 생성
	waveData.audioData.resize(chunkSize);

	// 데이터 읽기
	if(!ReadChunkData(hFile,waveData.audioData.data(),chunkSize,chunkPosition))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// XAUDIO2_BUFFER 구조체 설정
	waveData.buffer.AudioBytes = chunkSize;
	waveData.buffer.pAudioData = waveData.audioData.data();
	waveData.buffer.Flags = 0;
	waveData.buffer.PlayBegin = 0;
	waveData.buffer.PlayLength = 0;
	waveData.buffer.LoopBegin = 0;
	waveData.buffer.LoopLength = 0;
	waveData.buffer.LoopCount = 0;
	waveData.buffer.pContext = NULL;

	waveData.isLoaded = true;
	CloseHandle(hFile);

	return S_OK;
}

HRESULT SoundManager::LoadSound(const std::string& soundID,const std::wstring& filePath)
{
	// 이미 로드된 사운드인지 확인
	if(soundLibrary.find(soundID) != soundLibrary.end())
		return S_OK;

	WAVEDATA waveData;
	waveData.isLoaded = false;

	HRESULT hr = LoadWaveFile(filePath,waveData);
	if(FAILED(hr))
		return hr;

	// 사운드 라이브러리에 추가
	soundLibrary[soundID] = waveData;

	return S_OK;
}

void SoundManager::UnloadSound(const std::string& soundID)
{
	// 해당 사운드의 모든 인스턴스 정지 및 제거
	StopSound(soundID);

	// 사운드 라이브러리에서 제거
	auto it = soundLibrary.find(soundID);
	if(it != soundLibrary.end())
	{
		soundLibrary.erase(it);
	}
}

HRESULT SoundManager::PlaySound(const std::string& soundID,SoundType type,bool loop,float volume)
{
	// 사운드 데이터 찾기
	auto it = soundLibrary.find(soundID);
	if(it == soundLibrary.end())
		return E_FAIL;

	WAVEDATA& waveData = it->second;
	if(!waveData.isLoaded)
		return E_FAIL;

	// 새 소스 보이스 생성
	IXAudio2SourceVoice* sourceVoice = nullptr;
	HRESULT hr = xAudio->CreateSourceVoice(&sourceVoice,&waveData.waveFormat);
	if(FAILED(hr))
		return hr;

	// 루프 설정
	XAUDIO2_BUFFER buffer = waveData.buffer;
	if(loop)
	{
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// 카테고리별 볼륨과 마스터 볼륨 적용
	float adjustedVolume = volume * categoryVolume[type];
	if(isMuted)
		adjustedVolume = 0.0f;
	else
		adjustedVolume *= masterVolume;

	sourceVoice->SetVolume(adjustedVolume);

	// 버퍼 제출 및 재생 시작
	hr = sourceVoice->SubmitSourceBuffer(&buffer);
	if(FAILED(hr))
	{
		sourceVoice->DestroyVoice();
		return hr;
	}

	hr = sourceVoice->Start();
	if(FAILED(hr))
	{
		sourceVoice->DestroyVoice();
		return hr;
	}

	// 활성 인스턴스 목록에 추가
	SoundInstance instance;
	instance.sourceVoice = sourceVoice;
	instance.volume = volume;
	instance.pitch = 1.0f;
	instance.isPlaying = true;
	instance.isLooping = loop;

	if(activeVoices.find(soundID) == activeVoices.end())
	{
		activeVoices[soundID] = std::vector<SoundInstance>();
	}
	activeVoices[soundID].push_back(instance);

	return S_OK;
}

void SoundManager::StopSound(const std::string& soundID,int instanceID)
{
	auto it = activeVoices.find(soundID);
	if(it == activeVoices.end())
		return;

	auto& instances = it->second;

	if(instanceID == -1) // 모든 인스턴스 정지
	{
		for(auto& instance : instances)
		{
			if(instance.sourceVoice)
			{
				instance.sourceVoice->Stop();
				instance.sourceVoice->FlushSourceBuffers();
				instance.sourceVoice->DestroyVoice();
				instance.sourceVoice = nullptr;
			}
			instance.isPlaying = false;
		}
		instances.clear();
	} else if(instanceID >= 0 && instanceID < instances.size()) // 특정 인스턴스 정지
	{
		auto& instance = instances[instanceID];
		if(instance.sourceVoice)
		{
			instance.sourceVoice->Stop();
			instance.sourceVoice->FlushSourceBuffers();
			instance.sourceVoice->DestroyVoice();
			instance.sourceVoice = nullptr;
		}
		instance.isPlaying = false;
		instances.erase(instances.begin() + instanceID);
	}
}

void SoundManager::PauseSound(const std::string& soundID,int instanceID)
{
	auto it = activeVoices.find(soundID);
	if(it == activeVoices.end())
		return;

	auto& instances = it->second;

	if(instanceID == -1) // 모든 인스턴스 일시 정지
	{
		for(auto& instance : instances)
		{
			if(instance.sourceVoice && instance.isPlaying)
			{
				instance.sourceVoice->Stop();
				instance.isPlaying = false;
			}
		}
	} else if(instanceID >= 0 && instanceID < instances.size()) // 특정 인스턴스 일시 정지
	{
		auto& instance = instances[instanceID];
		if(instance.sourceVoice && instance.isPlaying)
		{
			instance.sourceVoice->Stop();
			instance.isPlaying = false;
		}
	}
}

void SoundManager::ResumeSound(const std::string& soundID,int instanceID)
{
	auto it = activeVoices.find(soundID);
	if(it == activeVoices.end())
		return;

	auto& instances = it->second;

	if(instanceID == -1) // 모든 인스턴스 재개
	{
		for(auto& instance : instances)
		{
			if(instance.sourceVoice && !instance.isPlaying)
			{
				instance.sourceVoice->Start();
				instance.isPlaying = true;
			}
		}
	} else if(instanceID >= 0 && instanceID < instances.size()) // 특정 인스턴스 재개
	{
		auto& instance = instances[instanceID];
		if(instance.sourceVoice && !instance.isPlaying)
		{
			instance.sourceVoice->Start();
			instance.isPlaying = true;
		}
	}
}

void SoundManager::SetMasterVolume(float volume)
{
	masterVolume = volume;
	if(masterVolume < 0.0f) masterVolume = 0.0f;
	if(masterVolume > 1.0f) masterVolume = 1.0f;

	// 모든 활성 인스턴스의 볼륨 업데이트
	for(auto& voicePair : activeVoices)
	{
		for(auto& instance : voicePair.second)
		{
			if(instance.sourceVoice)
			{
				float adjustedVolume = isMuted ? 0.0f : instance.volume * masterVolume;
				instance.sourceVoice->SetVolume(adjustedVolume);
			}
		}
	}
}

void SoundManager::SetCategoryVolume(SoundType type,float volume)
{
	if(volume < 0.0f) volume = 0.0f;
	if(volume > 1.0f) volume = 1.0f;

	categoryVolume[type] = volume;

	// 해당 카테고리의 모든 활성 인스턴스 볼륨 업데이트 필요
	// (여기에서는 카테고리 정보를 저장하지 않으므로 구현하지 않음)
}

void SoundManager::SetSoundVolume(const std::string& soundID,int instanceID,float volume)
{
	auto it = activeVoices.find(soundID);
	if(it == activeVoices.end())
		return;

	auto& instances = it->second;

	if(instanceID >= 0 && instanceID < instances.size()) // 특정 인스턴스 볼륨 설정
	{
		auto& instance = instances[instanceID];
		instance.volume = volume;
		if(instance.sourceVoice)
		{
			float adjustedVolume = isMuted ? 0.0f : volume * masterVolume;
			instance.sourceVoice->SetVolume(adjustedVolume);
		}
	}
}

void SoundManager::SetSoundPitch(const std::string& soundID,int instanceID,float pitch)
{
	auto it = activeVoices.find(soundID);
	if(it == activeVoices.end())
		return;

	auto& instances = it->second;

	if(instanceID >= 0 && instanceID < instances.size()) // 특정 인스턴스 피치 설정
	{
		auto& instance = instances[instanceID];
		instance.pitch = pitch;
		if(instance.sourceVoice)
		{
			// XAudio2에서 피치는 주파수 비율로 표현됨
			instance.sourceVoice->SetFrequencyRatio(pitch);
		}
	}
}

void SoundManager::SetMute(bool mute)
{
	isMuted = mute;

	// 모든 활성 소리의 볼륨 업데이트
	for(auto& voicePair : activeVoices)
	{
		for(auto& instance : voicePair.second)
		{
			if(instance.sourceVoice)
			{
				float adjustedVolume = isMuted ? 0.0f : instance.volume * masterVolume;
				instance.sourceVoice->SetVolume(adjustedVolume);
			}
		}
	}
}

void SoundManager::ToggleMute()
{
	SetMute(!isMuted);
}

bool SoundManager::IsPlaying(const std::string& soundID,int instanceID)
{
	auto it = activeVoices.find(soundID);
	if(it == activeVoices.end())
		return false;

	auto& instances = it->second;

	if(instanceID == -1) // 어떤 인스턴스라도 재생 중인지 확인
	{
		for(auto& instance : instances)
		{
			if(instance.isPlaying)
				return true;
		}
		return false;
	} else if(instanceID >= 0 && instanceID < instances.size()) // 특정 인스턴스 확인
	{
		return instances[instanceID].isPlaying;
	}

	return false;
}

void SoundManager::StopAllSounds()
{
	for(auto& voicePair : activeVoices)
	{
		for(auto& instance : voicePair.second)
		{
			if(instance.sourceVoice)
			{
				instance.sourceVoice->Stop();
				instance.sourceVoice->FlushSourceBuffers();
				instance.sourceVoice->DestroyVoice();
				instance.sourceVoice = nullptr;
			}
			instance.isPlaying = false;
		}
	}
	activeVoices.clear();
}

void SoundManager::Update()
{
	// 완료된 소리 정리 작업 (여기서는 간단하게 구현)
	for(auto it = activeVoices.begin(); it != activeVoices.end();)
	{
		auto& instances = it->second;

		// 각 인스턴스 상태 확인
		instances.erase(
			std::remove_if(instances.begin(),instances.end(),
			[](SoundInstance& instance) {
			if(!instance.sourceVoice)
				return true;

			XAUDIO2_VOICE_STATE state;
			instance.sourceVoice->GetState(&state);

			// 버퍼가 남아있지 않고 루프가 아닌 경우, 재생 완료로 간주
			if(state.BuffersQueued == 0 && !instance.isLooping)
			{
				instance.sourceVoice->DestroyVoice();
				instance.sourceVoice = nullptr;
				instance.isPlaying = false;
				return true;
			}
			return false;
		}
		),
			instances.end()
		);

		// 빈 목록 제거
		if(instances.empty())
			it = activeVoices.erase(it);
		else
			++it;
	}
}