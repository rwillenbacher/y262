# y262

y262 is a mpeg-1/2 video encoder

# features
* MPEG-1/2 video output as a raw bitstream
* 420, 422 and 444 chroma format
* All modes for frame pictures, including interlaced
* Multipass bitrate control, do as many passes as you want
* Adaptive quantization, yeah that variance based stuff
* Psyrd, but it does not work good for mpeg2
* Good picture quality, as far as i can tell
* Slice based threading

# Sadly missing
* Field pictures
* Dual prime
* Frame Threading

# How to build
You need cmake.
You need a C compiler supported by cmake.
You need the yasm assembler. cmake will look for it.

In the root directory of the y262 directory it should be as simple as:
```bash
mkdir build
cd build
cmake -G "Visual Studio 16 2019" -A x64 ..
```
You probably need to replace the generator and architecture with what you have.
Then build !


# Running y262
-- Running y262 --
Once you have your executable you can run the encoder without arguments to get a list of possible parameters.

Sample output:
```
        y262app -in <420yuv> -size <width> <height> -out <m2vout>

        -frames <number>    : number of frames to encode, 0 for all
        -threads <on> <cnt> :  threading enabled and number of concurrent slices
        -profile <profile>  :  simple or main profile
        -level <level>      :  low main high1440 or high level
        -chromaf            :  chroma format, 420, 422 or 444
        -rec <reconfile>    :  write reconstructed frames to <reconfile>
        -rcmode <pass>      :  0 = CQ, 1 = 1st pass, 2 = subsequent pass
        -mpin <statsfile>   :  stats file of previous pass
        -mpout <statsfile>  :  output stats file of current pass
        -bitrate <kbps>     :  average bitrate
        -vbvrate <kbps>     :  maximum bitrate
        -vbv <kbps>         :  video buffer size
        -quant <quantizer>  :  quantizer for CQ
        -interlaced         :  enable field macroblock modes
        -bff                :  first input frame is bottom field first
        -pulldown_frcode <num>:frame rate code to pull input up to
        -quality <number>   :  encoder complexity, negative faster, positive slower
        -frcode <number>    :  frame rate code, see mpeg2 spec
        -arinfo <number>    :  aspect ratio information, see mpeg2 spec
        -qscale0            :  use more linear qscale type
        -nump <number>      :  number of p frames between i frames
        -numb <number>      :  number of b frames between i/p frames
        -closedgop          :  bframes after i frames use only backwards prediction
        -noaq               :  disable variance based quantizer modulation
        -psyrd <number>     :  psy rd strength
        -avamat6            :  use avamat6 quantization matrices
        -flatmat            :  use flat quantization matrices <for high rate>
        -intramat <textfile>:  use the 64 numbers in the file as intra matrix
        -intermat <textfile>:  use the 64 numbers in the file as inter matrix
        -videoformat <fmt>  :  pal, secam, ntsc, 709 or unknown
        -mpeg1              :  output mpeg1 instead mpeg2, constraints apply
```

Notes about -in, you can specify "-" if you want to pipe the yuv into the application. The yuv format of the input should match -chromaf


Example: So to encode something you call it like so:

./y262app -in my_NTSC_video.yuv -size 720 480 -profile main -level high -rcmode 1 -mpout test.stats -bitrate
3000 -vbvrate 6000 -vbv 6000 -quality 0 -frcode 2 -pulldown_frcode 4 -arinfo 2 -nump 4 -numb 2 -out test_p1.m2v
-videoformat ntsc -interlaced

This will encode the 420 raw yuv file my_NTSC_video.yuv with the dimensions 720x480, signaling main profile at
high level. It is the first pass and multipass data is written to the file test.stats for subsequent passes.
It is encoded with an average bitrate of 3 Mbit and a maximum video rate of 6Mbit, using 6Mbit video buffer.
The encode is done at the default quality/speed tradeoff of 0 ( The -quality parameter roughly takes values from
around -50 to 50 ).
The framerate of the input is 24fps and the sequence is encoded at 29.97 fps, which will result in aprox. 3:2
pulldown in the resulting file. Aspect ratio is set to 4:3. The encoder will place 4 P frames between keyframes
and between the frames of the so resulting IPPPPI sequence 2 B Frames each ( IBBPBBPBBPBBPBBIBB.. ). The
elementary stream is written to test_p1.m2v. The video format and colorspace parameters are written out to
specify ntsc. Field macroblock modes are enabled, slowing things down a lot for a slight coding gain.

Possible -frcode and -pulldown_frcode ( frame rate ) values
```
1: ( 24000 / 1001 ) fps
2: 24 fps
3: 25 fps
4: ( 30000 / 1001 ) fps
5: 30 fps
6: 50 fps
7: ( 60000 / 1001 ) fps
8: 60 fps
```

Possible -arinfo ( aspect ratio ) values:
```
1: 1:1
2: 4:3
3: 16:9
4: 2.21:1
```

Possible -arinfo ( pixel aspect ratio ) values for -mpeg1:
```
1: 1.0
2: 0.6735
3: 0.7031
4: 0.7615
5: 0.8055
6: 0.8437
7: 0.8935
8: 0.9375
9: 0.9815
10: 1.0255
11: 1.0695
12: 1.1250
13: 1.1575
14: 1.2015
```

# Notes

This is an offline encoder. It may do some retries on bitrate control failure to prevent bad quality or video buffer violations. So on complex content it may slow down quite a lot. This is also the main reason for missing frame threading.

Patent situation regarding Mpeg2 you best check with the [MPEG LA](https://www.mpegla.com) or ask your legal department.