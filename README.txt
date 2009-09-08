Codec2 - Open Source Low Bit Rate Speech Codec
===============================================

David Rowe, VK5DGR

[[introduction]]
Introduction
------------

Codec2 is a open source low bit rate speech codec designed for
communications quality speech at around 2400 kbit/s.  Applications
include low bandwidth HF/VHF digital radio.  It fills a gap in open
source, free as in speech voice codecs beneath 5000 bit/s.

The motivations behind the project are summarised on this
link:/blog/?p=128[blog post].

[[status]]
Status
------

1. Unquantised encoder (sinenc) and decoder (sinedec) running under
   Linux/gcc, pitch estimator untested.  The decoder (sinedec) is a
   test-bed for various modelling and quantisation options - these are
   controlled via command line switches.

2. LPC modelling working and first pass LSP vector quantiser working
   at 37 bits/frame with acceptable voice quality.  Lots could be done
   to improve this.

3. Phase model developed that uses 0 bits for phase and 1 bit/frame
   for voiced/unvoiced decision.

[[source]]
The Source Code
---------------

Browse:

http://freetel.svn.sourceforge.net/viewvc/freetel/codec2/[http://freetel.svn.sourceforge.net/viewvc/freetel/codec2/]

Check Out:

  $ svn co https://freetel.svn.sourceforge.net/svnroot/freetel/codec2 codec2

[[support]]
The Mailing List
----------------

For any questions, comments, support, suggestions for applications by
end users and developers please post to the https://lists.sourceforge.net/lists/listinfo/freetel-codec2[Codec2 Mailing List]

[[quickstart]]
Quick Start
-----------

To encode the file raw/hts1a.raw to a set of sinusoidal model
parameters (hts1.mdl) then decode to a raw file src/hts1a_uq:

 $ cd codec2/src
 $ make
 $ ./sinenc ../raw/hts1a.raw hts1.mdl 300 ../pitch/hts1a.p
 $ ./sinedec ../raw/hts1a.raw hts1.mdl -o hts1a_uq.raw
 $ play -f s -r 8000 -s w ../raw/hts1a.raw
 $ play -f s -r 8000 -s w hts1a_uq.raw

[[plan]]
Development Roadmap
-------------------

  [X] Milestone 0 - Project kick off
  [X] Milestone 1 - Baseline unquantised codec running under Linux/gcc
  [ ] Milestone 3 - Prototype 2400 bit/s codec
      [X] Spectral amplitudes modelled and quantised 
      [X] Phase and voicing model developed
      [ ] Pitch estimator integrated into encoder
      [ ] Frame rate/quantisation schemes for 2400 bit/s developed
      [ ] Refactor to develop a encoder/decoder functions
      [ ] Test phone call over LAN

[[howitworks]]
How it Works
------------

Speech is modelled as a sum of sinusoids:

  for(m=1; m<=L; m++)
    s[n] = A[m]*cos(Wo*m + phi[m]);

The sinusoids are multiples of the fundamental frequency Wo
(omega-naught), so the technique is known as "harmonic sinusoidal
coding".  For each frame, we analyse the speech signal and extract a set of
parameters:

  Wo, {A}, {phi}

Where Wo is the fundamental frequency (also know as the pitch), { A
} is a set of L amplitudes and { phi } is a set of L phases.  L is
chosen to be equal to the number of harmonics that can fit in a 4 KHz
bandwidth:

  L = floor(pi/Wo)

Wo is specified in radians normalised to 4 kHz, such that pi radians
= 4 kHz.  the fundamental frequency in Hz is:

  F0 = (8000/(2*pi))*Wo

We then need to encode (quantise) Wo, { A }, { phi } and transmit them to
a decoder which reconstructs the speech.  A frame might be 10-20ms in
length so we update the parameters every 10-20ms (100 to 50 Hz
update rate).

The speech quality of the basic harmonic sinusoidal model is pretty
good, close to transparent.  It is also relatively robust to Wo
estimation errors.  Unvoiced Speech (e.g. consonants) are well
modelled by a bunch of harmonics with random phases.  Speech corrupted
with background noise also sounds OK, the background noise doesn't
introduce any grossly unpleasant artifacts.

As the parameters are quantised to a low bit rate and sent over the
channel, the speech quality drops.  The challenge is to achieve a
reasonable trade off between speech quality and bit rate.

[[challenges]]
Challenges
----------

The tough bits of this project are:

1. Parameter estimation, in particular pitch (Wo) detection.

2. Reduction of a time-varying number of parameters (L changes with Wo
   each frame) to a fixed number of parameters required for a fixed
   bit rate.  The trick here is that { A } tend to vary slowly with
   frequency, so we can "fit" a curve to the set of { A } and send
   parameters that describe that curve.

3. Discarding the phases { phi }.  In most low bit rate codecs phases
   are discarded, and synthesised at the decoder using a rule-based
   approach.  This also implies the need for a "voicing" model as
   voiced speech (vowels) tends to have a different phase structure to
   unvoiced (constants).  The voicing model needs to be accurate (not
   introduce distortion), and relatively low bit rate.

