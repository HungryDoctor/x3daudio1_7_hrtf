# x3daudio1_7_hrtf
## HRTF for Arma 3, Skyrim, and potentially other titles using X3dAudio + XAudio2.

### Installation
Put x3daudio1_7.dll to a game folder where main executable is located. Ensure it is the exe of the game itself, not a launcher. Then take *.mhr files from OpenAL Soft (google for it) and put it to _Game_folder_folder\hrtf\_ (it does not exist, you should create such subfolder). Start. Enjoy!

#### Notes for developers
##### How it works
Game hooks both x3daudio1_x.dll, wich is simple, and IXAduio2 COM interface, which is less trivial.
x3daudio proxy remembers 3d audio position into repository. To mark sound as 3d, then sets first coefficient of the mixing matrix to NaN. And the second one is set to sound ID, so we can later identificate it. We hope game is not that stupid and won't pass us 1x1 matrix.
Then, when game calls IXAuido2Voice::SetOutputMatrix(), we check if 1st coeff == NaN which means we have to enable our XAPO DSP effect and pass parameters to it (position, etc).

##### Arma 3
Works well now. No tricky things there. Has pretty complex audio mixing greaph, but nothing wierd.

##### TES V Skyrim
Works well now. Has simple but strange graph. Source voices has two outputs: one goes to the Mastering voice, and another to a Submix, which, in turn, sends to the Mastering one. The one that is sent to Submix, does not pass NaN. So we hae nothing to do but not to send any sound there by setting output matrix to zero.

Skyrim always creates Mastering voice as six-channel. We override it to two.
