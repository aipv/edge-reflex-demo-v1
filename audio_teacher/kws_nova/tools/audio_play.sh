#!/bin/bash

# Check the number of arguments
# $1 will be the file name to play the pcm data
# $ ./audio_play.sh pcm.data

if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <filename>"
    exit 1
fi

aplay -r 16000 -f S16_LE -c 1 $1