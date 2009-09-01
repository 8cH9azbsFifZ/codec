#!/bin/sh
# Converts 16 bit signed short 8 kHz raw (headerless) files to wave
sox -t raw -r 8000 -s -w -c 1 $1 $2
