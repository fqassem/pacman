#pragma once

#include "fmod.hpp"
#include "fmod_errors.h"
#include <iostream>
#include <map>
#include <string>

class Sound
{
public:
	Sound();
	~Sound();
	bool init();
	void destroy();
	void update();
	void playBackgroundMusic(std::string sound);
	void playSound(std::string sound, int loop);
	void stopSound(std::string sound);

	//app specific code

	std::map<std::string, FMOD::Sound*> soundMap;
private:
	FMOD::System *system;
	FMOD_RESULT result;
	unsigned int version;
	int numDrivers;
	FMOD_SPEAKERMODE speakerMode;
	FMOD_CAPS caps;
	char name[256];
	FMOD::Channel* soundEffectChannel, *bgChannel;
};