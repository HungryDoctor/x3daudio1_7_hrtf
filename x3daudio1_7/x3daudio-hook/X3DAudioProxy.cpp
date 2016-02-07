#include "stdafx.h"
#include "X3DAudioProxy.h"
#include "interop/ChannelMatrixMagic.h"

#include "math/math_types.h"

#include "logger.h"
#include <limits>

X3DAudioProxy::X3DAudioProxy(const x3daudio1_7_dll & original, ISound3DRegistry * registry, UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound)
	: m_original(original)
	, m_registry(registry)
{
	m_original.X3DAudioInitialize(SpeakerChannelMask, SpeedOfSound, m_handle);
}


void X3DAudioProxy::X3DAudioCalculate(const X3DAUDIO_LISTENER * pListener, const X3DAUDIO_EMITTER * pEmitter, UINT32 Flags, X3DAUDIO_DSP_SETTINGS * pDSPSettings)
{
	m_original.X3DAudioCalculate(m_handle, pListener, pEmitter, Flags, pDSPSettings);

	// changing left-hand to ortodox right-hand
	math::vector3 listener_position(pListener->Position.x, pListener->Position.y, pListener->Position.z);
	math::vector3 emitter_position(pEmitter->Position.x, pEmitter->Position.y, pEmitter->Position.z);

	auto listener_to_emitter = emitter_position - listener_position;

	math::vector3 listener_frame_front(pListener->OrientFront.x, pListener->OrientFront.y, pListener->OrientFront.z);
	math::vector3 listener_frame_up(pListener->OrientTop.x, pListener->OrientTop.y, pListener->OrientTop.z);
	auto listener_frame_right = math::cross(listener_frame_up, listener_frame_front);

	math::matrix3 listener_basis(listener_frame_right, listener_frame_up, listener_frame_front);
	auto world_to_listener_matrix = (listener_basis);

	Sound3DEntry entry;

	auto relative_position = world_to_listener_matrix * listener_to_emitter;
	auto distance = math::length(listener_to_emitter);

	entry.volume_multiplier = sample_volume_curve(pEmitter, distance);
	entry.azimuth = std::atan2(relative_position[0], relative_position[2]);
	entry.elevation = std::asin(math::normalize(relative_position)[1]);
	entry.distance = distance;

	auto id = m_registry->CreateEntry(entry);

	embed_sound_id(pDSPSettings->pMatrixCoefficients, pDSPSettings->SrcChannelCount, pDSPSettings->DstChannelCount, id);
}

float X3DAudioProxy::sample_volume_curve(const X3DAUDIO_EMITTER* pEmitter, float distance)
{
	float normalized_distance = distance / pEmitter->CurveDistanceScaler;

	if (pEmitter->pVolumeCurve == nullptr)
	{
		float clamped_distance = std::max(1.0f, normalized_distance);
		return 1.0f / clamped_distance;
	}

	if (pEmitter->pVolumeCurve->PointCount == 0)
	{
		logger::log("Warning: no points in the volume curve");
		return 0;
	}

	auto * greater_point = std::find_if(pEmitter->pVolumeCurve->pPoints, pEmitter->pVolumeCurve->pPoints + pEmitter->pVolumeCurve->PointCount, [=](const auto & point) { return point.Distance >= normalized_distance; });
	if (greater_point != pEmitter->pVolumeCurve->pPoints + pEmitter->pVolumeCurve->PointCount)
	{
		if (greater_point > &pEmitter->pVolumeCurve->pPoints[0])
		{
			auto * less_point = greater_point - 1;

			float fraction = (normalized_distance - less_point->Distance) / (greater_point->Distance - less_point->Distance);
			return less_point->DSPSetting + (greater_point->DSPSetting - less_point->DSPSetting) * fraction;
		}
		else
		{
			return greater_point->DSPSetting;
		}
	}
	else
	{
		return (greater_point - 1)->DSPSetting;
	}
}

const X3DAUDIO_HANDLE& X3DAudioProxy::GetHandle() const
{
	return m_handle;
}
