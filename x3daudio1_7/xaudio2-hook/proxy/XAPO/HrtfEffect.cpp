#include "stdafx.h"

#include "HrtfEffect.h"
#include "logger.h"
#include <limits>
#include <algorithm>

#include <intrin.h>

const UINT32 OUTPUT_CHANNEL_COUNT = 2;

XAPO_REGISTRATION_PROPERTIES HrtfXapoEffect::m_regProps = {
	__uuidof(HrtfXapoEffect),
	L"HRTF Effect",
	L"Copyright (C)2015 Roman Zavalov",
	1,
	0,
	XAPO_FLAG_FRAMERATE_MUST_MATCH
	| XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH
	| XAPO_FLAG_BUFFERCOUNT_MUST_MATCH
	| XAPO_FLAG_INPLACE_SUPPORTED,
	1, 1, 1, 1 };

HrtfXapoEffect* HrtfXapoEffect::CreateInstance()
{
	static std::shared_ptr<HrtfDataSet> hrtf_data(new HrtfDataSet({ L"hrtf\\default-44100.mhr", L"hrtf\\default-48000.mhr" }));
	auto instance = new HrtfXapoEffect(hrtf_data);
	instance->Initialize(nullptr, 0);
	return instance;
}

HrtfXapoEffect::HrtfXapoEffect(const std::shared_ptr<IHrtfDataSet> & hrtf_data) :
	CXAPOParametersBase(&m_regProps, reinterpret_cast<BYTE*>(m_params), sizeof(HrtfXapoParam), FALSE)
	, m_time_per_frame(0)
	, m_hrtf_data_set(hrtf_data)
	, m_hrtf_data(nullptr)
	, m_invalid_buffers_count(0)
	, m_buffers_per_history(0)
	, m_history_size(0)
{
	m_params[0] = { 0 };
	m_params[1] = { 0 };
	m_params[2] = { 0 };
	m_signal.reserve(192);
	m_hrtf_data_left.impulse_response.reserve(128);
	m_hrtf_data_right.impulse_response.reserve(128);
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
		if (m_hrtf_data_set->has_sample_rate(m_input_format.nSamplesPerSec))
		{
			m_hrtf_data = &m_hrtf_data_set->get_sample_rate_data(m_input_format.nSamplesPerSec);
			m_history_size = m_hrtf_data->get_longest_delay() + m_hrtf_data->get_respoone_length();
			m_buffers_per_history = m_history_size / (pInputLockedParameters[0].MaxFrameCount + m_history_size - 1);
			m_invalid_buffers_count = m_buffers_per_history;

			m_signal.resize(m_history_size + pInputLockedParameters[0].MaxFrameCount);
			std::fill(std::begin(m_signal), std::begin(m_signal) + m_history_size, 0.0f);
		}
		else
		{
			m_hrtf_data = nullptr;
		}
	}
	return hr;
}

void HrtfXapoEffect::process_valid_buffer(const float * pInput, float * pOutput, const UINT32 frame_count, const HrtfXapoParam & params)
{
	const float volume = params.volume_multiplier;

	if (m_input_format.nChannels == 1)
	{
		std::transform(pInput, pInput + frame_count, std::begin(m_signal) + m_history_size, [=](float value) { return value * volume; });
	}
	else if (m_input_format.nChannels == 2)
	{
		for (UINT32 i = 0; i < frame_count; i++)
		{
			m_signal[i + m_history_size] = (pInput[i * 2 + 0] + pInput[i * 2 + 1]) * 0.5f * volume;
		}
	}

	for (UINT32 i = 0; i < frame_count; i++)
	{
		process_frame(pOutput[i * OUTPUT_CHANNEL_COUNT + 0], pOutput[i * OUTPUT_CHANNEL_COUNT + 1], i);
	}

	std::copy(std::end(m_signal) - m_history_size, std::end(m_signal), std::begin(m_signal));
	m_invalid_buffers_count = 0;
}

void HrtfXapoEffect::process_invalid_buffer(float * pOutput, const UINT32 frames_to_write_count, UINT32 & valid_frames_counter, const HrtfXapoParam & params)
{
	std::fill(std::begin(m_signal) + m_history_size, std::end(m_signal), 0.0f);

	for (UINT32 i = 0; i < frames_to_write_count; i++)
	{
		process_frame(pOutput[i * OUTPUT_CHANNEL_COUNT + 0], pOutput[i * OUTPUT_CHANNEL_COUNT + 1], i);
	}
	valid_frames_counter = frames_to_write_count;

	std::copy(std::end(m_signal) - m_history_size, std::end(m_signal), std::begin(m_signal));
	m_invalid_buffers_count++;
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

void HrtfXapoEffect::convolve(const UINT32 frame_index, DirectionData& hrtf_data, float& output)
{
	const UINT32 start_signal_index = m_history_size + frame_index - hrtf_data.delay;

	_ASSERT(static_cast<int>(start_signal_index) - static_cast<int>(hrtf_data.impulse_response.size()) > 0);

#if 1

	const UINT32 whole_pack_count = hrtf_data.impulse_response.size() / 4;
	const UINT32 remainder_count = hrtf_data.impulse_response.size() % 4;

	__m128 packed_sum = _mm_setzero_ps();
	for (UINT32 i = 0; i < whole_pack_count; i++)
	{
		__m128 reversed_signal = _mm_loadu_ps(&m_signal[start_signal_index - (i * 4) - 3]);
		__m128 signal = _mm_shuffle_ps(reversed_signal, reversed_signal, _MM_SHUFFLE(0, 1, 2, 3));
		__m128 response = _mm_loadu_ps(&hrtf_data.impulse_response[i * 4]);
		packed_sum = _mm_add_ps(packed_sum, _mm_mul_ps(signal, response));
	}
	packed_sum = _mm_hadd_ps(_mm_hadd_ps(packed_sum, packed_sum), packed_sum);
	float sum = _mm_cvtss_f32(packed_sum);

	for (UINT32 i = 0; i < remainder_count; i++)
	{
		sum += m_signal[start_signal_index - (i + whole_pack_count * 4)] * hrtf_data.impulse_response[i + whole_pack_count * 4];
	}
	output = sum;
#else
	float sum = 0.0f;
	for (UINT32 i = 0; i < hrtf_data.impulse_response.size(); i++)
	{
		sum += m_signal[start_signal_index - i] * hrtf_data.impulse_response[i];
	}
	output = sum;
#endif
}

void HrtfXapoEffect::process_frame(float& left_output, float& right_output, const UINT32 frame_index)
{
	convolve(frame_index, m_hrtf_data_left, left_output);
	convolve(frame_index, m_hrtf_data_right, right_output);
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

	if (IsEnabled && m_hrtf_data != nullptr)
	{
		m_hrtf_data->get_direction_data(params->elevation, params->azimuth, params->distance, m_hrtf_data_left, m_hrtf_data_right);

		if (is_input_valid)
		{
			pOutputProcessParameters[0].BufferFlags = XAPO_BUFFER_VALID;
			pOutputProcessParameters[0].ValidFrameCount = input_frame_count;

			process_valid_buffer(pInput, pOutput, input_frame_count, *params);
		}
		else if (has_history())
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

bool HrtfXapoEffect::has_history() const
{
	return m_invalid_buffers_count < m_buffers_per_history;
}
