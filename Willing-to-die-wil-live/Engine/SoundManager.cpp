#include "pch.h"
#include "SoundManager.h"

void SoundManager::Init()
{
	result = FMOD::System_Create(&system);
	if (result != FMOD_OK)
		assert(1);                  // 시스템 객체를 생성한다.

	result = system->getVersion(&version);
	if (result != FMOD_OK)
		assert(1);
	else printf("FMOD version %08x\n", version);

	result = system->init(32, FMOD_INIT_NORMAL, fextradriverdata);  // 시스템 초기화 : 채널 32개까지 사용하겠다.
	if (result != FMOD_OK)
		assert(1);

	//result = system->createSound("singing.wav", FMOD_LOOP_NORMAL, 0, &fsound);
	CreateSound("..\\Resources\\Sound\\Footsteps.wav", "Footwalksound", true);

	CreateSound("..\\Resources\\Sound\\PistolShot.wav", "Pistolsound", false);
	CreateSound("..\\Resources\\Sound\\ShotgunShot.wav", "Shotgunsound", false);
	CreateSound("..\\Resources\\Sound\\SniperShot.wav", "Snipersound", false);
	CreateSound("..\\Resources\\Sound\\SmgShot.wav", "Smgsound", false);

	CreateSound("..\\Resources\\Sound\\SniperShot.wav", "Bruisersound", false);
	CreateSound("..\\Resources\\Sound\\SmgShot.wav", "Stalkersound", false);
	CreateSound("..\\Resources\\Sound\\SniperShot.wav", "Deadsound", false);
	CreateSound("..\\Resources\\Sound\\SmgShot.wav", "Buyingsound", false);
}

void SoundManager::Update()
{
	result = system->update();		//재생 업데이트
	if (result != FMOD_OK) assert(1);

}

void SoundManager::CreateSound(const std::string& filename, const std::string& soundname, const bool& loop)
{
	const int loopflag = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

	result = system->createSound(filename.c_str(), loopflag, 0, &_SoundMap[soundname]); // wav 파일로부터 sound 생성
	
	if (result != FMOD_OK)
	{
		std::cout << filename << " is not loaded";
	}
}

void SoundManager::PlaySound(const std::string& soundname, float volume)
{
	result = system->playSound(_SoundMap[soundname], 0, false, &channel);
	channel->setVolume(volume);
}



void SoundManager::StopSound(const std::string& soundname)
{
	result = system->playSound(_SoundMap[soundname], 0, true, &channel);
}

void SoundManager::PlayLoopSound(const string& soundname, float volume)
{
	result = system->playSound(_SoundMap[soundname], 0, false, &_ChannelMap[soundname]);
	_ChannelMap[soundname]->setPaused(false);

	_ChannelMap[soundname]->setVolume(volume);
	_PlayingSoundVector.push_back(soundname);
}

void SoundManager::StopLoopSound(const string& soundname)
{
	_ChannelMap[soundname]->setPaused(true);
	_PlayingSoundVector.erase(remove(_PlayingSoundVector.begin(), _PlayingSoundVector.end(), soundname), _PlayingSoundVector.end());
}

bool SoundManager::IsPlaying(const std::string& soundname)
{
	for (auto sound : _PlayingSoundVector)
	{
		if (sound == soundname)
		{
			return true;
		}
	}
	return false;
}