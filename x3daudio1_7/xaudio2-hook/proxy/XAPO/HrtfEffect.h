#pragma once

#define NOMINMAX
#include <xapobase.h>
#include <atlbase.h>
#include <atlcom.h>

struct HrtfXapoParam
{
	int payload;
	int payload2;
};


class __declspec(uuid("{80D4BED4-7605-4E4C-B29C-5579C375C599}")) HrtfXapoEffect : public CXAPOParametersBase
{
public:
	static HrtfXapoEffect * CreateInstance();

public:
	HrtfXapoEffect();

	// Inherited via CXAPOParametersBase
	STDMETHOD(LockForProcess)(UINT32 InputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters, UINT32 OutputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters) override;
	STDMETHOD_(void, Process)(UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS * pInputProcessParameters, UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS * pOutputProcessParameters, BOOL IsEnabled) override;

private:
	static XAPO_REGISTRATION_PROPERTIES m_regProps;

	WAVEFORMATEX m_input_format;
	WAVEFORMATEX m_output_format;
	HrtfXapoParam m_params[3]; // ring buffer as CXAPOParametersBase requires

	int m_sample_counter;
	float m_charge[2];
};