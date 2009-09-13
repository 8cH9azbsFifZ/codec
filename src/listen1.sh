#!/bin/sh
# listen1.sh
# David Rowe 10 Sep 2009
#
# Run menu with common sample file options, headphone version

../script/menu.sh ../raw/$1.raw $1_uq.raw $1_phase0.raw $1_lpc10.raw ../raw/$1_speex_8k.raw -d /dev/dsp1


