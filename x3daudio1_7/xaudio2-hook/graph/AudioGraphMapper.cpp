#include "stdafx.h"

#include "AudioGraphMapper.h"

#include "proxy/XAudio2VoiceProxy.h"
#include "proxy/XAudio2MasteringVoiceProxy.h"
#include "proxy/XAudio2SourceVoiceProxy.h"
#include "proxy/XAudio2SubmixVoiceProxy.h"

#include "util.h"
#include "common_types.h"
#include <vector>

XAUDIO2_VOICE_SENDS AudioGraphMapper::_emptySends = empty_sends();


AudioGraphMapper::AudioGraphMapper(IXAudio2* xaudio)
	: m_xaudio(xaudio)
	, m_masteringNode(nullptr)
{
}

AudioGraphMapper::~AudioGraphMapper()
{
}



IXAudio2SourceVoice* AudioGraphMapper::CreateSourceVoice(const WAVEFORMATEX* pSourceFormat, UINT32 Flags, float MaxFrequencyRatio, IXAudio2VoiceCallback* pCallback, const XAUDIO2_VOICE_SENDS* pSendList, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	auto proxyVoice = new XAudio2SourceVoiceProxy(pSourceFormat->nChannels, pSourceFormat->nSamplesPerSec, Flags, MaxFrequencyRatio, pCallback, -1, from_XAUDIO2_VOICE_SENDS(pSendList), from_XAUDIO2_EFFECT_CHAIN(pEffectChain));

	IXAudio2SourceVoice * actualVoiceRawPtr;
	m_xaudio->CreateSourceVoice(&actualVoiceRawPtr, pSourceFormat, Flags, MaxFrequencyRatio, pCallback, &_emptySends, pEffectChain);
	std::shared_ptr<IXAudio2SourceVoice> actualVoice(actualVoiceRawPtr,	[](IXAudio2SourceVoice * voice) { voice->DestroyVoice(); });

	{
		std::unique_ptr<Node> node = std::make_unique<Node>();
		node->inputSampleRate = pSourceFormat->nSamplesPerSec;
		node->inputChannelsCount = pSourceFormat->nChannels;
		node->actualProcessingStage = -1;
		node->mainOutputChannelsCount = pEffectChain ? pEffectChain->pEffectDescriptors[pEffectChain->EffectCount].OutputChannels : node->inputChannelsCount;
		node->proxyVoice.reset(proxyVoice);
		node->mainVoice = actualVoice;
		m_nodes.insert(std::make_pair(proxyVoice, std::move(node)));
	}

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
		actualVoice->SetFrequencyRatio(source->getFrequencyRatio(), operationSet);
	};

	proxyVoice->onSetSourceSampleRate = [actualVoice](XAudio2SourceVoiceProxy * source)
	{
		actualVoice->SetSourceSampleRate(source->getInputSampleRate());
	};

	setupCommonCallbacks(proxyVoice, actualVoice);

	updateSendsForVoice(proxyVoice);

	return proxyVoice;
}

IXAudio2SubmixVoice* AudioGraphMapper::CreateSubmixVoice(UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const XAUDIO2_VOICE_SENDS* pSendList, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	const UINT32 actualProcessingStage = ProcessingStage * 2; // we make room for intermediate 'tail' voices
	auto proxyVoice = new XAudio2SubmixVoiceProxy(InputChannels, InputSampleRate, Flags, ProcessingStage, from_XAUDIO2_VOICE_SENDS(pSendList), from_XAUDIO2_EFFECT_CHAIN(pEffectChain));

	IXAudio2SubmixVoice * actualVoiceRawPtr;
	m_xaudio->CreateSubmixVoice(&actualVoiceRawPtr, InputChannels, InputSampleRate, Flags, actualProcessingStage, &_emptySends, pEffectChain);
	std::shared_ptr<IXAudio2SubmixVoice> actualVoice(actualVoiceRawPtr,	[](IXAudio2SubmixVoice * voice) { voice->DestroyVoice(); });

	{
		std::unique_ptr<Node> node = std::make_unique<Node>();
		node->inputSampleRate = InputSampleRate;
		node->inputChannelsCount = InputChannels;
		node->actualProcessingStage = actualProcessingStage;
		node->mainOutputChannelsCount = pEffectChain ? pEffectChain->pEffectDescriptors[pEffectChain->EffectCount - 1].OutputChannels : node->inputChannelsCount;
		node->proxyVoice.reset(proxyVoice);
		node->mainVoice = actualVoice;
		m_nodes.insert(std::make_pair(proxyVoice, std::move(node)));
	}

	setupCommonCallbacks(proxyVoice, actualVoice);

	updateSendsForVoice(proxyVoice);

	return proxyVoice;
}

