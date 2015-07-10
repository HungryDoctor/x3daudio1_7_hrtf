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
	HRESULT hr = CXAPOParametersBase::LockForProcess(
		InputLockedParameterCount,
		pInputLockedParameters,
		OutputLockedParameterCount,
		pOutputLockedParameters);

	_ASSERT(pInputLockedParameters[0].pFormat->nChannels == 1 || pInputLockedParameters[0].pFormat->nChannels == 2);
	_ASSERT(pOutputLockedParameters[0].pFormat->nChannels == 2);

	if (SUCCEEDED(hr))
	{
		memcpy(&m_input_format, pInputLockedParameters[0].pFormat, sizeof(m_input_format));
		memcpy(&m_output_format, pOutputLockedParameters[0].pFormat, sizeof(m_output_format));
	}
	return hr;
}

void HrtfXapoEffect::Process(UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS * pInputProcessParameters, UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS * pOutputProcessParameters, BOOL IsEnabled)
{
	_ASSERT(IsLocked());
	_ASSERT(InputProcessParameterCount == 1);
	_ASSERT(OutputProcessParameterCount == 1);
	//_ASSERT(pInputProcessParameters[0].pBuffer != pOutputProcessParameters[0].pBuffer);

	const bool in_place = pInputProcessParameters[0].pBuffer == pOutputProcessParameters[0].pBuffer;
	const UINT32 frame_count = pInputProcessParameters[0].ValidFrameCount;
	auto pInput = reinterpret_cast<float*>(pInputProcessParameters[0].pBuffer);
	auto pOutput = reinterpret_cast<float*>(pOutputProcessParameters[0].pBuffer);


	auto params = CXAPOParametersBase::BeginProcess();

	pOutputProcessParameters[0].BufferFlags = pInputProcessParameters[0].BufferFlags;
	pOutputProcessParameters[0].ValidFrameCount = pInputProcessParameters[0].ValidFrameCount;

	if (IsEnabled)
	{
		/*if (m_input_format.nChannels == 1)
		{
			for (UINT32 i = 0; i < frame_count; i++)
			{
				float time = float(m_sample_counter) / float(m_input_format.nSamplesPerSec);
				float value = 0.5f * std::sin(time * 440.0f * 3.14159265358979f);

				m_charge[0] += (pInput[i] - m_charge[0]) * 0.1f;
				pOutput[i * OUTPUT_CHANNEL_COUNT + 0] = m_charge[0] * 2.0f;

				m_charge[1] += (pInput[i] - m_charge[1]) * 0.1f;
				pOutput[i * OUTPUT_CHANNEL_COUNT + 1] = (pInput[i] - m_charge[1]) * 2.0f;

				m_sample_counter++;
			}
		}
		else if (m_input_format.nChannels == 2)
		{
			for (UINT32 i = 0; i < frame_count; i++)
			{
				float time = float(m_sample_counter) / float(m_input_format.nSamplesPerSec);
				float value = 0.5f * std::sin(time * 440.0f * 3.14159265358979f);

				m_charge[0] += (pInput[i * 2 + 0] - m_charge[0]) * 0.1f;
				pOutput[i * OUTPUT_CHANNEL_COUNT + 0] = m_charge[0] * 2.0f;

				m_charge[1] += (pInput[i * 2 + 1] - m_charge[1]) * 0.1f;
				pOutput[i * OUTPUT_CHANNEL_COUNT + 1] = (pInput[i * 2 + 1] - m_charge[1]) * 2.0f;

				m_sample_counter++;
			}
		}*/

		if (pInputProcessParameters[0].BufferFlags == XAPO_BUFFER_VALID)
		{
			const float time_per_frame = 1.0f / float(m_input_format.nSamplesPerSec);

			if (m_input_format.nChannels == 1)
			{
				for (UINT32 i = 0; i < frame_count; i++)
				{
					//float time = float(m_sample_counter) * time_per_frame;

					m_charge[0] += (pInput[i] - m_charge[0]) * 0.1f;
					m_charge[1] += (pInput[i] - m_charge[1]) * 0.1f;

					pOutput[i * OUTPUT_CHANNEL_COUNT + 0] = m_charge[0] * 2.0f;
					pOutput[i * OUTPUT_CHANNEL_COUNT + 1] = (pInput[i] - m_charge[1]) * 2.0f;

					m_sample_counter++;
				}
			}
			else if (m_input_format.nChannels == 2)
			{
				for (UINT32 i = 0; i < frame_count; i++)
				{
					//float time = float(m_sample_counter) * time_per_frame;

					m_charge[0] += (pInput[i * 2 + 0] - m_charge[0]) * 0.1f;
					m_charge[1] += (pInput[i * 2 + 1] - m_charge[1]) * 0.1f;

					pOutput[i * OUTPUT_CHANNEL_COUNT + 0] = m_charge[0] * 2.0f;
					pOutput[i * OUTPUT_CHANNEL_COUNT + 1] = (pInput[i * 2 + 1] - m_charge[1]) * 2.0f;

					m_sample_counter++;
				}
			}
		}
	}
	else
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

	CXAPOParametersBase::EndProcess();
}
