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
	void PlaySound(const string& soundname, float volume);
	void StopSound(const string& soundname);
	void PlayLoopSound(const string& soundname, float volume);
	void StopLoopSound(const string& soundname);
	bool IsPlaying(const std::string& sound_name);

private:
	FMOD::System* system;

	std::map<std::string, FMOD::Sound*> _SoundMap;
	std::vector<std::string> _PlayingSoundVector;

	FMOD::Channel* channel;
	std::map<std::string, FMOD::Channel*> _ChannelMap;
	void* fextradriverdata;

	FMOD_RESULT       result;
	unsigned int      version;
};