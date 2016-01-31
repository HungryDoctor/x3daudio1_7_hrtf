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
		, m_sends(sends)
	{
	}

	explicit VoiceSends(std::vector<XAUDIO2_SEND_DESCRIPTOR> && sends)
		: m_sendToMasterOnly(false)
		, m_sends(sends)
	{
	}

	VoiceSends(const VoiceSends & other)
		: m_sendToMasterOnly(other.m_sendToMasterOnly)
		, m_sends(other.m_sends)
	{
	}

	VoiceSends(VoiceSends && other)
		: m_sendToMasterOnly(other.m_sendToMasterOnly)
		, m_sends(std::move(other.m_sends))
	{

	}

	bool getSendToMasterOnly() const
	{
		return m_sendToMasterOnly;
	}

	const std::vector<XAUDIO2_SEND_DESCRIPTOR> & getSends() const
	{
		return m_sends;
	}

	VoiceSends & operator=(const VoiceSends& other)
	{
		if (this == &other)
			return *this;
		m_sendToMasterOnly = other.m_sendToMasterOnly;
		m_sends = other.m_sends;
		return *this;
	}

	VoiceSends & operator=(VoiceSends&& other)
	{
		if (this == &other)
			return *this;
		m_sendToMasterOnly = other.m_sendToMasterOnly;
		m_sends = std::move(other.m_sends);
		return *this;
	}

private:
	bool m_sendToMasterOnly;
	std::vector<XAUDIO2_SEND_DESCRIPTOR> m_sends;
};