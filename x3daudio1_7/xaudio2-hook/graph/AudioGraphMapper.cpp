#include "stdafx.h"

#include "AudioGraphMapper.h"

#include "proxy/XAudio2VoiceProxy.h"
#include "proxy/XAudio2MasteringVoiceProxy.h"
#include "proxy/XAudio2SourceVoiceProxy.h"
#include "proxy/XAudio2SubmixVoiceProxy.h"

#include "util.h"
#include "common_types.h"
#include <vector>

AudioGraphMapper::AudioGraphMapper(IXAudio2* xaudio)
	: m_xaudio(xaudio)
	, m_masteringVoice(nullptr)
{
}

AudioGraphMapper::~AudioGraphMapper()
{
}



IXAudio2SourceVoice* AudioGraphMapper::CreateSourceVoice(const WAVEFORMATEX* pSourceFormat, UINT32 Flags, float MaxFrequencyRatio, IXAudio2VoiceCallback* pCallback, const XAUDIO2_VOICE_SENDS* pSendList, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	auto proxyVoice = new XAudio2SourceVoiceProxy(pSourceFormat->nChannels, pSourceFormat->nSamplesPerSec, Flags, MaxFrequencyRatio, pCallback, -1, from_XAUDIO2_VOICE_SENDS(pSendList), from_XAUDIO2_EFFECT_CHAIN(pEffectChain));

	IXAudio2SourceVoice * actualVoiceRawPtr;
	VoiceSends actual_sends = mapProxySendsToActualOnes(from_XAUDIO2_VOICE_SENDS(pSendList));
	auto actualSendsStruct = from_voice_sends(actual_sends);
	m_xaudio->CreateSourceVoice(&actualVoiceRawPtr, pSourceFormat, Flags, MaxFrequencyRatio, pCallback, &actualSendsStruct, pEffectChain);
	std::shared_ptr<IXAudio2SourceVoice> actualVoice(actualVoiceRawPtr, [](IXAudio2SourceVoice * voice) { voice->DestroyVoice(); });


	VoiceDescriptor desc;
	desc.proxyVoice = proxyVoice;
	desc.actualVoice = actualVoice.get();
	m_voices.insert(std::make_pair(proxyVoice, desc));

	proxyVoice->stateGetter = [actualVoice](XAudio2SourceVoiceProxy* source, XAUDIO2_VOICE_STATE& out_state)
	{
		actualVoice->GetState(&out_state);
	};

	proxyVoice->onStart = [actualVoice](XAudio2VoiceProxy * source, UINT32 flags, UINT32 operationSet)
	{
		actualVoice->Start(flags, operationSet);
	};

	proxyVoice->onStop = [actualVoice](XAudio2VoiceProxy * source, UINT32 flags, UINT32 operationSet)
	{
		actualVoice->Stop(flags, operationSet);
	};

	proxyVoice->onSubmitSourceBuffer = [actualVoice](XAudio2SourceVoiceProxy * source, const XAUDIO2_BUFFER * pBuffer, const XAUDIO2_BUFFER_WMA * pBufferWma)
	{
		actualVoice->SubmitSourceBuffer(pBuffer, pBufferWma);
	};

	proxyVoice->onFlushSourceBuffers = [actualVoice](XAudio2SourceVoiceProxy * source)
	{
		actualVoice->FlushSourceBuffers();
	};

	proxyVoice->onDiscontinuity = [actualVoice](XAudio2SourceVoiceProxy * source)
	{
		actualVoice->Discontinuity();
	};

	proxyVoice->onExitLoop = [actualVoice](XAudio2SourceVoiceProxy * source, UINT32 operationSet)
	{
		actualVoice->ExitLoop(operationSet);
	};

	proxyVoice->onSetFrequencyRatio = [actualVoice](XAudio2SourceVoiceProxy * source, UINT32 operationSet)
	{
		actualVoice->SetFrequencyRatio(source->getFrequencyRatio());
	};

	proxyVoice->onSetSourceSampleRate = [actualVoice](XAudio2SourceVoiceProxy * source)
	{
		actualVoice->SetSourceSampleRate(source->getInputSampleRate());
	};

	setupCommonCallbacks(proxyVoice, actualVoice);

	return proxyVoice;
}

