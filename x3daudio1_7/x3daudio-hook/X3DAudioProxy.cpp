#include "stdafx.h"
#include "X3DAudioProxy.h"

#include "logger.h"
#include <limits>

X3DAudioProxy::X3DAudioProxy(const x3daudio1_7_dll & original, ISound3DRegistry * registry, UINT32 SpeakerChannelMask, FLOAT32 SpeedOfSound)
	: m_original(original)
	, m_registry(registry)
{
	m_original.X3DAudioInitialize(SpeakerChannelMask, SpeedOfSound, m_handle);
}


X3DAudioProxy::~X3DAudioProxy()
{
}

void X3DAudioProxy::X3DAudioCalculate(const X3DAUDIO_LISTENER * pListener, const X3DAUDIO_EMITTER * pEmitter, UINT32 Flags, X3DAUDIO_DSP_SETTINGS * pDSPSettings)
{
	logger::log("X3DAudioProxy::X3DAudioCalculate");
	
	m_original.X3DAudioCalculate(m_handle, pListener, pEmitter, Flags, pDSPSettings);

	Sound3DEntry entry;

	entry.matrix_coefficients[0] = pDSPSettings->pMatrixCoefficients[0];
	entry.matrix_coefficients[1] = pDSPSettings->pMatrixCoefficients[1];

	entry.relative_position[0] = pListener->Position.x - pEmitter->Position.x;
	entry.relative_position[1] = pListener->Position.y - pEmitter->Position.y;
	entry.relative_position[2] = pListener->Position.z - pEmitter->Position.z;

	auto id = m_registry->CreateEntry(entry);

	pDSPSettings->pMatrixCoefficients[0] = std::numeric_limits<FLOAT32>::quiet_NaN(); // flagging as hacked
	pDSPSettings->pMatrixCoefficients[1] = *reinterpret_cast<FLOAT32*>(&id);
}

const X3DAUDIO_HANDLE & X3DAudioProxy::GetHandle() const
{
	return m_handle;
}
