#include "stdafx.h"
#include "XAudio2VoiceProxy.h"

XAudio2VoiceProxy::XAudio2VoiceProxy(UINT32 inputChannels, UINT32 inputSampleRate, UINT32 flags, UINT32 processingStage, const std::vector<XAUDIO2_SEND_DESCRIPTOR>& sends, const std::vector<XAUDIO2_EFFECT_DESCRIPTOR>& chain)
	: m_inputSampleRate(inputSampleRate)
	, m_inputChannels(inputChannels)
	, m_flags(flags)
	, m_processingStage(processingStage)
	, m_sends(sends)
	, m_effectChain(chain)
	, m_volume(1.0f)
{
}

XAudio2VoiceProxy::~XAudio2VoiceProxy()
{
}

UINT32 XAudio2VoiceProxy::getInputChannels() const
{
	return m_inputChannels;
}

UINT32 XAudio2VoiceProxy::getInputSampleRate() const
{
	return m_inputSampleRate;
}

UINT32 XAudio2VoiceProxy::getFlags() const
{
	return m_flags;
}

void XAudio2VoiceProxy::setOutputVoices(const std::vector<XAUDIO2_SEND_DESCRIPTOR>& sends)
{
	m_sends = sends;
	onSetOutputVoices(this, m_outputMatrices, m_outputFilterParameters);
}

void XAudio2VoiceProxy::setEffectChain(const std::vector<XAUDIO2_EFFECT_DESCRIPTOR>& chain)
{
	m_effectChain = chain;
	m_effectActivity.resize(m_effectChain.size());
	std::transform(std::begin(m_effectChain), std::end(m_effectChain), std::begin(m_effectActivity), [](const auto & effect) { return effect.InitialState == TRUE; });

	// we don't know anything about parameters. Maybe we should ask effect about it. But we don't know the size of them to ask.
	// let's hope, client app will not ask them, for now.
	std::transform(std::begin(m_effectChain), std::end(m_effectChain), std::begin(m_effectParameters), [](const auto & effect) { return std::vector<int8_t>(); });
	onSetEffectChain(this);
}

void XAudio2VoiceProxy::setEffectEnabled(UINT32 effectIndex, bool isEnabled, UINT32 operationSet)
{
	m_effectActivity[effectIndex] = isEnabled;
	onSetEffectEnabled(this, effectIndex, operationSet);
}

bool XAudio2VoiceProxy::getIsEffectEnabled(UINT32 effectIndex) const
{
	return m_effectActivity[effectIndex];
}

void XAudio2VoiceProxy::setEffectParameters(UINT32 effectIndex, const std::vector<int8_t>& parameters, UINT32 operationSet)
{
	m_effectParameters[effectIndex] = parameters;
	onSetEffectParameters(this, effectIndex, operationSet);
}

const std::vector<int8_t>& XAudio2VoiceProxy::getEffectParameters(UINT32 effectIndex) const
{
	return m_effectParameters[effectIndex];
}

void XAudio2VoiceProxy::setFilterParameters(const XAUDIO2_FILTER_PARAMETERS& parameters, UINT32 operationSet)
{
	m_filterParameters = parameters;
	onSetFilterParameters(this, operationSet);
}

XAUDIO2_FILTER_PARAMETERS XAudio2VoiceProxy::getFilterParameters() const
{
	return m_filterParameters;
}

void XAudio2VoiceProxy::setOutputFilterParameters(IXAudio2Voice* pDestinationVoice, const XAUDIO2_FILTER_PARAMETERS& parameters, UINT32 operationSet)
{
	m_outputFilterParameters[pDestinationVoice] = parameters;
	onSetFilterParameters(this, operationSet);
}

XAUDIO2_FILTER_PARAMETERS XAudio2VoiceProxy::getOutputFilterParameters(IXAudio2Voice* pDestinationVoice) const
{
	return m_outputFilterParameters.at(pDestinationVoice);
}

void XAudio2VoiceProxy::setVolume(float volume, UINT32 operationSet)
{
	m_volume = volume;
	onSetVolume(this, operationSet);
}

float XAudio2VoiceProxy::getVolume() const
{
	return m_volume;
}

void XAudio2VoiceProxy::setChannelVolumes(const std::vector<float> & volumes, UINT32 operationSet)
{
	m_channelVolumes = volumes;
	onSetChannelVolumes(this, operationSet);
}

std::vector<float> XAudio2VoiceProxy::getChannelVolumes() const
{
	return m_channelVolumes;
}

void XAudio2VoiceProxy::setOutputMatrix(IXAudio2Voice* pDestinationVoice, const ChannelMatrix& matrix, UINT32 operationSet)
{
	m_outputMatrices.insert(std::make_pair(pDestinationVoice, matrix));

	onSetOutputMatrix(this, pDestinationVoice, operationSet);
}

ChannelMatrix XAudio2VoiceProxy::getOutputMatrix(IXAudio2Voice* pDestinationVoice) const
{
	return m_outputMatrices.at(pDestinationVoice);
}

void XAudio2VoiceProxy::destroyVoice()
{
	onDestroyVoice(this);
}
