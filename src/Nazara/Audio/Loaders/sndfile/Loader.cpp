// Copyright (C) 2012 Jérôme Leclercq
// This file is part of the "Nazara Engine - Audio module"
// For conditions of distribution and use, see copyright notice in Config.hpp

#include <Nazara/Audio/Loaders/sndfile.hpp>
#include <Nazara/Audio/Audio.hpp>
#include <Nazara/Audio/Config.hpp>
#include <Nazara/Audio/Music.hpp>
#include <Nazara/Audio/SoundBuffer.hpp>
#include <Nazara/Audio/SoundStream.hpp>
#include <Nazara/Core/Endianness.hpp>
#include <Nazara/Core/Error.hpp>
#include <Nazara/Core/File.hpp>
#include <Nazara/Core/InputStream.hpp>
#include <Nazara/Core/MemoryStream.hpp>
#include <Nazara/Core/String.hpp>
#include <memory>
#include <sndfile/sndfile.h>
#include <Nazara/Audio/Debug.hpp>

namespace
{
	sf_count_t GetSize(void* user_data)
	{
        NzInputStream* stream = static_cast<NzInputStream*>(user_data);
		return stream->GetSize();
	}

    sf_count_t Read(void* ptr, sf_count_t count, void* user_data)
    {
        NzInputStream* stream = static_cast<NzInputStream*>(user_data);
        return stream->Read(ptr, count);
    }

    sf_count_t Seek(sf_count_t offset, int whence, void* user_data)
    {
        NzInputStream* stream = static_cast<NzInputStream*>(user_data);
        switch (whence)
        {
			case SEEK_CUR:
				stream->Read(nullptr, offset);
				break;

			case SEEK_END:
				stream->SetCursorPos(stream->GetSize() + offset); // L'offset est négatif ici
				break;

			case SEEK_SET:
				stream->SetCursorPos(offset);
				break;

			default:
				NazaraInternalError("Seek mode not handled");
        }

        return stream->GetCursorPos();
    }

    sf_count_t Tell(void* user_data)
    {
        NzInputStream* stream = reinterpret_cast<NzInputStream*>(user_data);
        return stream->GetCursorPos();
    }

	static SF_VIRTUAL_IO callbacks = {GetSize, Seek, Read, nullptr, Tell};

	class sndfileStream : public NzSoundStream
	{
		public:
			sndfileStream() :
			m_file(nullptr),
			m_handle(nullptr)
			{
			}

			~sndfileStream()
			{
				if (m_handle)
					sf_close(m_handle);

				if (m_file)
					delete m_file;
			}

			nzUInt32 GetDuration() const
			{
				return m_duration;
			}

			nzAudioFormat GetFormat() const
			{
				return m_format;
			}

			unsigned int GetSampleCount() const
			{
				return m_sampleCount;
			}

			unsigned int GetSampleRate() const
			{
				return m_sampleRate;
			}

			bool Open(const NzString& filePath)
			{
				m_file = new NzFile(filePath);
				if (!m_file->Open(NzFile::ReadOnly))
				{
					NazaraError("Failed to open file " + filePath);
					return false;
				}

				return Open(*m_file);
			}

			bool Open(NzInputStream& stream)
			{
				SF_INFO infos;
				m_handle = sf_open_virtual(&callbacks, SFM_READ, &infos, &stream);
				if (!m_handle)
				{
					NazaraError("Failed to open sound: " + NzString(sf_strerror(m_handle)));
					return false;
				}

				m_format = NzAudio::GetAudioFormat(infos.channels);
				if (m_format == nzAudioFormat_Unknown)
				{
					NazaraError("Channel count not handled");
					sf_close(m_handle);
					m_handle = nullptr;

					return false;
				}

				m_sampleCount = infos.channels*infos.frames;
				m_sampleRate = infos.samplerate;

				m_duration = 1000*m_sampleCount / (m_format*m_sampleRate);

				// https://github.com/LaurentGomila/SFML/issues/271
				// http://www.mega-nerd.com/libsndfile/command.html#SFC_SET_SCALE_FLOAT_INT_READ
				///FIXME: Seulement le Vorbis ?
				/*if (infos.format & SF_FORMAT_VORBIS)
					sf_command(m_handle, SFC_SET_SCALE_FLOAT_INT_READ, nullptr, SF_TRUE);*/

				return true;
			}

