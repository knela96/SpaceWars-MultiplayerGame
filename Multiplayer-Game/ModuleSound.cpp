#include "Networks.h"
#include "ModuleSound.h"

// DirectSoundCreate
#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppOS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);

// Sound
static LPDIRECTSOUNDBUFFER SoundBuffer;
static bool SoundIsPlaying = false;

struct sound_output
{
	DWORD SamplesPerSecond = 48000;       // NOTE(jesus): for each channel
	DWORD BytesPerSample = sizeof(int16); // NOTE(jesus): Size of a sample in a single channel
	DWORD ChannelCount = 2;
	DWORD SoundBufferSize = SamplesPerSecond * BytesPerSample * ChannelCount;
	DWORD LatencySampleCount = SamplesPerSecond * ChannelCount / 15;
	DWORD LastByteToLock = 0;
};

struct sound_output_buffer
{
	int16 *Samples;
	int32 SampleCount;
};

static sound_output SoundOutput;

#pragma pack(push, 1)

struct WAVE_header
{
	uint32 ChunkID;
	uint32 ChunkSize;
	uint32 Format;
};

struct WAVE_chunk
{
	uint32 ID;
	uint32 Size;
};

struct WAVE_fmt
{
	uint16 AudioFormat;
	uint16 NumChannels;
	uint32 SampleRate;
	uint32 ByteRate;
	uint16 BlockAlign;
	uint16 BitsPerSample;
	uint16 cbSize;
	uint16 ValidBitsPerSample;
	uint32 ChannelMask;
	uint8  SubFormat[16];
};

#pragma pack(pop)

#define QUAD_CHAR(a,b,c,d) (a) | (b<<8) | (c<<16) | (d<<24)

enum RIFFCode
{
	RIFF_RIFF = QUAD_CHAR('R', 'I', 'F', 'F'),
	RIFF_WAVE = QUAD_CHAR('W', 'A', 'V', 'E'),
	RIFF_fmt  = QUAD_CHAR('f', 'm', 't', ' '),
	RIFF_data = QUAD_CHAR('d', 'a', 't', 'a'),
};

bool ModuleSound::init()
{
	// Init DirectSound
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");
	if (DSoundLibrary)
	{
		direct_sound_create *DirectSoundCreate =
			(direct_sound_create*)GetProcAddress(DSoundLibrary, "DirectSoundCreate");
		LPDIRECTSOUND DirectSound;
		if (DirectSoundCreate && DirectSoundCreate(0, &DirectSound, 0) == DS_OK)
		{
			WAVEFORMATEX WaveFormat = {};
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nSamplesPerSec = SoundOutput.SamplesPerSecond;
			WaveFormat.wBitsPerSample = 16;
			WaveFormat.nChannels = 2;
			WaveFormat.nBlockAlign = WaveFormat.nChannels * (WaveFormat.wBitsPerSample / 8);
			WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec * WaveFormat.nBlockAlign;
			WaveFormat.cbSize = 0;

			if (DirectSound->SetCooperativeLevel(hwnd, DSSCL_PRIORITY) == DS_OK)
			{
				// Create a primary buffer
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				if (DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0) == DS_OK)
				{
					if (PrimaryBuffer->SetFormat(&WaveFormat) == DS_OK)
					{
						// We have finally set the format
						DLOG("The sound format was set correctly");
					}
					else
					{
						WLOG("ModuleSound::init() Could not set the format of the primary DSound buffer");
					}
				}
				else
				{
					WLOG("ModuleSound::init() Could not create the primary DSound buffer");
				}
			}
			else
			{
				WLOG("ModuleSound::init() Could not initialize DirectSound cooperative level");
			}

			// Create a secondary buffer
			DSBUFFERDESC BufferDescription = {};
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.dwFlags = 0;//DSBCAPS_GETCURRENTPOSITION2;
			BufferDescription.dwBufferBytes = SoundOutput.SoundBufferSize;
			BufferDescription.lpwfxFormat = &WaveFormat;
			if (DirectSound->CreateSoundBuffer(&BufferDescription, &SoundBuffer, 0) == DS_OK)
			{
				DLOG("Sound buffer created successfully");
			}
			else
			{
				WLOG("ModuleSound::init() Could not create the secondary DSound buffer");
			}
		}
		else
		{
			WLOG("ModuleSound::init() Could not create a DirectSound object");
		}
	}
	else
	{
		WLOG("ModuleSound::init() Could not load DirectSound library");
	}

	return true;
}