IXAudio2SubmixVoice* AudioGraphMapper::CreateSubmixVoice(UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const XAUDIO2_VOICE_SENDS* pSendList, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	auto proxyVoice = new XAudio2SubmixVoiceProxy(InputChannels, InputSampleRate, Flags, ProcessingStage, from_XAUDIO2_VOICE_SENDS(pSendList), from_XAUDIO2_EFFECT_CHAIN(pEffectChain));

	IXAudio2SubmixVoice * actualVoiceRawPtr;
	m_xaudio->CreateSubmixVoice(&actualVoiceRawPtr, InputChannels, InputSampleRate, Flags, ProcessingStage, pSendList, pEffectChain);
	std::shared_ptr<IXAudio2SubmixVoice> actualVoice(actualVoiceRawPtr, [](IXAudio2SubmixVoice * voice) { voice->DestroyVoice(); });

	VoiceDescriptor desc;
	desc.proxyVoice = proxyVoice;
	desc.actualVoice = actualVoice.get();
	m_voices.insert(std::make_pair(proxyVoice, desc));

	setupCommonCallbacks(proxyVoice, actualVoice);

	return proxyVoice;
}

IXAudio2MasteringVoice* AudioGraphMapper::CreateMasteringVoice(UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	auto proxyVoice = new XAudio2MasteringVoiceProxy(InputChannels, InputSampleRate, Flags, DeviceIndex, from_XAUDIO2_EFFECT_CHAIN(pEffectChain));

	IXAudio2MasteringVoice * actualVoiceRawPtr;
	m_xaudio->CreateMasteringVoice(&actualVoiceRawPtr, InputChannels, InputSampleRate, Flags, DeviceIndex, pEffectChain);
	std::shared_ptr<IXAudio2MasteringVoice> actualVoice(actualVoiceRawPtr, [](IXAudio2MasteringVoice * voice) { voice->DestroyVoice(); });

	VoiceDescriptor desc;
	desc.proxyVoice = proxyVoice;
	desc.actualVoice = actualVoice.get();
	m_voices.insert(std::make_pair(proxyVoice, desc));

	setupCommonCallbacks(proxyVoice, actualVoice);

	m_masteringVoice = &m_voices[proxyVoice];
	return proxyVoice;
}

VoiceSends AudioGraphMapper::mapProxySendsToActualOnes(const VoiceSends & proxySends) const
{
	voice_sends actualSends(proxySends.getSends().size());
	std::transform(std::begin(proxySends.getSends()), std::end(proxySends.getSends()), std::begin(actualSends),
		[&](const XAUDIO2_SEND_DESCRIPTOR & sendDesc)
	    {
		    XAUDIO2_SEND_DESCRIPTOR actualSendDesc;
		    actualSendDesc.Flags = sendDesc.Flags;
		    actualSendDesc.pOutputVoice = getDescriptorForProxyVoice(sendDesc.pOutputVoice).actualVoice;
		    return actualSendDesc;
	    });

	return VoiceSends(actualSends);
}



