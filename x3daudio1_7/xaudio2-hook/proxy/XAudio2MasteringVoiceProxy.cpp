#include "stdafx.h"
#include "XAudio2MasteringVoiceProxy.h"

#include "util.h"
#include <limits>


XAudio2MasteringVoiceProxy::XAudio2MasteringVoiceProxy(UINT32 inputChannels, UINT32 inputSampleRate, UINT32 flags, UINT32 deviceIndex, const effect_chain& effectChain)
	: XAudio2VoiceProxy(inputChannels, inputSampleRate, flags,std::numeric_limits<int>::max(), VoiceSends(), effectChain)
	, m_deviceIndex(deviceIndex)
{
}

XAudio2MasteringVoiceProxy::~XAudio2MasteringVoiceProxy()
{
	
}

void XAudio2MasteringVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS * pVoiceDetails)
{
	pVoiceDetails->InputChannels = getInputChannels();
	pVoiceDetails->CreationFlags = getFlags();
	pVoiceDetails->InputSampleRate = getInputSampleRate();
}

HRESULT XAudio2MasteringVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS * pSendList)
{
	setOutputVoices(from_XAUDIO2_VOICE_SENDS(pSendList));
	return S_OK;
}

HRESULT XAudio2MasteringVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN * pEffectChain)
{
	setEffectChain(from_XAUDIO2_EFFECT_CHAIN(pEffectChain));
	return S_OK;
}

HRESULT XAudio2MasteringVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	setEffectEnabled(EffectIndex, true, OperationSet);
	return S_OK;
}

HRESULT XAudio2MasteringVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	setEffectEnabled(EffectIndex, false, OperationSet);
	return S_OK;
}

void XAudio2MasteringVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL * pEnabled)
{
	*pEnabled = getIsEffectEnabled(EffectIndex);
}

HRESULT XAudio2MasteringVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void * pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	setEffectParameters(EffectIndex, buffer_to_vector(pParameters, ParametersByteSize), OperationSet);
	return S_OK;
}

HRESULT XAudio2MasteringVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void * pParameters, UINT32 ParametersByteSize)
{
	vector_to_buffer(getEffectParameters(EffectIndex), pParameters, ParametersByteSize);
	return S_OK;
}

HRESULT XAudio2MasteringVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet)
{
	setFilterParameters(*pParameters, OperationSet);
	return S_OK;
}

void XAudio2MasteringVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS * pParameters)
{
	*pParameters = getFilterParameters();
}

HRESULT XAudio2MasteringVoiceProxy::SetOutputFilterParameters(IXAudio2Voice * pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet)
{
	setOutputFilterParameters(pDestinationVoice, *pParameters, OperationSet);
	return S_OK;
}

void XAudio2MasteringVoiceProxy::GetOutputFilterParameters(IXAudio2Voice * pDestinationVoice, XAUDIO2_FILTER_PARAMETERS * pParameters)
{
	*pParameters = getOutputFilterParameters(pDestinationVoice);
}

HRESULT XAudio2MasteringVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	setVolume(Volume, OperationSet);
	return S_OK;
}

void XAudio2MasteringVoiceProxy::GetVolume(float * pVolume)
{
	*pVolume = getVolume();
}

HRESULT XAudio2MasteringVoiceProxy::SetChannelVolumes(UINT32 Channels, const float * pVolumes, UINT32 OperationSet)
{
	setChannelVolumes(buffer_to_vector(pVolumes, Channels), OperationSet);
	return S_OK;
}

void XAudio2MasteringVoiceProxy::GetChannelVolumes(UINT32 Channels, float * pVolumes)
{
	vector_to_buffer(getChannelVolumes(), pVolumes, Channels);
}

HRESULT XAudio2MasteringVoiceProxy::SetOutputMatrix(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float * pLevelMatrix, UINT32 OperationSet)
{
	setOutputMatrix(pDestinationVoice, ChannelMatrix(pLevelMatrix, SourceChannels, DestinationChannels), OperationSet);
	return S_OK;
}

void XAudio2MasteringVoiceProxy::GetOutputMatrix(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float * pLevelMatrix)
{
	from_ChannelMatrix(getOutputMatrix(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2MasteringVoiceProxy::DestroyVoice()
{
	destroyVoice();
}