4. Quantisation of the amplitudes { A } to a small number of bits while
   maintaining speech quality.  For example 30 bits/frame is 1500
   bits/s, a large part of our 2400 bit/s "budget".

5. Performance with different speakers and background noise
   conditions.  This is where you come in - as codec2 develops please
   send me samples of it's performance with various speakers and
   background noise conditions and together we will improve the
   algorithm.  This approach proved very powerful when developing
   link:oslec.html[Oslec].  One of the cool things about open source!

[[patents]]
Is it Patent Free?
------------------

I think so - much of the work is based on old papers from the 60, 70s
and 80's and the PhD thesis work [2] used as a baseline for this codec
was original.  A nice little mini project would be to audit the
patents used by proprietary 2400 bit/s codecs (MELP and xMBE) and
compare.

Proprietary codecs typically have small, novel parts of the algorithm
protected by patents.  However the designers of these codecs rely
heavily on large bodies of existing, public domain work.  The patents
cover perhaps 5% of the codec algorithms.  The designers of
proprietary codecs did not invent most of the algorithms they use in
their codec. Typically, the patents just cover enough to make
designing an interoperable codec very difficult.  These also tend to
be the parts that make their codecs sound good.

However there are many ways to make a codec sound good, so we simply
need to choose and develop other methods.

[[compat]]
Is Codec2 compatible with xMBE or MELP?
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Nope - I don't think it's possible to build a compatible codec without
infringing on patents or access to commercial in confidence
information.

[[lpc]]
LPC Modelling Notes
-------------------

  $ ./sinedec ../raw/hts1a.raw hts1a.mdl --lpc 10 - hts1a_lpc10.raw

The blog post [4] discusses why LPC modelling works so well when Am
recovered via RMS method (Section 5.1 of thesis).  Equal area model of
LPC spectra versus harmonic seems to work remarkably well, especially
compared to sampling.  SNRs up to 30dB on female frames.

m=1 harmonic problem for males when LPC modelled. The amplitude of
this harmonic is raised by as much as 30dB after LPC modelling as (I
think) LPC spectra must have zero derivative at DC.  This means it's
poor at modelling very low freq harmonics which unfortunately the ear
is very sensitive to.  Consider automatic lowering for 20dB of this
harmonic or maybe a few extra bits to quantise error.

[[phase]]
Phase Modelling Notes
---------------------

Phase modelling makes no attempt to match harmonic phases at frame
edges.  This area would be worth experimenting with, as it could cause
roughness.  Then again it might be responsible for effective mixed
voicing modelling.

Unvoiced speech can be represented well by random phases and a Wo
estimate that jumps around randomly.  If Wo is small the number of
harmonics is large which makes the noise less periodic and more noise
like to the ear.  With Wo jumping around phase tracks are
discontinuous between frames which also makes the synthesised signal
more noise like and prevents time domain pulses forming that the ear
is sensitive to.

Running Phase Models
~~~~~~~~~~~~~~~~~~~~

In src/phase.c two phase models have been implemented, the algorithms
are explained in the source code.

Zero Phase model (just one voicing bit/frame):

  $ ./sinedec ../raw/hts1a.raw hts1a.mdl --phase 0 - hts1a_phase0.raw

First order Phase model (around 12 bits/frame bits for pulse position
and phase):

  $ ./sinedec ../raw/hts1a.raw hts1a.mdl --phase 1 - hts1a_phase1.raw


[[octave]]
Octave Scripts 
--------------

pl.m    - plot a segment from a raw file

pl2.m   - plot the same segments from two different files to compare

plamp.m - menu based GUI interface to "dump" files, move back and forward
          through file examining time and frequency domain parameters, lpc 
          model etc
  
          $ ./sinedec ../raw/hts1a.raw hts1a.mdl --lpc 10 --dump hts1a
          $ cd ../octave
          $ octave
          octave:1> plamp("../src/hts1",25)

[[directories]]
Directories
-----------

  script   - shell scripts to simply common operations
  speex    - LSP quantisation code borrowed from Speex for testing
  src      - C source code
  octave   - Matlab/Octave scripts
  pitch    - pitch estimator output files
  raw      - speech files in raw format (16 bits signed linear 8 KHz)
  unittest - Unit test source code
  wav      - speech files in wave file format

[[refs]]
References
----------

[1] http://perens.com/[Bruce Perens] introducing the
    http://codec2.org/[codec2 project concept]

[2] David's PhD Thesis,
    http://www.itr.unisa.edu.au/~steven/thesis/dgr.pdf["Techniques for
    Harmonic Sinusoidal Coding"], used for baseline algorithm

[3] http://www.rowetel.com/blog/?p=128[Open Source Low rate Speech
    Codec Part 1 - Introduction]
    
[4] http://www.rowetel.com/blog/?p=130[Open Source Low rate Speech
    Codec Part 1 - Spectral Magnitudes]
    
[5] http://www.rowetel.com/blog/?p=131[Open Source Low rate Speech
    Codec Part 2 - Phase and Male Speech]