IXAudio2MasteringVoice* AudioGraphMapper::CreateMasteringVoice(UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 DeviceIndex, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	auto proxyVoice = new XAudio2MasteringVoiceProxy(InputChannels, InputSampleRate, Flags, DeviceIndex, from_XAUDIO2_EFFECT_CHAIN(pEffectChain));

	// For HRTF only two-channel mastering voice makes sense.
	IXAudio2MasteringVoice * actualVoiceRawPtr;
	m_xaudio->CreateMasteringVoice(&actualVoiceRawPtr, 2, InputSampleRate, Flags, DeviceIndex, pEffectChain);
	std::shared_ptr<IXAudio2MasteringVoice> actualVoice(actualVoiceRawPtr, [](IXAudio2MasteringVoice * voice) { voice->DestroyVoice(); });

	{
		std::unique_ptr<Node> node = std::make_unique<Node>();
		node->inputSampleRate = InputSampleRate;
		node->inputChannelsCount = 2;
		node->actualProcessingStage = std::numeric_limits<UINT32>::max();
		node->mainOutputChannelsCount = pEffectChain ? pEffectChain->pEffectDescriptors[pEffectChain->EffectCount - 1].OutputChannels : node->inputChannelsCount;
		node->proxyVoice.reset(proxyVoice);
		node->mainVoice = actualVoice;
		m_nodes.insert(std::make_pair(proxyVoice, std::move(node)));
	}

	setupCommonCallbacks(proxyVoice, actualVoice);

	m_masteringNode = m_nodes[proxyVoice].get();
	return proxyVoice;
}

VoiceSends AudioGraphMapper::mapProxySendsToActualOnes(const VoiceSends & proxySends) const
{
	voice_sends actualSends(proxySends.getVoices().size());
	std::transform(std::begin(proxySends.getVoices()), std::end(proxySends.getVoices()), std::begin(actualSends),
		[&](const XAUDIO2_SEND_DESCRIPTOR & sendDesc)
	{
		XAUDIO2_SEND_DESCRIPTOR actualSendDesc;
		actualSendDesc.Flags = sendDesc.Flags;
		actualSendDesc.pOutputVoice = getNodeForProxyVoice(sendDesc.pOutputVoice)->mainVoice.get();
		return actualSendDesc;
	});

	return VoiceSends(actualSends);
}



