#!/bin/bash

# Check the number of arguments
# $1 will be the file name to play the pcm data
# $ ./audio_play.sh pcm.data

if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <pcm_file> <wav_file>"
    exit 1
fi

aplay -r 16000 -f S16_LE -c 1 $1

ffmpeg -f s16le -ar 16000 -ac 1 -i $1 $2