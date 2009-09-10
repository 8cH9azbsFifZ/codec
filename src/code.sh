#!/bin/sh
# code.sh
# David Rowe 10 Sep 2009
#
# Run steps to encode a speech sample

../unittest/tnlp ../raw/$1.raw ../unittest/$1_nlp.p
../src/sinenc ../raw/$1.raw %1.mdl 300 ../unittest/$1_nlp.p
../src/sinedec ../raw/$1.raw %1.mdl -o $1_uq.raw
../src/sinedec ../raw/$1.raw %1.mdl --phase 0 -o $1_phase0.raw

