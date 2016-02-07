#include "stdafx.h"

#include "AudioGraphMapper.h"

#include "proxy/XAudio2VoiceProxy.h"
#include "proxy/XAudio2MasteringVoiceProxy.h"
#include "proxy/XAudio2SourceVoiceProxy.h"
#include "proxy/XAudio2SubmixVoiceProxy.h"

#include "interop/ChannelMatrixMagic.h"
#include "wave/WaveFile.h"

#include "util.h"
#include "common_types.h"
#include "logger.h"
#include <vector>
#include <xaudio2-hook/XAPO/HrtfEffect.h>

XAUDIO2_VOICE_SENDS AudioGraphMapper::_emptySends = empty_sends();


AudioGraphMapper::AudioGraphMapper(IXAudio2* xaudio, ISound3DRegistry * spatialSoundRegistry)
	: m_xaudio(xaudio)
	, m_spatialSoundRegistry(spatialSoundRegistry)
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
	std::shared_ptr<IXAudio2SourceVoice> actualVoice(actualVoiceRawPtr, [](IXAudio2SourceVoice * voice) { voice->DestroyVoice(); });

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

	WAVEFORMATEX sourceFormat = *pSourceFormat;
	proxyVoice->onSubmitSourceBuffer = [actualVoice, sourceFormat](XAudio2SourceVoiceProxy * source, const XAUDIO2_BUFFER * pBuffer, const XAUDIO2_BUFFER_WMA * pBufferWma)
	{
		{
			//WaveFile waveFile(std::wstring(L"wavs\\") + std::to_wstring(sourceFormat.nChannels) + L"_" + get_name(static_cast<XAudio2VoiceProxy*>(source)) + L".wav", sourceFormat.nChannels, sourceFormat.nSamplesPerSec, sourceFormat.wBitsPerSample);
			//waveFile.AppendData(pBuffer->pAudioData, pBuffer->AudioBytes);
		}

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

	resetSendsForVoice(proxyVoice);

	return proxyVoice;
}