void AudioGraphMapper::setupCommonCallbacks(XAudio2VoiceProxy* proxyVoice, const std::shared_ptr<IXAudio2Voice> & actualVoice)
{
	proxyVoice->onSetOutputVoices = [this, actualVoice](XAudio2VoiceProxy * source, matrices_map & outputMatrices, filter_params_map & outputFilterParams)
	{
		if (!source->getSends().getSendToMasterOnly())
		{
			VoiceSends proxySends = source->getSends();
			VoiceSends actual_sends = mapProxySendsToActualOnes(proxySends);
			auto actualSendsStruct = from_voice_sends(actual_sends);
			actualVoice->SetOutputVoices(&actualSendsStruct);

			// TODO : replace stub impementation with a normal one
			outputMatrices.clear();
			for (const auto & sendDesc : proxySends.getSends())
			{
				float vals[1]{ 0 };
				outputMatrices.insert(std::make_pair(sendDesc.pOutputVoice, ChannelMatrix(vals, 1, 1)));
				//actualVoice->GetOutputMatrix(getDescriptorForProxyVoice(sendDesc.pOutputVoice).actualVoice, );
			}

			outputFilterParams.clear();
			for (const auto & sendDesc : proxySends.getSends())
			{
				XAUDIO2_FILTER_PARAMETERS params;
				outputFilterParams.insert(std::make_pair(sendDesc.pOutputVoice, params));
			}
		}
		else
		{
			actualVoice->SetOutputVoices(nullptr);

			float vals[1]{ 0 };
			outputMatrices.insert(std::make_pair(m_masteringVoice->proxyVoice, ChannelMatrix(vals, 1, 1)));

			XAUDIO2_FILTER_PARAMETERS params;
			outputFilterParams.insert(std::make_pair(m_masteringVoice->proxyVoice, params));
		}

		
	};

	proxyVoice->onSetEffectChain = [actualVoice](XAudio2VoiceProxy * source)
	{
		auto chain = source->getEffectChain();
		if (chain.size() > 0)
		{
			XAUDIO2_EFFECT_CHAIN chainStruct;
			chainStruct.EffectCount = static_cast<UINT32>(chain.size());
			chainStruct.pEffectDescriptors = &*chain.begin();
			actualVoice->SetEffectChain(&chainStruct);
		}
		else
		{
			actualVoice->SetEffectChain(nullptr);
		}
	};

	proxyVoice->onSetEffectEnabled = [actualVoice](XAudio2VoiceProxy * source, UINT32 effectIndex, UINT32 operationSet)
	{
		if (source->getIsEffectEnabled(effectIndex))
			actualVoice->EnableEffect(effectIndex, operationSet);
		else
			actualVoice->DisableEffect(effectIndex, operationSet);
	};

	proxyVoice->onSetEffectParameters = [actualVoice](XAudio2VoiceProxy * source, UINT32 effectIndex, UINT32 operationSet)
	{
		auto params = source->getEffectParameters(effectIndex);
		actualVoice->SetEffectParameters(effectIndex, &*params.begin(), static_cast<UINT32>(params.size()), operationSet);
	};

	proxyVoice->onSetFilterParameters = [actualVoice](XAudio2VoiceProxy * source, UINT32 operationSet)
	{
		auto params = source->getFilterParameters();
		actualVoice->SetFilterParameters(&params, operationSet);
	};

	proxyVoice->onSetOutputFilterParameters = [this, actualVoice](XAudio2VoiceProxy * source, IXAudio2Voice * pDestination, UINT32 operationSet)
	{
		auto params = source->getOutputFilterParameters(pDestination);
		actualVoice->SetOutputFilterParameters(getDescriptorForProxyVoice(pDestination).actualVoice, &params, operationSet);
	};

	proxyVoice->onSetVolume = [actualVoice](XAudio2VoiceProxy * source, UINT32 operationSet)
	{
		actualVoice->SetVolume(source->getVolume(), operationSet);
	};

	proxyVoice->onSetChannelVolumes = [actualVoice](XAudio2VoiceProxy * source, UINT32 operationSet)
	{
		auto volumes = source->getChannelVolumes();
		actualVoice->SetChannelVolumes(static_cast<UINT32>(volumes.size()), &*volumes.begin(), operationSet);
	};

	proxyVoice->onSetOutputMatrix = [this, actualVoice](XAudio2VoiceProxy * source, IXAudio2Voice * pDestination, UINT32 operationSet)
	{
		auto matrix = source->getOutputMatrix(pDestination);
		std::vector<float> values;
		values.resize(matrix.getSourceCount() * matrix.getDestinationCount());
		from_ChannelMatrix(matrix, matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());
		if (pDestination)
		{
			actualVoice->SetOutputMatrix(getDescriptorForProxyVoice(pDestination).actualVoice, matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());
		}
		else if (source->getSends().getSendToMasterOnly())
		{
			actualVoice->SetOutputMatrix(nullptr, matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());
		}
		else if (source->getSends().getSends().size() == 1)
		{
			actualVoice->SetOutputMatrix(getDescriptorForProxyVoice(source->getSends().getSends()[0].pOutputVoice).actualVoice, matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());
		}
		else
		{
			throw std::logic_error("pDestination == nullptr but there's multiple output voices");
		}
	};

	proxyVoice->onDestroyVoice = [this, actualVoice](XAudio2VoiceProxy * source)
	{
		//actualVoice->DestroyVoice(); // actual voice will be destroyed by shared_ptr when all references will be released.
		// TODO : improve, get rid of dynamic_cast
		m_voices.erase(m_voices.find(dynamic_cast<IXAudio2Voice*>(source)));
		delete source;
	};
}

const VoiceDescriptor & AudioGraphMapper::getDescriptorForProxyVoice(IXAudio2Voice* pDestination) const
{
	return m_voices.at(pDestination);
}

VoiceDescriptor& AudioGraphMapper::getDescriptorForProxyVoice(IXAudio2Voice* pDestination)
{
	return m_voices.at(pDestination);
}
