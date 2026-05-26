#!/bin/bash

# $1 will be the file name to store the pcm data
# Check the number of arguments
if [ "$#" -eq 0 ]; then
    echo "Saving data to default.pcm"
	nc -l -p 8888 > default.pcm
else
    echo "Saving data to $1"
	nc -l -p 8888 > $1
fi
