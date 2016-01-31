#pragma once

#include "ChannelMatrix.h"

#include <XAudio2.h>
#include <sstream>
#include <vector>
#include <cstdint>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <string>

#include "common_types.h"

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

inline VoiceSends from_XAUDIO2_VOICE_SENDS(const XAUDIO2_VOICE_SENDS * pSendList)
{
	if (pSendList)
	{
		voice_sends sends;
		sends.resize(pSendList->SendCount);
		std::copy(pSendList->pSends, pSendList->pSends + pSendList->SendCount, std::begin(sends));
		return VoiceSends(sends);
	}
	else
	{
		return VoiceSends();
	}
}

inline effect_chain from_XAUDIO2_EFFECT_CHAIN(const XAUDIO2_EFFECT_CHAIN * pEffectChain)
{
	effect_chain effects;
	if (pEffectChain)
	{
		effects.resize(pEffectChain->EffectCount);
		std::copy(pEffectChain->pEffectDescriptors, pEffectChain->pEffectDescriptors + pEffectChain->EffectCount, std::begin(effects));
	}
	return effects;
}

template <typename T>
inline std::vector<T> buffer_to_vector(const T * pBuffer, UINT32 elementCount)
{
	std::vector<T> buf;
	buf.resize(elementCount);
	std::copy(pBuffer, pBuffer + elementCount, std::begin(buf));
	return buf;
}

inline std::vector<int8_t> buffer_to_vector(const void * pBuffer, UINT32 elementCount)
{
	return buffer_to_vector(reinterpret_cast<const int8_t *>(pBuffer), elementCount);
}


// TODO : const T * pBuffer should not compile!!!!
template<typename T>
inline void vector_to_buffer(const std::vector<T> & vec, T * pBuffer, UINT32 elementCount)
{
	if (vec.size() != elementCount)
		throw std::logic_error("Buffer size mismatch");
	
	std::copy(std::begin(vec), std::end(vec), pBuffer);
}

inline void vector_to_buffer(const std::vector<int8_t> & vec, void * pBuffer, UINT32 bufferByteSize)
{
	vector_to_buffer(vec, reinterpret_cast<int8_t*>(pBuffer), bufferByteSize);
}

inline void from_ChannelMatrix(const ChannelMatrix & matrix, UINT32 sourceChannels, UINT32 destinationChannels, float * pLevelMatrix)
{
	if (matrix.getSourceCount() != sourceChannels || matrix.getDestinationCount() != destinationChannels)
		throw std::invalid_argument("Matrix size mismatch");

	for (UINT32 destIndex = 0; destIndex < destinationChannels; destIndex++)
	{
		for (UINT32 srcIndex = 0; srcIndex < sourceChannels; srcIndex++)
		{
			pLevelMatrix[destIndex * sourceChannels + srcIndex] = matrix.getValue(srcIndex, destIndex);
		}
	}
}

inline std::wstring get_name(const void * ptr)
{
	std::wstringstream ss;
	auto result = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	ss << std::put_time(std::localtime(&result), L"%H-%M-%S") << L"_";
	ss << ptr;
	return ss.str();
}

inline XAUDIO2_VOICE_SENDS from_voice_sends(const VoiceSends & sends)
{
	XAUDIO2_VOICE_SENDS sendsStruct;
	sendsStruct.SendCount = static_cast<UINT32>(sends.getSends().size());
	sendsStruct.pSends = const_cast<XAUDIO2_SEND_DESCRIPTOR*>(&*sends.getSends().begin());
	return sendsStruct;
}