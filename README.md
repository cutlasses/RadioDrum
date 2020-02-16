# RadioDrum

RadioDrum is an alternate firmare for the RadioMusic, from MusicThing Modular. I wanted to get some drums into my set using as little HP as possible, so came up with this. The drum patterns are stored as text files on the SD card.

To get new samples into the firmware you need to run wav2sketch on the samples and include them in the project when you compile.

https://github.com/PaulStoffregen/Audio/blob/master/extras/wav2sketch/wav2sketch.c

use '-16' on the command line when you run it. The code is setup for uncompressed 16-bit uncompressed audio, but wav2sketch uses u-law encoding by default.

https://youtu.be/lzOFfdgeuCY

<a href="http://www.youtube.com/watch?feature=player_embedded&v=lzOFfdgeuCY
" target="_blank"><img src="http://img.youtube.com/vi/lzOFfdgeuCY/0.jpg" 
alt="RadioDrum Video" width="240" height="180" border="10" /></a>
