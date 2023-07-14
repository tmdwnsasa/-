#pragma once

#include <iostream>
#include "GameObject.h"
#include "fmod.hpp"

class SoundManager
{
	DECLARE_SINGLE(SoundManager);

public:
	void Init();
	void Update();

	void CreateSound(const string& filename, const string& soundname, const bool& loop);
	void PlaySound(const string& sound_name, float volume);
	void StopSound(const string& sound_name);

private:
	FMOD::System* system;

	std::map<std::string, FMOD::Sound*> _SoundMap;


	//Spawn
	FMOD::Sound* Bruisersound;
	FMOD::Sound* Stalkersound;

	//Enemy
	FMOD::Sound* Deadsound;

	//Shop
	FMOD::Sound* Buyingsound;

	FMOD::Channel* channel;
	void* fextradriverdata;

	FMOD_RESULT       result;
	unsigned int      version;
};

