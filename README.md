# iq_toolbox
This project is a toolbox to process IQ signals as simple as possible by providing a set of program that works on raw data stream either scalar or I/Q signals. This toolbox is compliant with programs that generate I/Q signals like the well-known *RTL-SDL* (http://sdr.osmocom.org/trac/wiki/rtl-sdr) or programs that can both generate / receive I/Q signals like *limesdr_toolbox* (https://github.com/emvivre/limesdr_toolbox).

Originally, the aim of this project is for educational purpose but turns out to run very well to perform I/Q signal processing in realtime. 

The toolbox provides the following programs:
 - iq_conv : convert the data type of an input signal
 - iq_normalize : normalize samples of an input signal to arbitrary range
 - iq_phasis : extract instantaneous phasis of a I/Q signal
 - iq_modfreq : frequency modulation from a scalar signal
 - iq_demodfreq : extract instantaneous frequency of a I/Q signal 
 - iq_preemphasis : pre-emphasis of a input signal
 - iq_deemphasis : de-emphasis of a input signal
 - iq_decimate : perform a low-passfilter then a downsampling of a input signal
 - iq_mix : mixing of a I/Q signal
 - iq_psd.py : display of Power Spectral Density (PSD)
 - iq_spectrogram.py : display of the spectrogram


Installation
============
To compile this project type:
```
git clone --recursive https://github.com/emvivre/iq_toolbox
cd iq_toolbox
make
```

To easily access the compiled programs, you can type the following command:
```
PATH=$(pwd)/bin/:$PATH
```

Description
===========
Each program is a step in the algorithm to perform, and communicate with the previous program to get inputs data and generate output to the next program. By default the Inter Process Communication (IPC) is based on pipe (stdin / stdout). Of course, a input or output file can be specified with **-i <INPUT_CAPTURE_FILE>** / **-o <OUTPUT_CAPTURE_FILE>** argument. 

Most of the programs of this toolbox work on both scalar or IQ signal. Input type can be set with **-t scalar** for a scalar input signal or **-t iq** for an IQ signal. Input and output samples data type can be specify respectively by **-d <INPUT_DATA_FORMAT>** / **-D <OUTPUT_DATA_FORMAT>**.

To display an help on the usage of a program, run the program without any argument, like:
```
iq_conv 
Usage: iq_conv <OPTIONS>
  -s <SAMPLE_RATE>
  -d <INPUT_DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i8)
  -D <OUTPUT_DATA_FORMAT> : i8 | i16 | i32 | f32 | f64 (default: i16)
  -i <INPUT_CAPTURE_FILE> (default: -)
  -o <OUTPUT_CAPTURE_FILE> (default: -)
```

Examples
========
Some examples of application are provided in the following.

- Capture / FM demodulatation / play 
```
F_STATION=94.0e6
S=250000
SSS=50000
FF=44100
rtl_sdr -f $F_STATION -s $S - | iq_phasis -d i8 -D f32 | iq_decimate -t scalar -s $S -f $FF -d f32 | iq_deemphasis -s $SSS | iq_normalize -t scalar -d f32 -m 10000 | iq_conv -t scalar -d f32 -D i16 | play -r $SSS -e signed -b 16 -t raw -
```

- From a local (or remote) file / FM modulation / emitting 
```
# INPUT_AUDIO_FILE=https://cdn.media.ccc.de/congress/2008/audio_only/25c3-2799-en-console_hacking_2008_wii_fail.ogg
INPUT_AUDIO_FILE=~/music/amiga/desert_dream.music.mp4
S=250000
F_STATION_OUT=108e6
TX_GAIN=1
ffmpeg -i "$INPUT_AUDIO_FILE" -c pcm_s16le -f wav - | sox -t wav - -c 1 -r $S -e signed -b 16 -t raw - | iq_conv -d i16 -D f32 | iq_preemphasis -s $S -t scalar -d f32 | iq_normalize -t scalar -d f32 -m 1 | iq_modfreq -d f32 -D i16 | limesdr_send -g $TX_GAIN -f $F_STATION_OUT -s $S
```

- Stream the audio output of a computer with RTP server / connection to the RTP music server / FM modulation / emitting
```
S=250000
F_STATION_OUT=108e6
TX_GAIN=1
pactl list | grep monitor
#  Monitor Source: alsa_output.pci-0000_00_0e.0.analog-stereo.monitor
#  Name: alsa_output.pci-0000_00_0e.0.analog-stereo.monitor
#                device.class = "monitor"
vlc pulse://alsa_output.pci-0000_00_0e.0.analog-stereo.monitor --sout '#transcode{vcodec=none,acodec=mp3,ab=320,channels=2,samplerate=44100}:standard{access=http,mux=mp3,dst=0.0.0.0:8080}}' --sout-keep &
sleep 1
ffmpeg -i http://localhost:8080 -c pcm_s16le -f wav - | sox -t wav - -c 1 -r $S -e signed -b 16 -t raw - | iq_conv -d i16 -D f32 | iq_preemphasis -s $S -t scalar -d f32 | iq_normalize -t scalar -d f32 -m 1 | iq_modfreq -d f32 -D i16 | limesdr_send -g $TX_GAIN -f $F_STATION_OUT -s $S
```

- Trick for TX DC offset 
In case your are unable to fix the TX DC offset, you can apply a frequency deviation on the I/Q signals just before the emitting step :
```
<IQ_GENERATOR_CMD> | iq_mix ... | <SENDING_CMD> 
```
The following is an exemple of 
```
S=450e3
F_STATION_OUT=108e6
TX_GAIN=1
F_MIX_OUT=110e3
F_STATION_OUT=$(python -c "print $F_STATION_OUT+$F_MIX_OUT")
ffmpeg -i "$INPUT_AUDIO_FILE" -c pcm_s16le -f wav - | sox -t wav - -c 1 -r $S -e signed -b 16 -t raw - | iq_conv -d i16 -D f32 | iq_preemphasis -s $S -t scalar -d f32 | iq_normalize -t scalar -d f32 -m 1 | iq_modfreq -d f32 -D i16 | iq_mix -d i16 -s $S -m $F_MIX_OUT | limesdr_send -g $TX_GAIN -f $F_STATION_OUT -s $S
```

Acknowledgment
==============
- https://witestlab.poly.edu/blog/capture-and-decode-fm-radio/
- http://jontio.zapto.org/hda1/preempiir.pdf


