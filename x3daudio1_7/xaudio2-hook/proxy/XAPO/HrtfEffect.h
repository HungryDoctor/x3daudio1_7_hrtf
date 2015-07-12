#pragma once

#define NOMINMAX
#include <xapobase.h>

#include "math/math_types.h"

struct HrtfXapoParam
{
	// source to emitter vector in listener's local space. Z is front, Y is Up, and X is left
	math::vector3 source_to_emitter_transformed;

	float volume_multiplier;
};


class __declspec(uuid("{80D4BED4-7605-4E4C-B29C-5579C375C599}")) HrtfXapoEffect : public CXAPOParametersBase
{
public:
	static HrtfXapoEffect* CreateInstance();

public:
	HrtfXapoEffect();

	// Inherited via CXAPOParametersBase
	STDMETHOD(LockForProcess)(UINT32 InputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS * pInputLockedParameters, UINT32 OutputLockedParameterCount, const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS * pOutputLockedParameters) override;
	STDMETHOD_(void, Process)(UINT32 InputProcessParameterCount, const XAPO_PROCESS_BUFFER_PARAMETERS * pInputProcessParameters, UINT32 OutputProcessParameterCount, XAPO_PROCESS_BUFFER_PARAMETERS * pOutputProcessParameters, BOOL IsEnabled) override;

private:
	bool is_charged() const;
	void process_valid_buffer(const float * pInput, float * pOutput, const UINT32 frame_count, const HrtfXapoParam & params);
	void process_invalid_buffer(float * pOutput, const UINT32 frames_to_write_count, UINT32 & valid_frames_counter, const HrtfXapoParam & params);
	void bypass(const float * pInput, float * pOutput, const UINT32 frame_count, const bool is_valid);

	void process_frame(const float left_input, const float right_input, float & left_output, float & right_output);
	static XAPO_REGISTRATION_PROPERTIES m_regProps;

	WAVEFORMATEX m_input_format;
	WAVEFORMATEX m_output_format;
	HrtfXapoParam m_params[3]; // ring buffer as CXAPOParametersBase requires
	float m_time_per_frame;

	float m_charge[2];

	const float left_param = 10000.0f;
	const float right_param = 10000.0f;
};
