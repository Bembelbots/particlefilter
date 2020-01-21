#!/bin/bash

set -xe
make -j4  

if [ -z "$1" ]
then 
	./particlefiltertest
else 
	./particlefiltertest "${1}"
fi

cd  test/LogFileVizualizer 
python vizualizer.py -l ../../ParticleFilterOutput.log
