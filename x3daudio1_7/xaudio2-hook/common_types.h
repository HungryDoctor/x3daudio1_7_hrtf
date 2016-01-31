#pragma once

#include <XAudio2.h>
#include <map>
#include <vector>

#include "ChannelMatrix.h"
#include "VoiceSends.h"

typedef std::map<IXAudio2Voice *, ChannelMatrix> matrices_map;
typedef std::map<IXAudio2Voice*, XAUDIO2_FILTER_PARAMETERS> filter_params_map;

typedef std::vector<XAUDIO2_SEND_DESCRIPTOR> voice_sends;
typedef std::vector<XAUDIO2_EFFECT_DESCRIPTOR> effect_chain;