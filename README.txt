README.txt for codec2
David Rowe
21 August 2009

Introduction
------------

codec2 is a open source low bit rate speech codec designed for
communications quality speech at around 2400 kbit/s.  Applications
include low bandwidth HF/VHF digital radio.

Status
------

Unquantised encoder and decoder running under Linux/gcc, pitch
estimator untested.

Quick Start
-----------

To encode the file raw/hts1a.raw to a set of sinusoidal model
parameters (src/hts1.mdl) then decode to a raw file src/hts1a_uq:

$ cd src
$ make
$ ./sinenc ../raw/hts1a.raw hts1.mdl 300 ../pitch/hts1a.p
$ ./sinedec ../raw/hts1a.raw hts1.mdl -o hts1a_uq.raw
$ play -f s -r 8000 -s w ../raw/hts1a.raw
$ play -f s -r 8000 -s w hts1a_uq.raw

Plan
----

[X] Milestone 0 - Project kick off
[X] Milestone 1 - Baseline unquantised codec running under Linux/gcc
[ ] Milestone 2 - Spectral amplitudes quantised 
[ ] Milestone 3 - Prototype 2400 bit/s codec

Directories
-----------

src      - C source code
octave   - Matlab/Octave scripts
pitch    - pitch estimator output files
raw      - speech files in raw format (16 bits signed linear)
unittest - Unit test source code
wav      - speech files in wave file format

References
----------

[1] Introductory Blog Post:
    http://www.rowetel.com/blog/?p=128

[2] Bruce Parens introducing the project:
    http://codec2.org/

[3] Davids Thesis, used for baseline algorithm:
    http://www.itr.unisa.edu.au/~steven/thesis/dgr.pdf

