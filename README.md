# X3DAudio HRTF
## HRTF for Arma 3, Skyrim, and potentially other titles using X3dAudio + XAudio2.

### Tags
HRTF, HRIR, Binaural sound, Spatial sound, Ambisonics

### Installation
Put x3daudio1_7.dll to a game folder where main executable is located. Ensure it is the exe of the game itself, not a launcher. Then take *.mhr files from OpenAL Soft (google for it) and put it to _Game_folder_folder\hrtf\_ (it does not exist, you should create such subfolder). Start. Enjoy!

#### Notes for developers
##### How it works
Game hooks both x3daudio1_x.dll, which is simple, and IXAudio2 COM interface, which is a less trivial thing to do.
X3daudio proxy remembers 3D audio position into repository. To mark sound as 3D it sets the first coefficient of the mixing matrix to NaN. And the second one is set to sound ID, so we can later identify it. Let's hope game is not that stupid and won't pass a 1x1 matrix.
Then, when game calls IXAuido2Voice::SetOutputMatrix(), we check if 1st coeff == NaN which means we have to enable our XAPO DSP effect and pass parameters to it (position, etc).

##### Arma 3
Works well now. No tricky things there. Has pretty complex audio mixing greaph, but nothing wierd.

##### Arma 2
Has glitches. I haven't investigate them yet. Most sounds are fine, but HMMWV's engine sound was coming from a completely different direction when I was playing.

##### TES V Skyrim
Works well now. Has simple but strange graph. Source voices has two outputs: one goes directly to the Mastering voice, and another to a Submix, which, in turn, sends to the Mastering one. The one that is sent to Submix, does not pass NaN. So we have nothing to do but not to send any sound there at all by setting output matrix to zero. However that works well.

Skyrim always creates Mastering voice as six-channel. We override it to two.
