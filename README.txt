Codec 2 README
--------------

Codec 2 is an open source 2400 bit/s speech codec.  For more
information please see:

    http://rowetel.com.codec2.html

Quickstart
----------

$ cd codec2/src
$ make
$ ./c2enc ../raw/hts1a.raw hts1a_c2.bit
$ ./c2dec hts1a_c2.bit hts1a_c2.raw 
$ ../scripts/menu.sh ../raw/hts1a.raw hts1a_c2.raw

Programs
--------

1/ c2enc encodes a file of speech sample to a file of encoded bits.
One bit is stored in the LSB or each byte.

2/ c2dec decodes a file of bits to a file of speech samples.

3/ c2sim is a simulation/development version of codec 2.  It allows
selective use of the various codec 2 algorithms.  For example
switching phase modelling or LSP quantisation on and off.

Directories
-----------

  script   - shell scripts for playing and converting raw files
  src      - C source code
  octave   - Octave scripts used for visualising internal signals 
             during development
  pitch    - pitch estimator output files
  raw      - speech files in raw format (16 bits signed linear 8 KHz)
  unittest - unit test source code
  wav      - speech files in wave file format