IXAudio2SubmixVoice* AudioGraphMapper::CreateSubmixVoice(UINT32 InputChannels, UINT32 InputSampleRate, UINT32 Flags, UINT32 ProcessingStage, const XAUDIO2_VOICE_SENDS* pSendList, const XAUDIO2_EFFECT_CHAIN* pEffectChain)
{
	const UINT32 actualProcessingStage = ProcessingStage * 2; // we make room for intermediate 'tail' voices
	auto proxyVoice = new XAudio2SubmixVoiceProxy(InputChannels, InputSampleRate, Flags, ProcessingStage, from_XAUDIO2_VOICE_SENDS(pSendList), from_XAUDIO2_EFFECT_CHAIN(pEffectChain));

	IXAudio2SubmixVoice * actualVoiceRawPtr;
	m_xaudio->CreateSubmixVoice(&actualVoiceRawPtr, InputChannels, InputSampleRate, Flags, actualProcessingStage, &_emptySends, pEffectChain);
	std::shared_ptr<IXAudio2SubmixVoice> actualVoice(actualVoiceRawPtr, [](IXAudio2SubmixVoice * voice) { voice->DestroyVoice(); });

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

	resetSendsForVoice(proxyVoice);

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
		resetSendsForVoice(source);

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
		Node* destinationNode = getNodeForProxyVoice(pDestinationProxy);
		if (!node->tailVoices.at(destinationNode).isSpatialized) // There's no need to add filters, i guess.
		{
			auto params = source->getOutputFilterParameters(pDestinationProxy);
			actualVoice->SetOutputFilterParameters(node->tailVoices.at(destinationNode).voice.get(), &params, operationSet);
		}
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

		if (pDestinationProxy == nullptr && node->tailVoices.size() != 1)
			throw std::logic_error("Cannot set output matrix to voice 'nullptr' when the number of output voices is not 1.");

		Node* destinationNode = pDestinationProxy ? getNodeForProxyVoice(pDestinationProxy) : *m_edges.getSuccessors(node).begin();

		auto clientMatrix = source->getOutputMatrix(pDestinationProxy);

		TailVoiceDescriptor & tailVoiceDescriptor = node->tailVoices.at(destinationNode);

		if (does_matrix_contain_id(clientMatrix))
		{
			if (node->mainOutputChannelsCount > 2)
				logger::logRelease(L"WARNING: onSetOutputMatrix ", source, " Trying to spatialize voice with more than two channels");

			if (destinationNode->inputChannelsCount != 2)
				logger::logRelease(L"WARNING: onSetOutputMatrix ", source, " Trying to send spatialized sound to non-stereo voice ", destinationNode->proxyVoice.get());

			if (!tailVoiceDescriptor.isSpatialized)
			{
				node->mainVoice->SetOutputVoices(&_emptySends); // to allow to destroy one of it's 'tails'

				XAUDIO2_EFFECT_DESCRIPTOR effectDesc;
				effectDesc.pEffect = static_cast<IUnknown *>(static_cast<IXAPOParameters *>(HrtfXapoEffect::CreateInstance()));
				effectDesc.InitialState = false;
				effectDesc.OutputChannels = 2;

				effect_chain hrtfEffectChain{ effectDesc };
				tailVoiceDescriptor.voice = createTailVoice(node, destinationNode, hrtfEffectChain);

				//TODO : set output matrix maybe?
				// we leave it default. However, sending to non-stereo voice makes no sense.

				updateSendsForMainVoice(node);

				tailVoiceDescriptor.isSpatialized = true;
			}

			sound_id id = extract_sound_id(clientMatrix);

			auto sound3d = m_spatialSoundRegistry->GetEntry(id);

			HrtfXapoParam params;
			params.volume_multiplier = sound3d.volume_multiplier;
			params.elevation = sound3d.elevation;
			params.azimuth = sound3d.azimuth;
			params.distance = sound3d.distance;

			tailVoiceDescriptor.voice->SetEffectParameters(0, &params, sizeof(params), XAUDIO2_COMMIT_NOW);
			tailVoiceDescriptor.voice->EnableEffect(0, XAUDIO2_COMMIT_NOW);
		}
		else
		{
			if (tailVoiceDescriptor.isSpatialized)
			{
				node->mainVoice->SetOutputVoices(&_emptySends); // to allow to destroy one of it's 'tails'

				tailVoiceDescriptor.voice = createTailVoice(node, destinationNode, effect_chain());

				updateSendsForMainVoice(node);

				tailVoiceDescriptor.isSpatialized = false;
			}

			ChannelMatrix matrix;

			if (destinationNode->inputChannelsCount == clientMatrix.getDestinationCount())
			{
				matrix = std::move(clientMatrix);
				//matrix = ChannelMatrix(clientMatrix.getSourceCount(), clientMatrix.getDestinationCount());
			}
			else if (destinationNode->inputChannelsCount == 2)
			{
				matrix = adaptChannelMatrixToStereoOutput(clientMatrix);
				//matrix = ChannelMatrix(clientMatrix.getSourceCount(), 2);
			}
			else
			{
				throw std::logic_error("Sender output channels count does not match sendee input channels count and sendee input channels count is not 2. That should not have happened.");
			}

			std::vector<float> values;
			values.resize(matrix.getSourceCount() * matrix.getDestinationCount());
			from_ChannelMatrix(matrix, matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());

			node->tailVoices.at(destinationNode).voice->SetOutputMatrix(destinationNode->mainVoice.get(), matrix.getSourceCount(), matrix.getDestinationCount(), &*values.begin());
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



void AudioGraphMapper::resetSendsForVoice(XAudio2VoiceProxy* proxyVoice)
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

	for (auto & proxySend : proxySends)
	{
		Node* sendNode = getNodeForProxyVoice(proxySend.pOutputVoice);
		auto tailVoice = createTailVoice(node, sendNode, effect_chain());

		TailVoiceDescriptor tailVoiceDesc;
		tailVoiceDesc.voice = std::move(tailVoice);
		tailVoiceDesc.flags = proxySend.Flags;
		tailVoiceDesc.isSpatialized = false;
		node->tailVoices.insert(std::make_pair(sendNode, std::move(tailVoiceDesc)));

		m_edges.addEdge(node, sendNode);
	}

	updateSendsForMainVoice(node);
}

void AudioGraphMapper::updateSendsForMainVoice(Node* node)
{
	std::vector<XAUDIO2_SEND_DESCRIPTOR> sendsToTailsVector;

	for (auto & tail : node->tailVoices)
	{
		XAUDIO2_SEND_DESCRIPTOR tailSendDescriptor;
		tailSendDescriptor.Flags = tail.second.flags;
		tailSendDescriptor.pOutputVoice = tail.second.voice.get();

		sendsToTailsVector.push_back(tailSendDescriptor);
	}

	auto sendsToTails = VoiceSends(sendsToTailsVector);
	XAUDIO2_VOICE_SENDS actualSendsStruct = from_voice_sends(sendsToTails);
	node->mainVoice->SetOutputVoices(&actualSendsStruct);
}

std::shared_ptr<IXAudio2SubmixVoice> AudioGraphMapper::createTailVoice(Node * senderNode, Node* sendNode, const effect_chain & effectChain)
{
	XAUDIO2_SEND_DESCRIPTOR sendDescriptor;
	sendDescriptor.pOutputVoice = sendNode->mainVoice.get();
	sendDescriptor.Flags = 0;
	XAUDIO2_VOICE_SENDS sends;
	sends.SendCount = 1;
	sends.pSends = &sendDescriptor;

	IXAudio2SubmixVoice * tailVoice;
	if (effectChain.size() == 0)
	{
		m_xaudio->CreateSubmixVoice(&tailVoice, senderNode->mainOutputChannelsCount, senderNode->inputSampleRate, 0, senderNode->actualProcessingStage + 1, &sends, nullptr);
	}
	else
	{
		XAUDIO2_EFFECT_CHAIN chainStruct;
		chainStruct.EffectCount = static_cast<UINT32>(effectChain.size());
		chainStruct.pEffectDescriptors = const_cast<XAUDIO2_EFFECT_DESCRIPTOR*>(&*effectChain.begin());
		m_xaudio->CreateSubmixVoice(&tailVoice, senderNode->mainOutputChannelsCount, senderNode->inputSampleRate, 0, senderNode->actualProcessingStage + 1, &sends, &chainStruct);
	}

	return std::shared_ptr<IXAudio2SubmixVoice>(tailVoice, [](IXAudio2Voice * pVoice) { pVoice->DestroyVoice(); });
}