void ModuleSound::RenderAudio(sound_output_buffer *Buffer, uint32 advancedSamples)
{
	// Clear audio buffer
	int16 *SampleOut = Buffer->Samples;
	DWORD OutSampleCount = Buffer->SampleCount;
	for (DWORD OutSampleIndex = 0; OutSampleIndex < OutSampleCount; ++OutSampleIndex)
	{
		*SampleOut++ = 0;
	}

	// Add sound from all audio sources
	for (uint32 audioSourceIndex = 0; audioSourceIndex < ArrayCount(audioSources); ++audioSourceIndex)
	{
		audio_source &audioSource = audioSources[audioSourceIndex];
		if (audioSource.clip == nullptr) continue;

		if (audioSource.flags & AUDIO_SOURCE_START_BIT)
		{
			audioSource.flags &= ~AUDIO_SOURCE_START_BIT;
			advancedSamples = 0;
		}

		uint32 audioClipSampleCount = audioSource.clip->sampleCount;
		uint32 audioSourceSampleIndex = audioSource.lastWriteSampleIndex + advancedSamples;

		if (audioSourceSampleIndex < audioClipSampleCount ||
			audioSource.flags & AUDIO_SOURCE_LOOPS_BIT)
		{
			audioSourceSampleIndex = audioSourceSampleIndex % audioClipSampleCount;
			audioSource.lastWriteSampleIndex = audioSourceSampleIndex;

			uint32 remainingSampleCount = OutSampleCount;
			if ((audioSource.flags & AUDIO_SOURCE_LOOPS_BIT) == 0)
			{
				if (audioClipSampleCount - audioSourceSampleIndex < remainingSampleCount)
				{
					remainingSampleCount = audioClipSampleCount - audioSourceSampleIndex;
				}
			}

			int16 *SampleOut = Buffer->Samples;
			int16 *SamplesIn = (int16*)audioSource.clip->samples;
			for (DWORD i = 0; i < remainingSampleCount; ++i)
			{
				*SampleOut++ = SamplesIn[(audioSourceSampleIndex + i) % audioClipSampleCount];
			}
		}
		else
		{
			audioSource = {};
		}
	}

#if 0
	const int ToneVolume = 3000;
	const int ToneHz = 256;
	const int TonePeriod = 48000 / ToneHz;
	static float tSin = 0.0f;

	int16 *SampleOut = Buffer->Samples;
	DWORD SampleCount = Buffer->SampleCount;
	for (DWORD SampleIndex = 0; SampleIndex < SampleCount; ++SampleIndex)
	{
		tSin += 2.0f * 3.14159f * 1.0f / TonePeriod;
		float SinValue = sinf(tSin);
		int16 SampleValue = (int16)(SinValue * ToneVolume);
		*SampleOut++ = SampleValue; // LEFT
		*SampleOut++ = SampleValue; // RIGHT
	}
#endif
}

