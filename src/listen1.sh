#!/bin/sh
# listen1.sh
# David Rowe 10 Sep 2009
#
# Run menu with common sample file options, headphone version

../script/menu.sh ../raw/$1.raw $1_uq.raw $1_phase0.raw $1_lpc10.raw $1_lsp.raw $1_phase0_lpc10.raw $1_phase0_lsp.raw ../raw/$1_g729a.raw $2 $3 -d /dev/dsp1


