#include "pch.h"
#include "SoundManager.h"

void SoundManager::Init()
{
	result = FMOD::System_Create(&system);
	if (result != FMOD_OK)
		assert(1);                  // �ý��� ��ü�� �����Ѵ�.

	result = system->getVersion(&version);
	if (result != FMOD_OK)
		assert(1);
	else printf("FMOD version %08x\n", version);

	result = system->init(32, FMOD_INIT_NORMAL, fextradriverdata);  // �ý��� �ʱ�ȭ : ä�� 32������ ����ϰڴ�.
	if (result != FMOD_OK)
		assert(1);

	//result = system->createSound("singing.wav", FMOD_LOOP_NORMAL, 0, &fsound);

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
	result = system->update();		//��� ������Ʈ
	if (result != FMOD_OK) assert(1);

	if (channel)
	{
		bool playing = false;
		result = channel->isPlaying(&playing);  // ���� ��� ���̶�� playing�� true�� �ǰ� ����� �����ٸ� false�� �Ǿ� ���� ���� ���������� ���α׷� ���� 
		//if (!playing) assert(1);
	}
}

void SoundManager::CreateSound(const std::string& filename, const std::string& soundname, const bool& loop)
{
	const int loopflag = loop ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;

	result = system->createSound(filename.c_str(), loopflag, 0, &_SoundMap[soundname]); // wav ���Ϸκ��� sound ����
	
	if (result != FMOD_OK)
	{
		std::cout << filename << " is not loaded";
	}
}

void SoundManager::PlaySound(const std::string& sound_name, float volume)
{
	result = system->playSound(_SoundMap[sound_name], 0, false, &channel);
	channel->setVolume(volume);
}

void SoundManager::StopSound(const std::string& sound_name)
{
	result = system->playSound(_SoundMap[sound_name], 0, true, &channel);
}