#include "stdafx.h"

#include "HrtfEffect.h"
#include "logger.h"
#include <limits>
#include <algorithm>

const UINT32 OUTPUT_CHANNEL_COUNT = 2;

XAPO_REGISTRATION_PROPERTIES HrtfXapoEffect::m_regProps = {
	__uuidof(HrtfXapoEffect),
	L"HRTF Effect",
	L"Copyright (C)2015 Roman Zavalov et al.",
	1,
	0,
	//XAPO_FLAG_INPLACE_REQUIRED
	//| XAPO_FLAG_CHANNELS_MUST_MATCH
	XAPO_FLAG_FRAMERATE_MUST_MATCH
	| XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH
	| XAPO_FLAG_BUFFERCOUNT_MUST_MATCH
	| XAPO_FLAG_INPLACE_SUPPORTED,
	1, 1, 1, 1 };

HrtfXapoEffect* HrtfXapoEffect::CreateInstance()
{
	auto instance = new HrtfXapoEffect();
	instance->Initialize(nullptr, 0);
	return instance;
}

HrtfXapoEffect::HrtfXapoEffect() :
	CXAPOParametersBase(&m_regProps, reinterpret_cast<BYTE*>(m_params), sizeof(HrtfXapoParam), FALSE)
	, m_time_per_frame(0)
{
	m_charge[0] = 0;
	m_charge[1] = 0;
}

HRESULT HrtfXapoEffect::LockForProcess(UINT32 InputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS * pInputLockedParameters, UINT32 OutputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS * pOutputLockedParameters)
{
	_ASSERT(pInputLockedParameters[0].pFormat->nChannels == 1 || pInputLockedParameters[0].pFormat->nChannels == 2);
	_ASSERT(pOutputLockedParameters[0].pFormat->nChannels == 2);

	HRESULT hr = CXAPOParametersBase::LockForProcess(
		InputLockedParameterCount,
		pInputLockedParameters,
		OutputLockedParameterCount,
		pOutputLockedParameters);

	if (SUCCEEDED(hr))
	{
		memcpy(&m_input_format, pInputLockedParameters[0].pFormat, sizeof(m_input_format));
		memcpy(&m_output_format, pOutputLockedParameters[0].pFormat, sizeof(m_output_format));

		m_time_per_frame = 1.0f / float(m_input_format.nSamplesPerSec);
	}
	return hr;
}

void HrtfXapoEffect::process_valid_buffer(const float * pInput, float * pOutput, const UINT32 frame_count, const HrtfXapoParam & params)
{
	const float volume = params.volume_multiplier;

	if (m_input_format.nChannels == 1)
	{
		for (UINT32 i = 0; i < frame_count; i++)
		{
			process_frame(pInput[i] * volume, pInput[i] * volume, pOutput[i * OUTPUT_CHANNEL_COUNT + 0], pOutput[i * OUTPUT_CHANNEL_COUNT + 1]);
		}
	}
	else if (m_input_format.nChannels == 2)
	{
		for (UINT32 i = 0; i < frame_count; i++)
		{
			process_frame(pInput[i * 2 + 0] * volume, pInput[i * 2 + 1] * volume, pOutput[i * OUTPUT_CHANNEL_COUNT + 0], pOutput[i * OUTPUT_CHANNEL_COUNT + 1]);
		}
	}
}

void HrtfXapoEffect::process_invalid_buffer(float * pOutput, const UINT32 frames_to_write_count, UINT32 & valid_frames_counter, const HrtfXapoParam & params)
{
	for (UINT32 i = 0; i < frames_to_write_count; i++)
	{
		process_frame(0, 0, pOutput[i * OUTPUT_CHANNEL_COUNT + 0], pOutput[i * OUTPUT_CHANNEL_COUNT + 1]);
	}
	valid_frames_counter = frames_to_write_count;
}

void HrtfXapoEffect::bypass(const float * pInput, float * pOutput, const UINT32 frame_count, const bool is_input_valid)
{
	const bool in_place = (pInput == pOutput);

	if (is_input_valid)
	{
		if (m_input_format.nChannels == 1)
		{
			for (UINT32 i = 0; i < frame_count; i++)
			{
				pOutput[i * OUTPUT_CHANNEL_COUNT + 0] = pInput[i];
				pOutput[i * OUTPUT_CHANNEL_COUNT + 1] = pInput[i];
			}
		}
		else if (m_input_format.nChannels == 2)
		{
			if (!in_place)
			{
				for (UINT32 i = 0; i < frame_count; i++)
				{
					pOutput[i * OUTPUT_CHANNEL_COUNT + 0] = pInput[i * 2 + 0];
					pOutput[i * OUTPUT_CHANNEL_COUNT + 1] = pInput[i * 2 + 1];
				}
			}
		}
	}
}

void HrtfXapoEffect::process_frame(const float left_input, const float right_input, float& left_output, float& right_output)
{
	m_charge[0] += (left_input - m_charge[0]) * m_time_per_frame * left_param;
	m_charge[1] += (right_input - m_charge[1]) * m_time_per_frame * right_param;

	left_output = m_charge[0] * 1.0f;
	right_output = (right_input - m_charge[1]) * 1.0f;
}

void HrtfXapoEffect::Process(UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS * pInputProcessParameters, UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS * pOutputProcessParameters, BOOL IsEnabled)
{
	_ASSERT(IsLocked());
	_ASSERT(InputProcessParameterCount == 1);
	_ASSERT(OutputProcessParameterCount == 1);

	const bool in_place = pInputProcessParameters[0].pBuffer == pOutputProcessParameters[0].pBuffer;
	const UINT32 input_frame_count = pInputProcessParameters[0].ValidFrameCount;
	const UINT32 frames_to_write_count = pOutputProcessParameters[0].ValidFrameCount;
	auto pInput = reinterpret_cast<const float*>(pInputProcessParameters[0].pBuffer);
	auto pOutput = reinterpret_cast<float*>(pOutputProcessParameters[0].pBuffer);
	const bool is_input_valid = pInputProcessParameters[0].BufferFlags == XAPO_BUFFER_VALID;

	auto params = reinterpret_cast<const HrtfXapoParam *>(BeginProcess());

	if (IsEnabled)
	{
		if (is_input_valid)
		{
			pOutputProcessParameters[0].BufferFlags = XAPO_BUFFER_VALID;
			pOutputProcessParameters[0].ValidFrameCount = input_frame_count;

			process_valid_buffer(pInput, pOutput, input_frame_count, *params);
		}
		else if (is_charged())
		{
			UINT32 valid_frames_counter;
			process_invalid_buffer(pOutput, frames_to_write_count, valid_frames_counter, *params);
			pOutputProcessParameters[0].BufferFlags = XAPO_BUFFER_VALID;
			pOutputProcessParameters[0].ValidFrameCount = valid_frames_counter;
		}
		else
		{
			pOutputProcessParameters[0].BufferFlags = XAPO_BUFFER_SILENT;
		}
	}
	else
	{
		pOutputProcessParameters[0].BufferFlags = pInputProcessParameters[0].BufferFlags;
		pOutputProcessParameters[0].ValidFrameCount = pInputProcessParameters[0].ValidFrameCount;

		bypass(pInput, pOutput, input_frame_count, is_input_valid);
	}

	EndProcess();
}

bool HrtfXapoEffect::is_charged() const
{
	return std::abs(m_charge[0]) > 0.001 && std::abs(m_charge[1]) > 0.001;
}
