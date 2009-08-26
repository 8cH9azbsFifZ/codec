#!/bin/sh
# Plays a raw file
# usage:
#   playraw file.raw
#   playraw file.raw -d /deve/dsp1
play -f s -r 8000 -s w $1 $2 $3
