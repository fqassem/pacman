#include "Sound.h"

void FMODErrorCheck(FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        std::cout << "FMOD error! (" << result << ") " << FMOD_ErrorString(result) << std::endl;
        exit(-1);
    }
}

Sound::Sound()
{
}

Sound::~Sound()
{
}

bool Sound::init()
{
	// Create FMOD interface object
	result = FMOD::System_Create(&system);
	FMODErrorCheck(result);
	result = system->getVersion(&version);
	FMODErrorCheck(result);
 
	if (version < FMOD_VERSION)
	{
		std::cout << "Error! You are using an old version of FMOD " << version << ". This program requires " << FMOD_VERSION << std::endl;
		return false; 
	}

	// Get number of sound cards 
	result = system->getNumDrivers(&numDrivers);
	FMODErrorCheck(result);
 
	// No sound cards (disable sound)
	if (numDrivers == 0)
	{
		result = system->setOutput(FMOD_OUTPUTTYPE_NOSOUND);
		FMODErrorCheck(result);
	}
	else
	{
		// Get the capabilities of the default (0) sound card
		result = system->getDriverCaps(0, &caps, 0, &speakerMode);
		FMODErrorCheck(result);
 
		// Set the speaker mode to match that in Control Panel
		result = system->setSpeakerMode(speakerMode);
		FMODErrorCheck(result);
		// Increase buffer size if user has Acceleration slider set to off
		if (caps & FMOD_CAPS_HARDWARE_EMULATED)
		{
			result = system->setDSPBufferSize(1024, 10);
			FMODErrorCheck(result);
		}
		// Get name of driver
		result = system->getDriverInfo(0, name, 256, 0);
		FMODErrorCheck(result);

	}

	result = system->init(100, FMOD_INIT_NORMAL, 0);
	if (result == FMOD_ERR_OUTPUT_CREATEBUFFER)
    {
        result = system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
        FMODErrorCheck(result);
 
        result = system->init(100, FMOD_INIT_NORMAL, 0);
    }
    FMODErrorCheck(result);
	return true;
}

void Sound::destroy()
{
	system->release();
}

void Sound::playBackgroundMusic(std::string sound)
{
	FMOD::Sound* soundEffect = soundMap["sound"];
	if(soundEffect == NULL)
	{
		result = system->createStream(sound.c_str(), FMOD_DEFAULT, 0, &soundEffect);
		FMODErrorCheck(result);
	}

    result = system->playSound(FMOD_CHANNEL_FREE, soundEffect, false, &bgChannel);
	bgChannel->setLoopCount(-1);
	bgChannel->setVolume(1.0);
	bgChannel->setMode(FMOD_LOOP_NORMAL);
}

void Sound::playSound(std::string sound, int loop)
{
	FMOD::Sound* soundEffect = soundMap["sound"];
	if(soundEffect == NULL)
	{
		result = system->createSound(sound.c_str(), FMOD_DEFAULT, 0, &soundEffect);
		FMODErrorCheck(result);
	}

    result = system->playSound(FMOD_CHANNEL_FREE, soundEffect, false, &soundEffectChannel);
	soundEffectChannel->setLoopCount(0);
	soundEffectChannel->setVolume(1.0);

}

void Sound::stopSound(std::string sound)
{
}

void Sound::update()
{
	FMODErrorCheck(system->update());
}