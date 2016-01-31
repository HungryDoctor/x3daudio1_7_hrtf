#include "stdafx.h"
#include "XAudio2SourceVoiceProxy.h"

#include "util.h"


XAudio2SourceVoiceProxy::XAudio2SourceVoiceProxy(UINT32 inputChannels, UINT32 inputSampleRate, UINT32 flags, float maxFrequencyRatio, IXAudio2VoiceCallback * pCallback, UINT32 processingStage, const VoiceSends & sends, const effect_chain & chain)
	: XAudio2VoiceProxy(inputChannels, inputSampleRate, flags, processingStage, sends, chain)
	, m_maxFrequencyRatio(maxFrequencyRatio)
	, m_frequencyRatio(std::min(maxFrequencyRatio, 1.0f))
	, m_voiceCallback(pCallback)
{
	
}


XAudio2SourceVoiceProxy::~XAudio2SourceVoiceProxy()
{
	
}

HRESULT XAudio2SourceVoiceProxy::Start(UINT32 Flags, UINT32 OperationSet)
{
	onStart(this, Flags, OperationSet);
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::Stop(UINT32 Flags, UINT32 OperationSet)
{
	onStop(this, Flags, OperationSet);
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::SubmitSourceBuffer(const XAUDIO2_BUFFER * pBuffer, const XAUDIO2_BUFFER_WMA * pBufferWMA)
{
	onSubmitSourceBuffer(this, pBuffer, pBufferWMA);
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::FlushSourceBuffers()
{
	onFlushSourceBuffers(this);
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::Discontinuity()
{
	onDiscontinuity(this);
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::ExitLoop(UINT32 OperationSet)
{
	onExitLoop(this, OperationSet);
	return S_OK;
}

void XAudio2SourceVoiceProxy::GetState(XAUDIO2_VOICE_STATE * pVoiceState)
{
	stateGetter(this, *pVoiceState);
}

HRESULT XAudio2SourceVoiceProxy::SetFrequencyRatio(float Ratio, UINT32 OperationSet)
{
	m_frequencyRatio = Ratio;
	onSetFrequencyRatio(this, OperationSet);
	return S_OK;
}

void XAudio2SourceVoiceProxy::GetFrequencyRatio(float * pRatio)
{
	*pRatio = m_frequencyRatio;
}

HRESULT XAudio2SourceVoiceProxy::SetSourceSampleRate(UINT32 NewSourceSampleRate)
{
	m_inputSampleRate = NewSourceSampleRate;
	onSetSourceSampleRate(this);
	return S_OK;
}

// End Source-specific

// Common part

void XAudio2SourceVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS * pVoiceDetails)
{
	pVoiceDetails->InputChannels = getInputChannels();
	pVoiceDetails->CreationFlags = getFlags();
	pVoiceDetails->InputSampleRate = getInputSampleRate();
}

HRESULT XAudio2SourceVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS * pSendList)
{
	setOutputVoices(from_XAUDIO2_VOICE_SENDS(pSendList));
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN * pEffectChain)
{
	setEffectChain(from_XAUDIO2_EFFECT_CHAIN(pEffectChain));
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	setEffectEnabled(EffectIndex, true, OperationSet);
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	setEffectEnabled(EffectIndex, false, OperationSet);
	return S_OK;
}

void XAudio2SourceVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL * pEnabled)
{
	*pEnabled = getIsEffectEnabled(EffectIndex);
}

HRESULT XAudio2SourceVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void * pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	setEffectParameters(EffectIndex, buffer_to_vector(pParameters, ParametersByteSize), OperationSet);
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void * pParameters, UINT32 ParametersByteSize)
{
	vector_to_buffer(getEffectParameters(EffectIndex), pParameters, ParametersByteSize);
	return S_OK;
}

HRESULT XAudio2SourceVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet)
{
	setFilterParameters(*pParameters, OperationSet);
	return S_OK;
}

void XAudio2SourceVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS * pParameters)
{
	*pParameters = getFilterParameters();
}

HRESULT XAudio2SourceVoiceProxy::SetOutputFilterParameters(IXAudio2Voice * pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet)
{
	setOutputFilterParameters(pDestinationVoice, *pParameters, OperationSet);
	return S_OK;
}

void XAudio2SourceVoiceProxy::GetOutputFilterParameters(IXAudio2Voice * pDestinationVoice, XAUDIO2_FILTER_PARAMETERS * pParameters)
{
	*pParameters = getOutputFilterParameters(pDestinationVoice);
}

HRESULT XAudio2SourceVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	setVolume(Volume, OperationSet);
	return S_OK;
}

void XAudio2SourceVoiceProxy::GetVolume(float * pVolume)
{
	*pVolume = getVolume();
}

HRESULT XAudio2SourceVoiceProxy::SetChannelVolumes(UINT32 Channels, const float * pVolumes, UINT32 OperationSet)
{
	setChannelVolumes(buffer_to_vector(pVolumes, Channels), OperationSet);
	return S_OK;
}

void XAudio2SourceVoiceProxy::GetChannelVolumes(UINT32 Channels, float * pVolumes)
{
	vector_to_buffer(getChannelVolumes(), pVolumes, Channels);
}

HRESULT XAudio2SourceVoiceProxy::SetOutputMatrix(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float * pLevelMatrix, UINT32 OperationSet)
{
	setOutputMatrix(pDestinationVoice, ChannelMatrix(pLevelMatrix, SourceChannels, DestinationChannels), OperationSet);
	return S_OK;
}

void XAudio2SourceVoiceProxy::GetOutputMatrix(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float * pLevelMatrix)
{
	from_ChannelMatrix(getOutputMatrix(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2SourceVoiceProxy::DestroyVoice()
{
	destroyVoice();
}
