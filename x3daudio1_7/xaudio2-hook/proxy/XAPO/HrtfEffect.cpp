#include "stdafx.h"

#include "HrtfEffect.h"
#include <limits>
#include <algorithm>

XAPO_REGISTRATION_PROPERTIES HrtfXapoEffect::m_regProps = {
	__uuidof(HrtfXapoEffect),
	L"HRTF Effect",
	L"Copyright (C)2015 Roman Zavalov et al.",
	1,
	0,
	XAPO_FLAG_FRAMERATE_MUST_MATCH | XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH,
	1, 1, 1, 1 };

HrtfXapoEffect * HrtfXapoEffect::CreateInstance()
{
	auto instance = new HrtfXapoEffect();
	instance->Initialize(nullptr, 0);
	return instance;
}

HrtfXapoEffect::HrtfXapoEffect() :
	CXAPOParametersBase(&m_regProps, reinterpret_cast<BYTE*>(m_params), sizeof(HrtfXapoParam), FALSE)
{
}

HRESULT HrtfXapoEffect::LockForProcess(UINT32 InputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS * pInputLockedParameters, UINT32 OutputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS * pOutputLockedParameters)
{
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

void HrtfXapoEffect::Process(UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS * pInputProcessParameters, UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS * pOutputProcessParameters, BOOL IsEnabled)
{
	_ASSERT(IsLocked());
	_ASSERT(InputProcessParameterCount == 1);
	_ASSERT(OutputProcessParameterCount == 1);
	_ASSERT(pInputProcessParameters[0].pBuffer != pOutputProcessParameters[0].pBuffer);

	auto params = CXAPOParametersBase::BeginProcess();

	if (pInputProcessParameters[0].BufferFlags == XAPO_BUFFER_SILENT)
	{
		memset(pOutputProcessParameters[0].pBuffer, 0, pInputProcessParameters[0].ValidFrameCount * m_input_format.nChannels * m_input_format.wBitsPerSample / 8);
	}
	else
	{
		if (m_input_format.wBitsPerSample == 16 && m_input_format.nChannels == 1)
		{
			auto pInput = reinterpret_cast<INT16*>(pInputProcessParameters[0].pBuffer);
			auto pOutput = reinterpret_cast<INT16*>(pOutputProcessParameters[0].pBuffer);

			// assume output is always stereo
			for (size_t i = 0; i < pInputProcessParameters[0].ValidFrameCount; i++)
			{
				pOutput[i * 2 + 0] = pInput[i];
				pOutput[i * 2 + 1] = -std::max(INT16(std::numeric_limits<INT16>::min() + INT16(1)), pInput[i]); // clamp
			}
		}
		else if (m_input_format.wBitsPerSample == 16 && m_input_format.nChannels == 2)
		{
			auto pInput = reinterpret_cast<INT16*>(pInputProcessParameters[0].pBuffer);
			auto pOutput = reinterpret_cast<INT16*>(pOutputProcessParameters[0].pBuffer);

			// assume output is always stereo
			for (size_t i = 0; i < pInputProcessParameters[0].ValidFrameCount; i++)
			{
				pOutput[i * 2 + 0] = pInput[i * 2 + 0];
				pOutput[i * 2 + 1] = -std::max(INT16(std::numeric_limits<INT16>::min() + INT16(1)), pInput[i * 2 + 1]); // clamp
			}
		}
	}

	CXAPOParametersBase::EndProcess();
}
