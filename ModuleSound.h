#ifndef MODULE_SOUND_H
#define MODULE_SOUND_H


struct sound_output;
struct sound_output_buffer;

struct AudioClip
{
	void *samples = nullptr;
	uint32 sampleCount = 0;
	uint32 samplingRate = 0;
	uint16 bitsPerSample = 0;
	uint16 channelCount = 0;
};

class ModuleSound : public Module
{
public:

	// Module methods

	bool init() override;

	bool postUpdate() override;

	bool cleanUp() override;


	// ModuleSound methods

	AudioClip* loadAudioClip (const char *filename);

	void freeAudioClip(AudioClip *audioClip);

	void playAudioClip(AudioClip *audioClip);

private:

	void RenderAudio(sound_output_buffer *Buffer, uint32 advancedSamples);

	void OutputAudio(sound_output *SoundOutput, sound_output_buffer *SourceBuffer, DWORD ByteToLock, DWORD BytesToWrite);

	int16 Samples[48000 * 2];

	AudioClip audioClips[2] = {};

	enum audio_source_flags {
		AUDIO_SOURCE_START_BIT = 1<<0,
		AUDIO_SOURCE_LOOPS_BIT = 1<<1
	};

	struct audio_source
	{
		AudioClip *clip = nullptr;
		uint32 lastWriteSampleIndex = 0;
		uint8 flags = 0;
	};

	audio_source audioSources[2] = {};
};

#endif // MODULE_SOUND_H
