#include "stdafx.h"
#include "XAudio2SubmixVoiceProxy.h"

#include "util.h"


XAudio2SubmixVoiceProxy::XAudio2SubmixVoiceProxy(UINT32 inputChannels, UINT32 inputSampleRate, UINT32 flags, UINT32 processingStage, const VoiceSends & sends, const effect_chain & chain)
	: XAudio2VoiceProxy(inputChannels, inputSampleRate, flags, processingStage, sends, chain)
{

}

XAudio2SubmixVoiceProxy::~XAudio2SubmixVoiceProxy()
{
	
}

void XAudio2SubmixVoiceProxy::GetVoiceDetails(XAUDIO2_VOICE_DETAILS * pVoiceDetails)
{
	pVoiceDetails->InputChannels = getInputChannels();
	pVoiceDetails->CreationFlags = getFlags();
	pVoiceDetails->InputSampleRate = getInputSampleRate();
}

HRESULT XAudio2SubmixVoiceProxy::SetOutputVoices(const XAUDIO2_VOICE_SENDS * pSendList)
{
	setOutputVoices(from_XAUDIO2_VOICE_SENDS(pSendList));
	return S_OK;
}

HRESULT XAudio2SubmixVoiceProxy::SetEffectChain(const XAUDIO2_EFFECT_CHAIN * pEffectChain)
{
	setEffectChain(from_XAUDIO2_EFFECT_CHAIN(pEffectChain));
	return S_OK;
}

HRESULT XAudio2SubmixVoiceProxy::EnableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	setEffectEnabled(EffectIndex, true, OperationSet);
	return S_OK;
}

HRESULT XAudio2SubmixVoiceProxy::DisableEffect(UINT32 EffectIndex, UINT32 OperationSet)
{
	setEffectEnabled(EffectIndex, false, OperationSet);
	return S_OK;
}

void XAudio2SubmixVoiceProxy::GetEffectState(UINT32 EffectIndex, BOOL * pEnabled)
{
	*pEnabled = getIsEffectEnabled(EffectIndex);
}

HRESULT XAudio2SubmixVoiceProxy::SetEffectParameters(UINT32 EffectIndex, const void * pParameters, UINT32 ParametersByteSize, UINT32 OperationSet)
{
	setEffectParameters(EffectIndex, buffer_to_vector(pParameters, ParametersByteSize), OperationSet);
	return S_OK;
}

HRESULT XAudio2SubmixVoiceProxy::GetEffectParameters(UINT32 EffectIndex, void * pParameters, UINT32 ParametersByteSize)
{
	vector_to_buffer(getEffectParameters(EffectIndex), pParameters, ParametersByteSize);
	return S_OK;
}

HRESULT XAudio2SubmixVoiceProxy::SetFilterParameters(const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet)
{
	setFilterParameters(*pParameters, OperationSet);
	return S_OK;
}

void XAudio2SubmixVoiceProxy::GetFilterParameters(XAUDIO2_FILTER_PARAMETERS * pParameters)
{
	*pParameters = getFilterParameters();
}

HRESULT XAudio2SubmixVoiceProxy::SetOutputFilterParameters(IXAudio2Voice * pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS * pParameters, UINT32 OperationSet)
{
	setOutputFilterParameters(pDestinationVoice, *pParameters, OperationSet);
	return S_OK;
}

void XAudio2SubmixVoiceProxy::GetOutputFilterParameters(IXAudio2Voice * pDestinationVoice, XAUDIO2_FILTER_PARAMETERS * pParameters)
{
	*pParameters = getOutputFilterParameters(pDestinationVoice);
}

HRESULT XAudio2SubmixVoiceProxy::SetVolume(float Volume, UINT32 OperationSet)
{
	setVolume(Volume, OperationSet);
	return S_OK;
}

void XAudio2SubmixVoiceProxy::GetVolume(float * pVolume)
{
	*pVolume = getVolume();
}

HRESULT XAudio2SubmixVoiceProxy::SetChannelVolumes(UINT32 Channels, const float * pVolumes, UINT32 OperationSet)
{
	setChannelVolumes(buffer_to_vector(pVolumes, Channels), OperationSet);
	return S_OK;
}

void XAudio2SubmixVoiceProxy::GetChannelVolumes(UINT32 Channels, float * pVolumes)
{
	vector_to_buffer(getChannelVolumes(), pVolumes, Channels);
}

HRESULT XAudio2SubmixVoiceProxy::SetOutputMatrix(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, const float * pLevelMatrix, UINT32 OperationSet)
{
	
	setOutputMatrix(pDestinationVoice, ChannelMatrix(pLevelMatrix, SourceChannels, DestinationChannels), OperationSet);
	return S_OK;
}

void XAudio2SubmixVoiceProxy::GetOutputMatrix(IXAudio2Voice * pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels, float * pLevelMatrix)
{
	from_ChannelMatrix(getOutputMatrix(pDestinationVoice), SourceChannels, DestinationChannels, pLevelMatrix);
}

void XAudio2SubmixVoiceProxy::DestroyVoice()
{
	destroyVoice();
}