void AudioGraphMapper::setupCommonCallbacks(XAudio2VoiceProxy* proxyVoice, const std::shared_ptr<IXAudio2Voice> & actualVoice)
{
	proxyVoice->onSetOutputVoices = [this](XAudio2VoiceProxy * source, matrices_map & outputMatrices, filter_params_map & outputFilterParams)
	{
		updateSendsForVoice(source);

		// TODO : replace stub impementation with a normal one
		outputMatrices.clear();
		for (auto & sendDesc : source->getSends().getVoices())
		{
			float vals[1]{ 666 };
			outputMatrices.insert(std::make_pair(sendDesc.pOutputVoice, ChannelMatrix(vals, 1, 1)));
		}

		outputFilterParams.clear();
		for (auto & sendDesc : source->getSends().getVoices())
		{
			XAUDIO2_FILTER_PARAMETERS params;
			outputFilterParams.insert(std::make_pair(sendDesc.pOutputVoice, params));
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

	proxyVoice->onSetOutputFilterParameters = [this, actualVoice](XAudio2VoiceProxy * source, IXAudio2Voice * pDestinationProxy, UINT32 operationSet)
	{
		Node* node = getNodeForProxyVoice(source->asXAudio2Voice());
		auto params = source->getOutputFilterParameters(pDestinationProxy);
		actualVoice->SetOutputFilterParameters(node->tailVoices.at(pDestinationProxy).get(), &params, operationSet);
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

	proxyVoice->onSetOutputMatrix = [this](XAudio2VoiceProxy * source, IXAudio2Voice * pDestinationProxy, UINT32 operationSet)
	{
		Node* node = getNodeForProxyVoice(source->asXAudio2Voice());
		Node* destinatioNode = pDestinationProxy ? getNodeForProxyVoice(pDestinationProxy) : m_masteringNode;

		auto clientMatrix = source->getOutputMatrix(pDestinationProxy);

		ChannelMatrix matrix;

		if (destinatioNode->inputChannelsCount == clientMatrix.getDestinationCount())
		{
			matrix = std::move(clientMatrix);
		}
		else if (destinatioNode->inputChannelsCount == 2)
		{
			matrix = adaptChannelMatrixToStereoOutput(clientMatrix);
		}
		else
		{
			throw std::logic_error("Sender output channels count does not match sendee input channels count and sendee input channels count is not 2. That should not have happened.");
		}

		std::vector<float> values;
		values.resize(matrix.getSourceCount() * matrix.getDestinationCount());
		from_ChannelMatrix(matrix, matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());
		if (pDestinationProxy)
		{
			node->tailVoices.at(pDestinationProxy)->SetOutputMatrix(getNodeForProxyVoice(pDestinationProxy)->mainVoice.get(), matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());
		}
		else if (pDestinationProxy == nullptr && source->getSends().getSendToMasterOnly())
		{
			node->tailVoices.at(m_masteringNode->proxyVoice.get())->SetOutputMatrix(m_masteringNode->mainVoice.get(), matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());
		}
		else if (pDestinationProxy == nullptr && source->getSends().getVoices().size() == 1)
		{
			IXAudio2Voice * proxySend = source->getSends().getVoices()[0].pOutputVoice;
			auto sendNode = getNodeForProxyVoice(proxySend);
			node->tailVoices.at(proxySend)->SetOutputMatrix(sendNode->mainVoice.get(), matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());
		}
		else
		{
			throw std::logic_error("pDestination == nullptr but there's multiple output voices");
		}
	};

	proxyVoice->onDestroyVoice = [this, actualVoice](XAudio2VoiceProxy * source)
	{
		m_nodes.erase(m_nodes.find(source->asXAudio2Voice()));
	};
}

const Node * AudioGraphMapper::getNodeForProxyVoice(IXAudio2Voice* pDestination) const
{
	return m_nodes.at(pDestination).get();
}

Node * AudioGraphMapper::getNodeForProxyVoice(IXAudio2Voice* pDestination)
{
	return m_nodes.at(pDestination).get();
}

void AudioGraphMapper::updateSendsForVoice(XAudio2VoiceProxy* proxyVoice)
{
	Node* node = getNodeForProxyVoice(proxyVoice->asXAudio2Voice());

	m_edges.removeAllSuccessors(node);
	
	node->mainVoice->SetOutputVoices(&_emptySends);
	node->tailVoices.clear();

	std::vector<XAUDIO2_SEND_DESCRIPTOR> proxySends;

	if (!proxyVoice->getSends().getSendToMasterOnly())
	{
		proxySends = proxyVoice->getSends().getVoices();
	}
	else
	{
		XAUDIO2_SEND_DESCRIPTOR masterSendDescriptor;
		masterSendDescriptor.Flags = 0;
		masterSendDescriptor.pOutputVoice = m_masteringNode->proxyVoice.get();
		proxySends.push_back(masterSendDescriptor);
	}

	std::vector<XAUDIO2_SEND_DESCRIPTOR> sendsToTailsVector;
	for (auto & proxySend : proxySends)
	{
		Node* sendNode = getNodeForProxyVoice(proxySend.pOutputVoice);
		IXAudio2SubmixVoice * tailVoice;
		XAUDIO2_SEND_DESCRIPTOR sendDescriptor;
		sendDescriptor.pOutputVoice = sendNode->mainVoice.get();
		sendDescriptor.Flags = 0;
		XAUDIO2_VOICE_SENDS sends;
		sends.SendCount = 1;
		sends.pSends = &sendDescriptor;
		m_xaudio->CreateSubmixVoice(&tailVoice, node->mainOutputChannelsCount, node->inputSampleRate, 0, node->actualProcessingStage + 1, &sends, nullptr);

		node->tailVoices.insert(std::make_pair(proxySend.pOutputVoice, std::shared_ptr<IXAudio2SubmixVoice>(tailVoice, [](IXAudio2Voice * pVoice) { pVoice->SetOutputVoices(&_emptySends); pVoice->DestroyVoice(); })));

		XAUDIO2_SEND_DESCRIPTOR tailSendDescriptor;
		tailSendDescriptor.Flags = proxySend.Flags;
		tailSendDescriptor.pOutputVoice = tailVoice;

		sendsToTailsVector.push_back(tailSendDescriptor);
		m_edges.addEdge(node, sendNode);
	}

	auto sendsToTails = VoiceSends(sendsToTailsVector);
	XAUDIO2_VOICE_SENDS actualSendsStruct = from_voice_sends(sendsToTails);
	node->mainVoice->SetOutputVoices(&actualSendsStruct);
}