			unsigned int Read(void* buffer, unsigned int sampleCount)
			{
				return sf_read_short(m_handle, reinterpret_cast<nzInt16*>(buffer), sampleCount);
			}

			void Seek(nzUInt32 offset)
			{
				sf_seek(m_handle, offset*m_sampleRate / 1000, SEEK_SET);
			}

		private:
			nzAudioFormat m_format;
			NzFile* m_file;
			SNDFILE* m_handle;
			unsigned int m_duration;
			unsigned int m_sampleCount;
			unsigned int m_sampleRate;
	};

	bool IsSupported(const NzString& extension)
	{
		static std::set<NzString> supportedExtensions = {
			"aiff", "au", "avr", "caf", "flac", "htk", "ircam", "mat4", "mat5", "mpc2k",
			"nist","ogg", "pvf", "raw", "rf64", "sd2", "sds", "svx", "voc", "w64", "wav", "wve"};

		return supportedExtensions.find(extension) != supportedExtensions.end();
	}

	bool CheckMusic(NzInputStream& stream, const NzMusicParams& parameters)
	{
		NazaraUnused(parameters);

		SF_INFO info;
		SNDFILE* file = sf_open_virtual(&callbacks, SFM_READ, &info, &stream);
		if (file)
		{
			sf_close(file);
			return true;
		}
		else
			return false;
	}

	bool LoadMusicFile(NzMusic* music, const NzString& filePath, const NzMusicParams& parameters)
	{
		NazaraUnused(parameters);

		std::unique_ptr<sndfileStream> musicStream(new sndfileStream);
		if (!musicStream->Open(filePath))
		{
			NazaraError("Failed to open music stream");
			return false;
		}

		if (!music->Create(musicStream.get()))
		{
			NazaraError("Failed to create music");
			return false;
		}

		musicStream.release();

		return true;
	}

	bool LoadMusicStream(NzMusic* music, NzInputStream& stream, const NzMusicParams& parameters)
	{
		NazaraUnused(parameters);

		std::unique_ptr<sndfileStream> musicStream(new sndfileStream);
		if (!musicStream->Open(stream))
		{
			NazaraError("Failed to open music stream");
			return false;
		}

		if (!music->Create(musicStream.get()))
		{
			NazaraError("Failed to create music");
			return false;
		}

		musicStream.release();

		return true;
	}

	bool CheckSoundBuffer(NzInputStream& stream, const NzSoundBufferParams& parameters)
	{
		NazaraUnused(parameters);

		SF_INFO info;
		SNDFILE* file = sf_open_virtual(&callbacks, SFM_READ, &info, &stream);
		if (file)
		{
			sf_close(file);
			return true;
		}
		else
			return false;
	}

	bool LoadSoundBuffer(NzSoundBuffer* soundBuffer, NzInputStream& stream, const NzSoundBufferParams& parameters)
	{
		NazaraUnused(parameters);

		sndfileStream musicStream;
		if (!musicStream.Open(stream))
			return false; // L'erreur a déjà été envoyée par la méthode

		unsigned int sampleCount = musicStream.GetSampleCount();
		std::unique_ptr<nzInt16[]> samples(new nzInt16[sampleCount]);

		if (musicStream.Read(samples.get(), sampleCount) != sampleCount)
		{
			NazaraError("Failed to read samples");
			return false;
		}

		if (!soundBuffer->Create(musicStream.GetFormat(), sampleCount, musicStream.GetSampleRate(), samples.get()))
		{
			NazaraError("Failed to create sound buffer");
			return false;
		}

		return true;
	}
}

void NzLoaders_sndfile_Register()
{
	NzMusicLoader::RegisterLoader(IsSupported, CheckMusic, LoadMusicStream, LoadMusicFile);
	NzSoundBufferLoader::RegisterLoader(IsSupported, CheckSoundBuffer, LoadSoundBuffer);
}

void NzLoaders_sndfile_Unregister()
{
	NzMusicLoader::UnregisterLoader(IsSupported, CheckMusic, LoadMusicStream, LoadMusicFile);
	NzSoundBufferLoader::UnregisterLoader(IsSupported, CheckSoundBuffer, LoadSoundBuffer);
}
