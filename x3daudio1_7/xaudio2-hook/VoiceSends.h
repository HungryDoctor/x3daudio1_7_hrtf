#pragma once

#include <XAudio2.h>
#include <vector>

class VoiceSends
{
public:
	VoiceSends()
		: m_sendToMasterOnly(true)
	{
	}

	explicit VoiceSends(const std::vector<XAUDIO2_SEND_DESCRIPTOR> & sends)
		: m_sendToMasterOnly(false)
		, m_voices(sends)
	{
	}

	explicit VoiceSends(std::vector<XAUDIO2_SEND_DESCRIPTOR> && sends)
		: m_sendToMasterOnly(false)
		, m_voices(sends)
	{
	}

	VoiceSends(const VoiceSends & other)
		: m_sendToMasterOnly(other.m_sendToMasterOnly)
		, m_voices(other.m_voices)
	{
	}

	VoiceSends(VoiceSends && other)
		: m_sendToMasterOnly(other.m_sendToMasterOnly)
		, m_voices(std::move(other.m_voices))
	{

	}

	bool getSendToMasterOnly() const
	{
		return m_sendToMasterOnly;
	}

	const std::vector<XAUDIO2_SEND_DESCRIPTOR> & getVoices() const
	{
		return m_voices;
	}

	VoiceSends & operator=(const VoiceSends& other)
	{
		if (this == &other)
			return *this;
		m_sendToMasterOnly = other.m_sendToMasterOnly;
		m_voices = other.m_voices;
		return *this;
	}

	VoiceSends & operator=(VoiceSends&& other)
	{
		if (this == &other)
			return *this;
		m_sendToMasterOnly = other.m_sendToMasterOnly;
		m_voices = std::move(other.m_voices);
		return *this;
	}

private:
	bool m_sendToMasterOnly;
	std::vector<XAUDIO2_SEND_DESCRIPTOR> m_voices;
};