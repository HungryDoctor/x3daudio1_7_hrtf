#pragma once

#include <XAudio2.h>
#include <sstream>

inline void print_sends(std::wstringstream & ss, const XAUDIO2_VOICE_SENDS * pSendList)
{
	ss << "sends [";
	if (pSendList)
	{
		if (pSendList->SendCount > 1)
		{
			for (UINT32 i = 0; i < pSendList->SendCount - 1; i++)
			{
				ss << pSendList->pSends[i].pOutputVoice << ", ";
			}
		}
		if (pSendList->SendCount > 0)
			ss << pSendList->pSends[pSendList->SendCount - 1].pOutputVoice;
	}
	ss << "]";
}