void ModuleSound::OutputAudio(sound_output *SoundOutput, sound_output_buffer *SourceBuffer, DWORD ByteToLock, DWORD BytesToWrite)
{
	VOID *Region1;
	DWORD Region1Size;
	VOID *Region2;
	DWORD Region2Size;
	if (SoundBuffer->Lock(ByteToLock, BytesToWrite,
		&Region1, &Region1Size,
		&Region2, &Region2Size,
		0) == DS_OK)
	{
		int16 *SrcSample = SourceBuffer->Samples;

		// First region (from ByteToLock)
		DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
		int16 *DstSample = (int16*)Region1;
		for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
		{
			*DstSample++ = *SrcSample++;
		}

		// Second region, if the interval to write looped back to the beginning
		DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
		DstSample = (int16*)Region2;
		for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
		{
			*DstSample++ = *SrcSample++;
		}

		SoundBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}

bool ModuleSound::postUpdate()
{
	DWORD PlayCursor;
	DWORD WriteCursor;
	if (SoundBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
	{
		// Current region to write
		DWORD ByteToLock = WriteCursor;
		DWORD BytesToWrite = SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample;
		DWORD TargetCursor = (WriteCursor + BytesToWrite) % SoundOutput.SoundBufferSize;

		// # of samples advanced since the last GetCurrentPosition
		DWORD AdvancedBytes = (ByteToLock > SoundOutput.LastByteToLock) ?
			ByteToLock - SoundOutput.LastByteToLock :
			SoundOutput.SoundBufferSize - SoundOutput.LastByteToLock + ByteToLock;
		DWORD AdvancedSamples = AdvancedBytes / SoundOutput.BytesPerSample;
		SoundOutput.LastByteToLock = ByteToLock;

		// Render audio sources in progress
		sound_output_buffer SoundOutputBuffer;
		SoundOutputBuffer.Samples = Samples;
		SoundOutputBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
		RenderAudio(&SoundOutputBuffer, AdvancedSamples);

		// Output rendered audio
		OutputAudio(&SoundOutput, &SoundOutputBuffer, ByteToLock, BytesToWrite);

	}

	if (!SoundIsPlaying)
	{
		SoundBuffer->Play(0, 0, DSBPLAY_LOOPING);
		SoundIsPlaying = true;
	}

	return true;
}

bool ModuleSound::cleanUp()
{
	for (uint32 i = 0; i < ArrayCount(audioClips); ++i)
	{
		free(audioClips[i].samples);
		audioClips[i] = {};
	}

	return true;
}

AudioClip * ModuleSound::loadAudioClip(const char * filename)
{
	AudioClip *audioClip = nullptr;

	for (uint32 i = 0; i < ArrayCount(audioClips); ++i)
	{
		if (audioClips[i].samples == nullptr) {
			audioClip = &audioClips[i];
			break;
		}
	}

	if (audioClip != nullptr)
	{
		FILE *file = fopen(filename, "rb");
		if (file != nullptr)
		{
			WAVE_header Header;
			WAVE_chunk  Chunk;
			WAVE_fmt    Fmt;
			uint32      dataSize;
			void       *data = nullptr;

			fread(&Header, sizeof(Header), 1, file);
			ASSERT(Header.ChunkID == RIFF_RIFF); // "RIFF"
			ASSERT(Header.Format == RIFF_WAVE); // "WAVE"

			while (1)
			{
				fread(&Chunk, sizeof(Chunk), 1, file);
				if (feof(file))
				{
					break;
				}

				switch (Chunk.ID)
				{
				case RIFF_fmt:
					fread(&Fmt, Chunk.Size, 1, file);
					ASSERT(Fmt.AudioFormat == 1); // 1 means PCM
					ASSERT(Fmt.SampleRate == 48000);
					ASSERT(Fmt.NumChannels == 2);
					ASSERT(Fmt.BitsPerSample == 16);
					break;
				case RIFF_data:
					ASSERT(data == nullptr);
					data = malloc(Chunk.Size);
					fread(data, Chunk.Size, 1, file);
					dataSize = Chunk.Size;
					ASSERT(data != nullptr);
					break;
				default:
					fseek(file, Chunk.Size, SEEK_CUR);
					break;
				}
			}


			audioClip->bitsPerSample = Fmt.BitsPerSample;
			audioClip->samplingRate = Fmt.SampleRate;
			audioClip->channelCount = Fmt.NumChannels;
			audioClip->sampleCount = dataSize / (Fmt.BitsPerSample / 8);
			audioClip->samples = data;

			fclose(file);
		}
		else
		{
			WLOG("Could not load sound file %s", filename);
			ASSERT(false);
		}
	}

	ASSERT(audioClip != nullptr);
	return audioClip;
}

void ModuleSound::freeAudioClip(AudioClip * audioClip)
{
	free(audioClip->samples);
	*audioClip = {};
}

void ModuleSound::playAudioClip(AudioClip * audioClip)
{
	// Search the same clip to reset the play
	for (uint32 i = 0; i < ArrayCount(audioSources); ++i)
	{
		if (audioSources[i].clip == audioClip)
		{
			audioSources[i].lastWriteSampleIndex = 0;
			audioSources[i].flags = AUDIO_SOURCE_START_BIT;
			return;
		}
	}

	// Clip not found, search an empty source
	for (uint32 i = 0; i < ArrayCount(audioSources); ++i)
	{
		if (audioSources[i].clip == nullptr)
		{
			audioSources[i].clip = audioClip;
			audioSources[i].lastWriteSampleIndex = 0;
			audioSources[i].flags = AUDIO_SOURCE_START_BIT;
			break;
		}
	}
}
