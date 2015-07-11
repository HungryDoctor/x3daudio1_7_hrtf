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

HrtfXapoEffect * HrtfXapoEffect::CreateInstance()
{
	auto instance = new HrtfXapoEffect();
	instance->Initialize(nullptr, 0);
	return instance;
}

HrtfXapoEffect::HrtfXapoEffect() :
	CXAPOParametersBase(&m_regProps, reinterpret_cast<BYTE*>(m_params), sizeof(HrtfXapoParam), FALSE)
	, m_sample_counter(0)
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
	}
	return hr;
}

void HrtfXapoEffect::process_valid_buffer(const float* pInput, float* pOutput, const UINT32 frame_count)
{
	const float time_per_frame = 1.0f / float(m_input_format.nSamplesPerSec);

	if (m_input_format.nChannels == 1)
	{
		for (UINT32 i = 0; i < frame_count; i++)
		{
			m_charge[0] += (pInput[i] - m_charge[0]) * time_per_frame * left_param;
			m_charge[1] += (pInput[i] - m_charge[1]) * time_per_frame * right_param;

			pOutput[i * OUTPUT_CHANNEL_COUNT + 0] = m_charge[0] * 0.25f;
			pOutput[i * OUTPUT_CHANNEL_COUNT + 1] = (pInput[i] - m_charge[1]) * 0.25f;

			m_sample_counter++;
		}
	}
	else if (m_input_format.nChannels == 2)
	{
		for (UINT32 i = 0; i < frame_count; i++)
		{
			m_charge[0] += (pInput[i * 2 + 0] - m_charge[0]) * time_per_frame * left_param;
			m_charge[1] += (pInput[i * 2 + 1] - m_charge[1]) * time_per_frame * right_param;

			pOutput[i * OUTPUT_CHANNEL_COUNT + 0] = m_charge[0] * 0.25f;
			pOutput[i * OUTPUT_CHANNEL_COUNT + 1] = (pInput[i * 2 + 1] - m_charge[1]) * 0.25f;

			m_sample_counter++;
		}
	}
}

void HrtfXapoEffect::process_invalid_buffer(float* pOutput, const UINT32 frames_to_write_count, UINT32& valid_frames_counter)
{
	const float time_per_frame = 1.0f / float(m_input_format.nSamplesPerSec);

	UINT32 valid_frames = 0;

	for (UINT32 i = 0; i < frames_to_write_count ; i++)
	{
		m_charge[0] += (0 - m_charge[0]) * time_per_frame * left_param;
		m_charge[1] += (0 - m_charge[1]) * time_per_frame * right_param;

		pOutput[i * OUTPUT_CHANNEL_COUNT + 0] = m_charge[0] * 0.25f;
		pOutput[i * OUTPUT_CHANNEL_COUNT + 1] = (0 - m_charge[1]) * 0.25f;

		if (
			!(pOutput[i * OUTPUT_CHANNEL_COUNT + 0] < 10.0f && pOutput[i * OUTPUT_CHANNEL_COUNT + 0] > -10.0f
				&& pOutput[i * OUTPUT_CHANNEL_COUNT + 1] < 10.0f && pOutput[i * OUTPUT_CHANNEL_COUNT + 1] > -10.0f)
			)
		{
			logger::log("shiet");
		}

		m_sample_counter++;
		valid_frames++;
	}
	valid_frames_counter = valid_frames;
}

void HrtfXapoEffect::bypass(const float* pInput, float* pOutput, const UINT32 frame_count, const bool is_input_valid)
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

	auto params = CXAPOParametersBase::BeginProcess();

	pOutputProcessParameters[0].BufferFlags = (is_input_valid || is_charged()) ? XAPO_BUFFER_VALID : XAPO_BUFFER_SILENT;

	if (IsEnabled)
	{
		/*if (is_input_valid)
		{
			pOutputProcessParameters[0].BufferFlags = XAPO_BUFFER_VALID;
			pOutputProcessParameters[0].ValidFrameCount = input_frame_count;

			process_valid_buffer(pInput, pOutput, input_frame_count);
		}
		else if (is_charged())
		{
			UINT32 valid_frames_counter;
			process_invalid_buffer(pOutput, frames_to_write_count, valid_frames_counter);
			pOutputProcessParameters[0].BufferFlags = XAPO_BUFFER_VALID;
			pOutputProcessParameters[0].ValidFrameCount = valid_frames_counter;
		}
		else
		{
			pOutputProcessParameters[0].BufferFlags = XAPO_BUFFER_SILENT;
		}*/
		bypass(pInput, pOutput, input_frame_count, is_input_valid);
	}
	else
	{
		pOutputProcessParameters[0].BufferFlags = pInputProcessParameters[0].BufferFlags;
		pOutputProcessParameters[0].ValidFrameCount = pInputProcessParameters[0].ValidFrameCount;

		bypass(pInput, pOutput, input_frame_count, is_input_valid);
	}

	CXAPOParametersBase::EndProcess();
}

bool HrtfXapoEffect::is_charged() const
{
	return std::abs(m_charge[0]) > 0.001 && std::abs(m_charge[1]) > 0.001;
}
